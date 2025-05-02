#include "osgb2b3dm.h"
#include <osg/Geode>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/BoundingBox>
#include <osg/Group>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/Image>
#include <osg/BlendFunc>
#include <osg/AlphaFunc>
#include <osg/AnimationPath>
#include <osgAnimation/MorphGeometry>
#include <osgAnimation/AnimationUpdateCallback>
#include <fstream>
#include <sstream>
#include <zlib.h>
#include <iostream>

Osgb2B3dm::Osgb2B3dm() {}

Osgb2B3dm::~Osgb2B3dm() {}

bool Osgb2B3dm::convert(const std::string& inputPath, const std::string& outputPath) {
    // 读取 OSGB 文件
    osg::ref_ptr<osg::Node> node = readOsgb(inputPath);
    if (!node) {
        std::cerr << "Failed to read OSGB file: " << inputPath << std::endl;
        return false;
    }

    // 提取几何数据
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<unsigned short> indices;
    std::vector<std::string> texturePaths;
    osg::BoundingBox bbox;

    // 遍历场景图
    osg::Group* group = node->asGroup();
    if (group) {
        for (unsigned int i = 0; i < group->getNumChildren(); ++i) {
            osg::Node* child = group->getChild(i);
            if (osg::Geode* geode = child->asGeode()) {
                for (unsigned int j = 0; j < geode->getNumDrawables(); ++j) {
                    osg::Geometry* geom = geode->getDrawable(j)->asGeometry();
                    if (geom) {
                        extractGeometry(geom, positions, normals, texcoords, indices, bbox, osg::Matrix::identity());
                    }
                }
            }
        }
    }

    // 提取变形动画数据
    extractMorphData(node.get());

    // 提取动画事件
    extractAnimationEvents(node.get());

    // 生成 glTF JSON
    nlohmann::json gltfJson = generateGltfJson(positions, normals, texcoords, indices, bbox, texturePaths);

    // 添加变形目标
    addMorphTargetsToGltf(gltfJson, _morphTargets);

    // 添加变形动画
    addMorphAnimationsToGltf(gltfJson, _animations);

    // 添加动画事件
    addAnimationEventsToGltf(gltfJson, _animationEvents);

    // 打包二进制数据
    std::vector<unsigned char> binaryData = packBinaryData(positions, normals, texcoords, indices);

    // 写入 B3DM 文件
    return writeB3dm(outputPath, gltfJson, binaryData);
}

void Osgb2B3dm::extractGeometry(osg::Geometry* geom,
                               std::vector<float>& positions,
                               std::vector<float>& normals,
                               std::vector<float>& texcoords,
                               std::vector<unsigned short>& indices,
                               osg::BoundingBox& bbox,
                               const osg::Matrix& matrix) {
    if (!geom) return;

    // 获取顶点数组
    osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
    if (!verts) return;

    // 获取法线数组
    osg::Vec3Array* norms = dynamic_cast<osg::Vec3Array*>(geom->getNormalArray());

    // 获取纹理坐标数组
    osg::Vec2Array* texs = dynamic_cast<osg::Vec2Array*>(geom->getTexCoordArray(0));

    // 记录当前顶点数
    size_t vertexOffset = positions.size() / 3;

    // 添加顶点和法线
    for (size_t i = 0; i < verts->size(); ++i) {
        osg::Vec3 v = (*verts)[i] * matrix;
        positions.push_back(v.x());
        positions.push_back(v.y());
        positions.push_back(v.z());
        bbox.expandBy(v);

        if (norms && i < norms->size()) {
            osg::Vec3 n = osg::Matrix::transform3x3(matrix, (*norms)[i]);
            n.normalize();
            normals.push_back(n.x());
            normals.push_back(n.y());
            normals.push_back(n.z());
        }

        if (texs && i < texs->size()) {
            const osg::Vec2& t = (*texs)[i];
            texcoords.push_back(t.x());
            texcoords.push_back(t.y());
        }
    }

    // 添加索引
    for (unsigned int i = 0; i < geom->getNumPrimitiveSets(); ++i) {
        osg::PrimitiveSet* primSet = geom->getPrimitiveSet(i);
        if (!primSet) continue;

        switch (primSet->getMode()) {
            case osg::PrimitiveSet::TRIANGLES:
            case osg::PrimitiveSet::TRIANGLE_STRIP:
            case osg::PrimitiveSet::TRIANGLE_FAN: {
                for (unsigned int j = 0; j < primSet->getNumIndices(); ++j) {
                    unsigned int index = primSet->index(j) + vertexOffset;
                    if (index < 65536) {  // 限制为 16 位索引
                        indices.push_back(static_cast<unsigned short>(index));
                    }
                }
                break;
            }
            default:
                break;
        }
    }
}

osg::ref_ptr<osg::Node> Osgb2B3dm::readOsgb(const std::string& path) {
    osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(path);
    return node;
}

void Osgb2B3dm::extractMorphData(osg::Node* node) {
    if (!node) return;

    // 检查变形几何体
    osg::Geode* geode = node->asGeode();
    if (geode) {
        for (unsigned int i = 0; i < geode->getNumDrawables(); ++i) {
            osgAnimation::MorphGeometry* morphGeom = dynamic_cast<osgAnimation::MorphGeometry*>(
                geode->getDrawable(i));
            if (morphGeom) {
                processMorphGeometry(morphGeom);
                extractMorphAnimation(node, node->getName());
            }
        }
    }

    // 递归处理子节点
    osg::Group* group = node->asGroup();
    if (group) {
        for (unsigned int i = 0; i < group->getNumChildren(); ++i) {
            extractMorphData(group->getChild(i));
        }
    }
}

void Osgb2B3dm::processMorphGeometry(osgAnimation::MorphGeometry* morphGeom) {
    if (!morphGeom) return;

    // 获取基础几何体
    osg::Geometry* baseGeom = morphGeom;  // MorphGeometry inherits from Geometry
    if (!baseGeom) return;

    // 创建变形目标
    MorphTarget target;
    target.name = morphGeom->getName() + "_morph";

    // 提取顶点和法线
    osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>(baseGeom->getVertexArray());
    osg::Vec3Array* norms = dynamic_cast<osg::Vec3Array*>(baseGeom->getNormalArray());

    if (verts) {
        for (size_t i = 0; i < verts->size(); ++i) {
            const osg::Vec3& v = (*verts)[i];
            target.positions.push_back(v.x());
            target.positions.push_back(v.y());
            target.positions.push_back(v.z());
        }
    }

    if (norms) {
        for (size_t i = 0; i < norms->size(); ++i) {
            const osg::Vec3& n = (*norms)[i];
            target.normals.push_back(n.x());
            target.normals.push_back(n.y());
            target.normals.push_back(n.z());
        }
    }

    // 获取权重
    const std::vector<osgAnimation::MorphGeometry::MorphTarget>& targets = morphGeom->getMorphTargetList();
    for (const auto& t : targets) {
        target.weights.push_back(static_cast<float>(t.getWeight()));
    }

    _morphTargets.push_back(target);
}

void Osgb2B3dm::extractMorphAnimation(osg::Node* node, const std::string& nodeName) {
    if (!node) return;

    // 创建动画
    Animation anim;
    anim.name = nodeName + "_morph_animation";
    
    // 创建动画通道
    AnimationChannel channel;
    channel.targetNode = nodeName;
    channel.path = "weights";
    
    // 获取变形几何体
    osg::Geode* geode = node->asGeode();
    if (geode) {
        for (unsigned int i = 0; i < geode->getNumDrawables(); ++i) {
            osgAnimation::MorphGeometry* morphGeom = dynamic_cast<osgAnimation::MorphGeometry*>(
                geode->getDrawable(i));
            if (morphGeom) {
                // 获取权重关键帧
                const std::vector<osgAnimation::MorphGeometry::MorphTarget>& targets = 
                    morphGeom->getMorphTargetList();
                
                // 添加关键帧
                for (size_t j = 0; j < targets.size(); ++j) {
                    channel.times.push_back(static_cast<float>(j));
                    channel.values.push_back(static_cast<float>(targets[j].getWeight()));
                }
                
                // 设置动画持续时间
                anim.duration = static_cast<double>(targets.size());
            }
        }
    }
    
    if (!channel.times.empty()) {
        // 添加通道和采样器
        std::string channelId = nodeName + "_weights";
        anim.channels.push_back(channelId);
        
        AnimationSampler sampler;
        sampler.input = channel.times;
        sampler.output = channel.values;
        _animationSamplers.push_back(sampler);
        
        _animationChannels.push_back(channel);
        _animations.push_back(anim);
    }
}

void Osgb2B3dm::extractAnimationEvents(osg::Node* node) {
    if (!node) return;

    // 检查动画事件
    processAnimationCallback(node, node->getName());

    // 递归处理子节点
    osg::Group* group = node->asGroup();
    if (group) {
        for (unsigned int i = 0; i < group->getNumChildren(); ++i) {
            extractAnimationEvents(group->getChild(i));
        }
    }
}

void Osgb2B3dm::processAnimationCallback(osg::Node* node, const std::string& nodeName) {
    if (!node) return;

    // 检查用户数据中的事件
    osg::Referenced* userData = node->getUserData();
    if (userData) {
        AnimationEvent event;
        event.name = nodeName + "_event";
        event.time = 0.0;  // 默认时间
        event.type = "user_event";
        
        // 创建一个基本的事件数据
        event.data = {
            {"node", nodeName},
            {"type", "user_event"}
        };
        _animationEvents.push_back(event);
    }
}

void Osgb2B3dm::addMorphTargetsToGltf(nlohmann::json& gltf, 
                                     const std::vector<MorphTarget>& morphTargets) {
    if (morphTargets.empty()) return;

    // 添加变形目标
    gltf["meshes"][0]["primitives"][0]["targets"] = nlohmann::json::array();
    for (const auto& target : morphTargets) {
        nlohmann::json morphTarget = {
            {"POSITION", target.positions},
            {"NORMAL", target.normals}
        };
        gltf["meshes"][0]["primitives"][0]["targets"].push_back(morphTarget);
    }

    // 添加初始权重
    gltf["meshes"][0]["weights"] = nlohmann::json::array();
    for (const auto& target : morphTargets) {
        if (!target.weights.empty()) {
            gltf["meshes"][0]["weights"].push_back(target.weights[0]);
        } else {
            gltf["meshes"][0]["weights"].push_back(0.0f);
        }
    }
}

void Osgb2B3dm::addMorphAnimationsToGltf(nlohmann::json& gltf,
                                        const std::vector<Animation>& animations) {
    if (animations.empty()) return;

    // 添加动画
    gltf["animations"] = nlohmann::json::array();
    
    for (size_t i = 0; i < animations.size(); ++i) {
        const Animation& anim = animations[i];
        nlohmann::json animation = {
            {"name", anim.name},
            {"channels", nlohmann::json::array()},
            {"samplers", nlohmann::json::array()}
        };

        // 添加采样器
        for (size_t j = 0; j < anim.samplers.size(); ++j) {
            const AnimationSampler& sampler = _animationSamplers[j];
            animation["samplers"].push_back({
                {"input", sampler.input},
                {"output", sampler.output},
                {"interpolation", sampler.interpolation}
            });
        }

        // 添加通道
        for (size_t j = 0; j < anim.channels.size(); ++j) {
            const AnimationChannel& channel = _animationChannels[j];
            animation["channels"].push_back({
                {"sampler", static_cast<int>(j)},
                {"target", {
                    {"node", 0},
                    {"path", channel.path}
                }}
            });
        }

        gltf["animations"].push_back(animation);
    }
}

void Osgb2B3dm::addAnimationEventsToGltf(nlohmann::json& gltf,
                                        const std::vector<AnimationEvent>& events) {
    if (events.empty()) return;

    // 添加动画事件扩展
    gltf["extensions"]["KHR_animation_events"] = nlohmann::json::array();
    
    for (const auto& event : events) {
        nlohmann::json eventJson = {
            {"name", event.name},
            {"time", event.time},
            {"type", event.type},
            {"data", event.data}
        };
        gltf["extensions"]["KHR_animation_events"].push_back(eventJson);
    }
}

nlohmann::json Osgb2B3dm::generateGltfJson(const std::vector<float>& positions,
                                          const std::vector<float>& normals,
                                          const std::vector<float>& texcoords,
                                          const std::vector<unsigned short>& indices,
                                          const osg::BoundingBox& bbox,
                                          const std::vector<std::string>& texturePaths) {
    nlohmann::json gltf;

    // 设置基本结构
    gltf["asset"] = {
        {"version", "2.0"},
        {"generator", "Osgb2B3dm"}
    };

    // 添加 buffers
    gltf["buffers"] = nlohmann::json::array();
    gltf["buffers"].push_back({
        {"uri", "data.bin"},
        {"byteLength", positions.size() * sizeof(float) +
                      normals.size() * sizeof(float) +
                      texcoords.size() * sizeof(float) +
                      indices.size() * sizeof(unsigned short)}
    });

    // 添加 bufferViews
    gltf["bufferViews"] = nlohmann::json::array();
    size_t offset = 0;

    // 顶点
    gltf["bufferViews"].push_back({
        {"buffer", 0},
        {"byteOffset", offset},
        {"byteLength", positions.size() * sizeof(float)},
        {"target", 34962} // ARRAY_BUFFER
    });
    offset += positions.size() * sizeof(float);

    // 法线
    gltf["bufferViews"].push_back({
        {"buffer", 0},
        {"byteOffset", offset},
        {"byteLength", normals.size() * sizeof(float)},
        {"target", 34962}
    });
    offset += normals.size() * sizeof(float);

    // 纹理坐标
    gltf["bufferViews"].push_back({
        {"buffer", 0},
        {"byteOffset", offset},
        {"byteLength", texcoords.size() * sizeof(float)},
        {"target", 34962}
    });
    offset += texcoords.size() * sizeof(float);

    // 索引
    gltf["bufferViews"].push_back({
        {"buffer", 0},
        {"byteOffset", offset},
        {"byteLength", indices.size() * sizeof(unsigned short)},
        {"target", 34963} // ELEMENT_ARRAY_BUFFER
    });

    // 添加 accessors
    gltf["accessors"] = nlohmann::json::array();
    
    // 顶点访问器
    gltf["accessors"].push_back({
        {"bufferView", 0},
        {"componentType", 5126}, // FLOAT
        {"count", positions.size() / 3},
        {"type", "VEC3"},
        {"min", {bbox.xMin(), bbox.yMin(), bbox.zMin()}},
        {"max", {bbox.xMax(), bbox.yMax(), bbox.zMax()}}
    });

    // 法线访问器
    gltf["accessors"].push_back({
        {"bufferView", 1},
        {"componentType", 5126}, // FLOAT
        {"count", normals.size() / 3},
        {"type", "VEC3"}
    });

    // 纹理坐标访问器
    gltf["accessors"].push_back({
        {"bufferView", 2},
        {"componentType", 5126}, // FLOAT
        {"count", texcoords.size() / 2},
        {"type", "VEC2"}
    });

    // 索引访问器
    gltf["accessors"].push_back({
        {"bufferView", 3},
        {"componentType", 5123}, // UNSIGNED_SHORT
        {"count", indices.size()},
        {"type", "SCALAR"}
    });

    // 添加材质
    gltf["materials"] = nlohmann::json::array();
    for (const auto& mat : _materials) {
        nlohmann::json material = {
            {"name", mat.name},
            {"doubleSided", mat.doubleSided},
            {"alphaMode", mat.blendMode},
            {"alphaCutoff", 0.5f},
            {"pbrMetallicRoughness", {
                {"baseColorFactor", mat.diffuse},
                {"metallicFactor", mat.metallic},
                {"roughnessFactor", mat.roughness}
            }},
            {"normalTexture", {
                {"index", getTextureIndex(texturePaths, mat.normalTexture.path)},
                {"scale", mat.normalScale}
            }},
            {"occlusionTexture", {
                {"index", getTextureIndex(texturePaths, mat.occlusionTexture.path)},
                {"strength", mat.occlusion}
            }},
            {"emissiveFactor", mat.emission},
            {"emissiveTexture", {
                {"index", getTextureIndex(texturePaths, mat.emissiveTexture.path)}
            }},
            {"extensions", {
                {"KHR_materials_pbrSpecularGlossiness", {
                    {"diffuseFactor", mat.diffuse},
                    {"specularFactor", mat.specular},
                    {"glossinessFactor", 1.0f - mat.roughness}
                }}
            }}
        };

        // 添加纹理变换
        if (!mat.baseColorTexture.path.empty()) {
            material["pbrMetallicRoughness"]["baseColorTexture"] = {
                {"index", getTextureIndex(texturePaths, mat.baseColorTexture.path)},
                {"texCoord", mat.baseColorTexture.texCoord},
                {"extensions", {
                    {"KHR_texture_transform", {
                        {"offset", mat.baseColorTexture.offset},
                        {"rotation", mat.baseColorTexture.rotation[2]},
                        {"scale", {mat.baseColorTexture.scale, mat.baseColorTexture.scale}}
                    }}
                }}
            };
        }

        gltf["materials"].push_back(material);
    }

    // 如果没有材质，添加默认材质
    if (gltf["materials"].empty()) {
        gltf["materials"].push_back({
            {"name", "default"},
            {"pbrMetallicRoughness", {
                {"baseColorFactor", {1.0, 1.0, 1.0, 1.0}},
                {"metallicFactor", 0.0},
                {"roughnessFactor", 1.0}
            }}
        });
    }

    // 添加 meshes
    gltf["meshes"] = nlohmann::json::array();
    gltf["meshes"].push_back({
        {"name", "mesh"},
        {"primitives", nlohmann::json::array({
            {
                {"attributes", {
                    {"POSITION", 0},
                    {"NORMAL", 1},
                    {"TEXCOORD_0", 2}
                }},
                {"indices", 3},
                {"material", 0},
                {"mode", 4} // TRIANGLES
            }
        })}
    });

    // 添加节点
    gltf["nodes"] = nlohmann::json::array();
    gltf["nodes"].push_back({
        {"name", "root"},
        {"mesh", 0}
    });

    // 添加场景
    gltf["scenes"] = nlohmann::json::array();
    gltf["scenes"].push_back({
        {"name", "default"},
        {"nodes", {0}}
    });

    // 设置默认场景
    gltf["scene"] = 0;

    // 添加动画
    if (!_animations.empty()) {
        gltf["animations"] = nlohmann::json::array();
        for (const auto& anim : _animations) {
            nlohmann::json animation = {
                {"name", anim.name},
                {"channels", anim.channels},
                {"samplers", anim.samplers}
            };
            gltf["animations"].push_back(animation);
        }
    }

    // 添加变形目标
    addMorphTargetsToGltf(gltf, _morphTargets);

    // 添加变形动画
    addMorphAnimationsToGltf(gltf, _animations);

    // 添加动画事件
    addAnimationEventsToGltf(gltf, _animationEvents);

    return gltf;
}

std::vector<unsigned char> Osgb2B3dm::packBinaryData(const std::vector<float>& positions,
                                                   const std::vector<float>& normals,
                                                   const std::vector<float>& texcoords,
                                                   const std::vector<unsigned short>& indices) {
    std::vector<unsigned char> binaryData;
    
    // 计算总大小
    size_t totalSize = positions.size() * sizeof(float) +
                      normals.size() * sizeof(float) +
                      texcoords.size() * sizeof(float) +
                      indices.size() * sizeof(unsigned short);
    
    binaryData.resize(totalSize);
    unsigned char* ptr = binaryData.data();
    
    // 复制顶点数据
    memcpy(ptr, positions.data(), positions.size() * sizeof(float));
    ptr += positions.size() * sizeof(float);
    
    // 复制法线数据
    memcpy(ptr, normals.data(), normals.size() * sizeof(float));
    ptr += normals.size() * sizeof(float);
    
    // 复制纹理坐标数据
    memcpy(ptr, texcoords.data(), texcoords.size() * sizeof(float));
    ptr += texcoords.size() * sizeof(float);
    
    // 复制索引数据
    memcpy(ptr, indices.data(), indices.size() * sizeof(unsigned short));
    
    return binaryData;
}

bool Osgb2B3dm::writeB3dm(const std::string& path, const nlohmann::json& gltfJson,
                         const std::vector<unsigned char>& binaryData) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;

    // B3DM 头部
    const char magic[4] = {'b', '3', 'd', 'm'};
    out.write(magic, 4);

    // 版本
    uint32_t version = 1;
    out.write(reinterpret_cast<const char*>(&version), 4);

    // 计算总长度
    std::string jsonStr = gltfJson.dump();
    uint32_t totalLength = 28 + jsonStr.size() + binaryData.size();
    out.write(reinterpret_cast<const char*>(&totalLength), 4);

    // JSON 长度
    uint32_t jsonLength = jsonStr.size();
    out.write(reinterpret_cast<const char*>(&jsonLength), 4);

    // 写入 JSON
    out.write(jsonStr.c_str(), jsonStr.size());

    // 写入二进制数据
    out.write(reinterpret_cast<const char*>(binaryData.data()), binaryData.size());

    return true;
}

void Osgb2B3dm::extractMaterialProperties(osg::StateSet* stateSet, Material& material) {
    if (!stateSet) return;

    // 提取材质
    osg::Material* osgMaterial = dynamic_cast<osg::Material*>(
        stateSet->getAttribute(osg::StateAttribute::MATERIAL));
    if (osgMaterial) {
        // 基础颜色属性
        osg::Vec4 ambient = osgMaterial->getAmbient(osg::Material::FRONT);
        osg::Vec4 diffuse = osgMaterial->getDiffuse(osg::Material::FRONT);
        osg::Vec4 specular = osgMaterial->getSpecular(osg::Material::FRONT);
        osg::Vec4 emission = osgMaterial->getEmission(osg::Material::FRONT);
        float shininess = osgMaterial->getShininess(osg::Material::FRONT);

        material.ambient = {ambient.r(), ambient.g(), ambient.b(), ambient.a()};
        material.diffuse = {diffuse.r(), diffuse.g(), diffuse.b(), diffuse.a()};
        material.specular = {specular.r(), specular.g(), specular.b(), specular.a()};
        material.emission = {emission.r(), emission.g(), emission.b(), emission.a()};

        // 将光泽度转换为粗糙度
        material.roughness = 1.0f - (shininess / 128.0f);
        
        // 从镜面反射颜色计算金属度
        float maxSpecular = std::max({specular.r(), specular.g(), specular.b()});
        material.metallic = maxSpecular;
    }

    // 检查双面渲染
    osg::StateAttribute::OverrideValue ov = stateSet->getMode(GL_CULL_FACE);
    material.doubleSided = (ov == osg::StateAttribute::OFF);

    // 检查透明度和混合模式
    osg::BlendFunc* blendFunc = dynamic_cast<osg::BlendFunc*>(
        stateSet->getAttribute(osg::StateAttribute::BLENDFUNC));
    osg::AlphaFunc* alphaFunc = dynamic_cast<osg::AlphaFunc*>(
        stateSet->getAttribute(osg::StateAttribute::ALPHAFUNC));

    if (blendFunc || alphaFunc) {
        material.blendMode = "BLEND";
        if (alphaFunc && alphaFunc->getFunction() == osg::AlphaFunc::GREATER) {
            material.blendMode = "MASK";
        }
    }

    // 提取纹理信息
    extractTextureInfo(stateSet, material.baseColorTexture, 
                      osg::StateAttribute::TEXTURE, 0);
    extractTextureInfo(stateSet, material.normalTexture, 
                      osg::StateAttribute::TEXTURE, 1);
    extractTextureInfo(stateSet, material.metallicRoughnessTexture, 
                      osg::StateAttribute::TEXTURE, 2);
    extractTextureInfo(stateSet, material.occlusionTexture, 
                      osg::StateAttribute::TEXTURE, 3);
    extractTextureInfo(stateSet, material.emissiveTexture, 
                      osg::StateAttribute::TEXTURE, 4);
}

void Osgb2B3dm::extractTextureInfo(osg::StateSet* stateSet, 
                                  Material::TextureInfo& textureInfo,
                                  osg::StateAttribute::Type type, 
                                  int unit) {
    if (!stateSet) return;

    osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(
        stateSet->getTextureAttribute(unit, type));
    if (texture && texture->getImage()) {
        textureInfo.path = texture->getImage()->getFileName();
        
        // 提取纹理变换
        osg::TexMat* texMat = dynamic_cast<osg::TexMat*>(
            stateSet->getTextureAttribute(unit, osg::StateAttribute::TEXMAT));
        if (texMat) {
            osg::Matrix mat = texMat->getMatrix();
            textureInfo.scale = static_cast<float>(mat(0, 0)); // 假设均匀缩放
            textureInfo.offset = {
                static_cast<float>(mat(3, 0)), 
                static_cast<float>(mat(3, 1))
            };
            
            // 提取旋转（简化版本）
            float angle = static_cast<float>(std::atan2(mat(1, 0), mat(0, 0)));
            textureInfo.rotation = {0.0f, 0.0f, angle};
        }
    }
}

// 辅助函数：获取纹理索引
int Osgb2B3dm::getTextureIndex(const std::vector<std::string>& texturePaths, 
                              const std::string& path) {
    if (path.empty()) return -1;
    auto it = std::find(texturePaths.begin(), texturePaths.end(), path);
    return (it != texturePaths.end()) ? std::distance(texturePaths.begin(), it) : -1;
}