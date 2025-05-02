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
#include <fstream>
#include <sstream>
#include <zlib.h>
#include <iostream>

Osgb2B3dm::Osgb2B3dm() {}

Osgb2B3dm::~Osgb2B3dm() {}

bool Osgb2B3dm::convert(const std::string& inputPath, const std::string& outputPath) {
    std::cout << "Converting " << inputPath << " to " << outputPath << std::endl;
    
    // 1. 读取 OSGB
    osg::ref_ptr<osg::Node> node = readOsgb(inputPath);
    if (!node) {
        std::cerr << "Failed to read OSGB file: " << inputPath << std::endl;
        return false;
    }

    // 2. 提取数据
    std::vector<float> positions, normals, texcoords;
    std::vector<unsigned short> indices;
    osg::BoundingBox bbox;
    std::vector<std::string> texturePaths;
    
    // 从单位矩阵开始
    extractNodeData(node.get(), positions, normals, texcoords, indices, bbox, texturePaths, osg::Matrix::identity());

    if (positions.empty()) {
        std::cerr << "No geometry data found in the model" << std::endl;
        return false;
    }

    // 3. 生成 glTF JSON
    nlohmann::json gltfJson = generateGltfJson(positions, normals, texcoords, indices, bbox, texturePaths);

    // 4. 生成二进制数据
    std::vector<unsigned char> binaryData = packBinaryData(positions, normals, texcoords, indices);

    // 5. 写入 B3DM
    if (!writeB3dm(outputPath, gltfJson, binaryData)) {
        std::cerr << "Failed to write B3DM file: " << outputPath << std::endl;
        return false;
    }

    std::cout << "Conversion completed successfully" << std::endl;
    return true;
}

void Osgb2B3dm::extractNodeData(osg::Node* node, 
                               std::vector<float>& positions,
                               std::vector<float>& normals,
                               std::vector<float>& texcoords,
                               std::vector<unsigned short>& indices,
                               osg::BoundingBox& bbox,
                               std::vector<std::string>& texturePaths,
                               const osg::Matrix& parentMatrix) {
    if (!node) return;

    // 计算当前节点的变换矩阵
    osg::Matrix currentMatrix = parentMatrix;
    osg::MatrixTransform* transform = dynamic_cast<osg::MatrixTransform*>(node);
    if (transform) {
        currentMatrix = currentMatrix * transform->getMatrix();
    }

    // 处理 Geode
    osg::Geode* geode = node->asGeode();
    if (geode) {
        for (unsigned int i = 0; i < geode->getNumDrawables(); ++i) {
            osg::Geometry* geom = geode->getDrawable(i)->asGeometry();
            if (geom) {
                // 处理材质和纹理
                osg::StateSet* stateSet = geom->getStateSet();
                if (stateSet) {
                    // 处理材质
                    osg::Material* material = dynamic_cast<osg::Material*>(
                        stateSet->getAttribute(osg::StateAttribute::MATERIAL));
                    if (material) {
                        // 提取材质属性
                        osg::Vec4 ambient = material->getAmbient(osg::Material::FRONT);
                        osg::Vec4 diffuse = material->getDiffuse(osg::Material::FRONT);
                        osg::Vec4 specular = material->getSpecular(osg::Material::FRONT);
                        osg::Vec4 emission = material->getEmission(osg::Material::FRONT);
                        float shininess = material->getShininess(osg::Material::FRONT);

                        // 创建材质对象
                        Material mat;
                        mat.ambient = {ambient.r(), ambient.g(), ambient.b(), ambient.a()};
                        mat.diffuse = {diffuse.r(), diffuse.g(), diffuse.b(), diffuse.a()};
                        mat.specular = {specular.r(), specular.g(), specular.b(), specular.a()};
                        mat.emission = {emission.r(), emission.g(), emission.b(), emission.a()};
                        mat.shininess = shininess;
                        
                        // 检查双面渲染
                        osg::StateAttribute::OverrideValue ov = stateSet->getMode(GL_CULL_FACE);
                        mat.doubleSided = (ov == osg::StateAttribute::OFF);

                        // 存储材质
                        _materials.push_back(mat);
                    }

                    // 处理纹理
                    osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(
                        stateSet->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
                    if (texture && texture->getImage()) {
                        std::string imagePath = texture->getImage()->getFileName();
                        if (!imagePath.empty()) {
                            texturePaths.push_back(imagePath);
                        }
                    }
                }

                extractGeometry(geom, positions, normals, texcoords, indices, bbox, currentMatrix);
            }
        }
    }

    // 处理 Group
    osg::Group* group = node->asGroup();
    if (group) {
        for (unsigned int i = 0; i < group->getNumChildren(); ++i) {
            extractNodeData(group->getChild(i), positions, normals, texcoords, indices, bbox, texturePaths, currentMatrix);
        }
    }
}

void Osgb2B3dm::extractGeometry(osg::Geometry* geom,
                               std::vector<float>& positions,
                               std::vector<float>& normals,
                               std::vector<float>& texcoords,
                               std::vector<unsigned short>& indices,
                               osg::BoundingBox& bbox,
                               const osg::Matrix& matrix) {
    // 提取顶点
    osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
    if (verts) {
        size_t startIndex = positions.size() / 3;
        for (size_t i = 0; i < verts->size(); ++i) {
            // 应用变换矩阵到顶点
            osg::Vec3 v = (*verts)[i] * matrix;
            positions.push_back(v.x());
            positions.push_back(v.y());
            positions.push_back(v.z());
            bbox.expandBy(v);
        }

        // 提取法线
        osg::Vec3Array* norms = dynamic_cast<osg::Vec3Array*>(geom->getNormalArray());
        if (norms) {
            // 计算法线变换矩阵（忽略平移）
            osg::Matrix normalMatrix = matrix;
            normalMatrix.setTrans(0, 0, 0);
            normalMatrix = osg::Matrix::inverse(normalMatrix);
            osg::Matrix transposedMatrix;
            normalMatrix.transpose(transposedMatrix);
            normalMatrix = transposedMatrix;

            for (size_t i = 0; i < norms->size(); ++i) {
                // 应用变换矩阵到法线
                osg::Vec3 n = (*norms)[i] * normalMatrix;
                n.normalize();
                normals.push_back(n.x());
                normals.push_back(n.y());
                normals.push_back(n.z());
            }
        }

        // 提取纹理坐标
        osg::Vec2Array* texs = dynamic_cast<osg::Vec2Array*>(geom->getTexCoordArray(0));
        if (texs) {
            for (size_t i = 0; i < texs->size(); ++i) {
                const osg::Vec2& t = (*texs)[i];
                texcoords.push_back(t.x());
                texcoords.push_back(t.y());
            }
        }

        // 提取索引
        for (unsigned int i = 0; i < geom->getNumPrimitiveSets(); ++i) {
            osg::PrimitiveSet* ps = geom->getPrimitiveSet(i);
            if (ps->getMode() == osg::PrimitiveSet::TRIANGLES) {
                osg::DrawElements* de = ps->getDrawElements();
                if (de) {
                    for (size_t j = 0; j < de->getNumIndices(); ++j) {
                        // 调整索引以考虑之前添加的顶点
                        indices.push_back(static_cast<unsigned short>(startIndex + de->index(j)));
                    }
                }
            }
        }
    }
}

osg::ref_ptr<osg::Node> Osgb2B3dm::readOsgb(const std::string& path) {
    osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(path);
    return node;
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
            {"name", "material_" + std::to_string(gltf["materials"].size())},
            {"doubleSided", mat.doubleSided},
            {"pbrMetallicRoughness", {
                {"baseColorFactor", mat.diffuse},
                {"metallicFactor", 0.0},
                {"roughnessFactor", 1.0 - (mat.shininess / 128.0)}
            }},
            {"emissiveFactor", mat.emission},
            {"extensions", {
                {"KHR_materials_pbrSpecularGlossiness", {
                    {"diffuseFactor", mat.diffuse},
                    {"specularFactor", mat.specular},
                    {"glossinessFactor", mat.shininess / 128.0}
                }}
            }}
        };
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