#include "osgb2b3dm.h"
#include "osgb2b3dm_error.h"
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

namespace osgb2b3dm {

Osgb2B3dm::Osgb2B3dm() {
    clearError();
}

Osgb2B3dm::~Osgb2B3dm() {
    // 清理资源
}

void Osgb2B3dm::setError(ErrorCode code, const std::string& message) {
    lastError_ = make_error_code(code);
    lastErrorMessage_ = message;
    if (verbose_) {
        std::cerr << "Error: " << message << " (code: " << code << ")" << std::endl;
    }
}

void Osgb2B3dm::clearError() {
    lastError_ = std::error_code();
    lastErrorMessage_.clear();
}

bool Osgb2B3dm::convert(const std::string& inputPath, const std::string& outputPath) {
    clearError();

    if (inputPath.empty()) {
        setError(ErrorCode::INVALID_INPUT, "Input path is empty");
        return false;
    }

    if (outputPath.empty()) {
        setError(ErrorCode::INVALID_OUTPUT, "Output path is empty");
        return false;
    }

    if (!loadInputFile(inputPath)) {
        return false;
    }

    if (validateInput_ && !validateInput()) {
        return false;
    }

    if (!processScene()) {
        return false;
    }

    if (validateOutput_ && !validateOutput()) {
        return false;
    }

    if (!saveOutputFile(outputPath)) {
        return false;
    }

    return true;
}

bool Osgb2B3dm::loadInputFile(const std::string& path) {
    try {
        // 检查文件是否存在
        std::ifstream file(path);
        if (!file) {
            setError(ErrorCode::FILE_READ_ERROR, "Input file does not exist: " + path);
            return false;
        }

        // 加载输入文件的实现
        osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(path);
        if (!node) {
            setError(ErrorCode::FILE_READ_ERROR, "Failed to read input file: " + path);
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        setError(ErrorCode::FILE_READ_ERROR, std::string("Failed to load input file: ") + e.what());
        return false;
    }
}

bool Osgb2B3dm::processScene() {
    try {
        // 检查动画数据
        if (!_animations.empty()) {
            for (const auto& anim : _animations) {
                if (anim.channels.empty() || anim.samplers.empty()) {
                    setError(ErrorCode::ANIMATION_DATA_INVALID, "Invalid animation data: missing channels or samplers");
                    return false;
                }
            }
        }

        // 检查实例数据
        if (!_instanceGroups.empty()) {
            for (const auto& group : _instanceGroups) {
                if (group.instances.empty()) {
                    setError(ErrorCode::INSTANCE_PROCESSING_ERROR, "Missing instance data");
                    return false;
                }
                for (const auto& instance : group.instances) {
                    if (instance.meshIndex < 0) {
                        setError(ErrorCode::INSTANCE_DATA_INVALID, "Invalid instance mesh index");
                        return false;
                    }
                }
            }
        }

        return true;
    } catch (const std::exception& e) {
        setError(ErrorCode::DATA_PROCESSING_ERROR, std::string("Failed to process scene: ") + e.what());
        return false;
    }
}

bool Osgb2B3dm::saveOutputFile(const std::string& path) {
    try {
        // 保存输出文件的实现
        // ... existing code ...
        return true;
    } catch (const std::exception& e) {
        setError(ErrorCode::FILE_WRITE_ERROR, std::string("Failed to save output file: ") + e.what());
        return false;
    }
}

bool Osgb2B3dm::validateInput() {
    try {
        // 验证输入的实现
        // ... existing code ...
        return true;
    } catch (const std::exception& e) {
        setError(ErrorCode::VALIDATION_FAILED, std::string("Input validation failed: ") + e.what());
        return false;
    }
}

bool Osgb2B3dm::validateOutput() {
    try {
        // 验证输出的实现
        // ... existing code ...
        return true;
    } catch (const std::exception& e) {
        setError(ErrorCode::VALIDATION_FAILED, std::string("Output validation failed: ") + e.what());
        return false;
    }
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

void Osgb2B3dm::extractSkeletonData(osg::Node* node) {
    if (!node) return;

    // 检查是否是骨骼节点
    osgAnimation::Skeleton* osgSkeleton = dynamic_cast<osgAnimation::Skeleton*>(node);
    if (osgSkeleton) {
        Skeleton skeleton;
        skeleton.name = node->getName();
        skeleton.osgSkeleton = osgSkeleton;
        
        // 提取骨骼层级
        extractJointHierarchy(osgSkeleton, skeleton);
        
        // 处理骨骼动画
        for (size_t i = 0; i < skeleton.joints.size(); ++i) {
            if (skeleton.joints[i].bone) {
                processBoneAnimation(skeleton.joints[i].bone, skeleton.name);
            }
        }
        
        _skeletons.push_back(skeleton);
    }

    // 检查蒙皮几何体
    osg::Geode* geode = node->asGeode();
    if (geode) {
        for (unsigned int i = 0; i < geode->getNumDrawables(); ++i) {
            osgAnimation::RigGeometry* rigGeometry = 
                dynamic_cast<osgAnimation::RigGeometry*>(geode->getDrawable(i));
            if (rigGeometry) {
                SkinData skinData;
                extractSkinningData(rigGeometry, skinData);
                _skinData.push_back(skinData);
            }
        }
    }

    // 递归处理子节点
    osg::Group* group = node->asGroup();
    if (group) {
        for (unsigned int i = 0; i < group->getNumChildren(); ++i) {
            extractSkeletonData(group->getChild(i));
        }
    }
}

void Osgb2B3dm::extractJointHierarchy(osgAnimation::Skeleton* skeleton, Skeleton& outSkeleton) {
    if (!skeleton) return;

    std::map<osgAnimation::Bone*, int> boneIndices;
    
    // 第一遍：创建所有关节
    for (unsigned int i = 0; i < skeleton->getNumChildren(); ++i) {
        osgAnimation::Bone* bone = dynamic_cast<osgAnimation::Bone*>(skeleton->getChild(i));
        if (bone) {
            Joint joint;
            joint.name = bone->getName();
            joint.bone = bone;
            joint.localMatrix = bone->getMatrix();
            joint.inverseBindMatrix = bone->getInverseMatrix();
            
            int jointIndex = outSkeleton.joints.size();
            boneIndices[bone] = jointIndex;
            outSkeleton.joints.push_back(joint);
        }
    }
    
    // 第二遍：建立父子关系
    for (size_t i = 0; i < outSkeleton.joints.size(); ++i) {
        Joint& joint = outSkeleton.joints[i];
        osgAnimation::Bone* bone = joint.bone;
        
        if (bone->getNumParents() > 0) {
            osgAnimation::Bone* parentBone = dynamic_cast<osgAnimation::Bone*>(bone->getParent(0));
            if (parentBone && boneIndices.find(parentBone) != boneIndices.end()) {
                joint.parentIndex = boneIndices[parentBone];
                outSkeleton.joints[joint.parentIndex].children.push_back(i);
            }
        } else {
            outSkeleton.rootJoint = i;
        }
    }
}

void Osgb2B3dm::processBoneAnimation(osgAnimation::Bone* bone, const std::string& skeletonName) {
    if (!bone) return;

    // 获取骨骼的动画回调
    osgAnimation::BasicAnimationManager* animManager = 
        dynamic_cast<osgAnimation::BasicAnimationManager*>(bone->getUpdateCallback());
    if (!animManager) return;

    // 创建动画通道
    SkeletonAnimationChannel channel;
    channel.jointName = bone->getName();
    
    // 获取所有动画
    const osgAnimation::AnimationList& animations = animManager->getAnimationList();
    for (const auto& anim : animations) {
        // 处理每个通道
        const osgAnimation::ChannelList& channels = anim->getChannels();
        for (const auto& animChannel : channels) {
            const std::string& targetName = animChannel->getName();
            const auto& sampler = animChannel->getSampler();
            
            if (targetName.find("translation") != std::string::npos) {
                // 处理位移动画
                channel.path = "translation";
                if (auto* keyframes = dynamic_cast<Vec3KeyframeContainer*>(sampler->getKeyframeContainer())) {
                    for (size_t i = 0; i < keyframes->size(); ++i) {
                        channel.times.push_back((*keyframes)[i].getTime());
                        const osg::Vec3d& pos = (*keyframes)[i].getValue();
                        channel.values.push_back(static_cast<float>(pos.x()));
                        channel.values.push_back(static_cast<float>(pos.y()));
                        channel.values.push_back(static_cast<float>(pos.z()));
                    }
                }
            }
            else if (targetName.find("rotation") != std::string::npos) {
                // 处理旋转动画
                channel.path = "rotation";
                if (auto* keyframes = dynamic_cast<QuatKeyframeContainer*>(sampler->getKeyframeContainer())) {
                    for (size_t i = 0; i < keyframes->size(); ++i) {
                        channel.times.push_back((*keyframes)[i].getTime());
                        const osg::Quat& rot = (*keyframes)[i].getValue();
                        channel.values.push_back(static_cast<float>(rot.x()));
                        channel.values.push_back(static_cast<float>(rot.y()));
                        channel.values.push_back(static_cast<float>(rot.z()));
                        channel.values.push_back(static_cast<float>(rot.w()));
                    }
                }
            }
            else if (targetName.find("scale") != std::string::npos) {
                // 处理缩放动画
                channel.path = "scale";
                if (auto* keyframes = dynamic_cast<Vec3KeyframeContainer*>(sampler->getKeyframeContainer())) {
                    for (size_t i = 0; i < keyframes->size(); ++i) {
                        channel.times.push_back((*keyframes)[i].getTime());
                        const osg::Vec3d& scl = (*keyframes)[i].getValue();
                        channel.values.push_back(static_cast<float>(scl.x()));
                        channel.values.push_back(static_cast<float>(scl.y()));
                        channel.values.push_back(static_cast<float>(scl.z()));
                    }
                }
            }
        }
    }
    
    if (!channel.times.empty()) {
        _skeletonChannels.push_back(channel);
    }
}

void Osgb2B3dm::extractSkinningData(osgAnimation::RigGeometry* rigGeometry, SkinData& outSkinData) {
    if (!rigGeometry) return;

    // 获取顶点权重数组
    osg::Vec4Array* weights = dynamic_cast<osg::Vec4Array*>(
        rigGeometry->getVertexAttribArray(1));  // 权重通常存储在属性1中
    osg::Vec4Array* joints = dynamic_cast<osg::Vec4Array*>(
        rigGeometry->getVertexAttribArray(2));  // 关节索引通常存储在属性2中

    if (!weights || !joints) return;

    // 提取权重和关节索引
    for (size_t i = 0; i < weights->size(); ++i) {
        const osg::Vec4& w = (*weights)[i];
        const osg::Vec4& j = (*joints)[i];

        // 添加关节索引
        outSkinData.joints.push_back(static_cast<int>(j.x()));
        outSkinData.joints.push_back(static_cast<int>(j.y()));
        outSkinData.joints.push_back(static_cast<int>(j.z()));
        outSkinData.joints.push_back(static_cast<int>(j.w()));

        // 添加权重
        outSkinData.weights.push_back(w.x());
        outSkinData.weights.push_back(w.y());
        outSkinData.weights.push_back(w.z());
        outSkinData.weights.push_back(w.w());
    }
}

void Osgb2B3dm::processAnimationMixer(osg::Node* node) {
    if (!node) return;

    // 检查节点是否有动画混合器数据
    osg::UserDataContainer* userDataContainer = node->getUserDataContainer();
    if (userDataContainer) {
        for (unsigned int i = 0; i < userDataContainer->getNumUserObjects(); ++i) {
            const osg::Object* obj = userDataContainer->getUserObject(i);
            if (const osgAnimation::BasicAnimationManager* animManager = 
                dynamic_cast<const osgAnimation::BasicAnimationManager*>(obj)) {
                
                // 创建动画混合器
                AnimationMixer mixer;
                mixer.name = node->getName() + "_mixer";
                
                // 获取所有动画
                const osgAnimation::AnimationList& animations = animManager->getAnimationList();
                for (const auto& anim : animations) {
                    mixer.animations.push_back(anim->getName());
                    mixer.duration = std::max(mixer.duration, 
                        static_cast<float>(anim->getDuration()));
                }
                
                // 初始化权重
                mixer.weights.resize(mixer.animations.size(), 0.0f);
                if (!mixer.weights.empty()) {
                    mixer.weights[0] = 1.0f;  // 默认第一个动画权重为1
                }
                
                _animationMixers.push_back(mixer);
            }
        }
    }
}

void Osgb2B3dm::addSkeletonToGltf(nlohmann::json& gltf, const Skeleton& skeleton) {
    if (skeleton.joints.empty()) return;

    // 添加骨骼节点
    if (gltf.find("nodes") == gltf.end()) {
        gltf["nodes"] = nlohmann::json::array();
    }

    // 记录骨骼节点的起始索引
    int baseNodeIndex = gltf["nodes"].size();

    // 添加所有关节节点
    for (const auto& joint : skeleton.joints) {
        nlohmann::json node = {
            {"name", joint.name}
        };

        // 添加局部变换
        osg::Vec3 translation = matrixToTranslation(joint.localMatrix);
        osg::Quat rotation = matrixToQuaternion(joint.localMatrix);
        osg::Vec3 scale = matrixToScale(joint.localMatrix);

        node["translation"] = {translation.x(), translation.y(), translation.z()};
        node["rotation"] = {rotation.x(), rotation.y(), rotation.z(), rotation.w()};
        node["scale"] = {scale.x(), scale.y(), scale.z()};

        if (!joint.children.empty()) {
            node["children"] = nlohmann::json::array();
            for (int childIndex : joint.children) {
                node["children"].push_back(baseNodeIndex + childIndex);
            }
        }

        gltf["nodes"].push_back(node);
    }

    // 添加皮肤
    if (gltf.find("skins") == gltf.end()) {
        gltf["skins"] = nlohmann::json::array();
    }

    nlohmann::json skin = {
        {"name", skeleton.name},
        {"joints", nlohmann::json::array()}
    };

    // 添加关节索引
    for (size_t i = 0; i < skeleton.joints.size(); ++i) {
        skin["joints"].push_back(baseNodeIndex + i);
    }

    // 添加逆绑定矩阵
    std::vector<float> ibmData;
    for (const auto& joint : skeleton.joints) {
        const osg::Matrix& ibm = joint.inverseBindMatrix;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                ibmData.push_back(static_cast<float>(ibm(i,j)));
            }
        }
    }

    // 创建 IBM 缓冲视图和访问器
    if (gltf.find("bufferViews") == gltf.end()) {
        gltf["bufferViews"] = nlohmann::json::array();
    }
    if (gltf.find("accessors") == gltf.end()) {
        gltf["accessors"] = nlohmann::json::array();
    }

    int bufferViewIndex = gltf["bufferViews"].size();
    int accessorIndex = gltf["accessors"].size();

    gltf["bufferViews"].push_back({
        {"buffer", 0},
        {"byteOffset", 0},  // 需要在打包二进制数据时更新
        {"byteLength", ibmData.size() * sizeof(float)}
    });

    gltf["accessors"].push_back({
        {"bufferView", bufferViewIndex},
        {"componentType", 5126},  // FLOAT
        {"count", skeleton.joints.size()},
        {"type", "MAT4"}
    });

    skin["inverseBindMatrices"] = accessorIndex;
    gltf["skins"].push_back(skin);
}

void Osgb2B3dm::addSkeletonAnimationToGltf(nlohmann::json& gltf,
                                          const std::vector<SkeletonAnimationChannel>& channels) {
    if (channels.empty()) return;

    if (gltf.find("animations") == gltf.end()) {
        gltf["animations"] = nlohmann::json::array();
    }

    nlohmann::json animation = {
        {"name", "skeletonAnimation"},
        {"channels", nlohmann::json::array()},
        {"samplers", nlohmann::json::array()}
    };

    int samplerIndex = 0;
    for (const auto& channel : channels) {
        // 添加采样器
        animation["samplers"].push_back({
            {"input", createAccessor(gltf, channel.times, "SCALAR")},
            {"output", createAccessor(gltf, channel.values, 
                channel.path == "rotation" ? "VEC4" : "VEC3")},
            {"interpolation", channel.interpolation}
        });

        // 添加通道
        animation["channels"].push_back({
            {"sampler", samplerIndex},
            {"target", {
                {"node", findNodeIndex(gltf, channel.jointName)},
                {"path", channel.path}
            }}
        });

        samplerIndex++;
    }

    gltf["animations"].push_back(animation);
}

void Osgb2B3dm::addAnimationMixerToGltf(nlohmann::json& gltf,
                                       const std::vector<AnimationMixer>& mixers) {
    if (mixers.empty()) return;

    // 添加动画混合器扩展
    if (gltf.find("extensions") == gltf.end()) {
        gltf["extensions"] = nlohmann::json::object();
    }
    
    gltf["extensions"]["KHR_animation_mixer"] = nlohmann::json::array();

    for (const auto& mixer : mixers) {
        nlohmann::json mixerJson = {
            {"name", mixer.name},
            {"animations", mixer.animations},
            {"weights", mixer.weights},
            {"duration", mixer.duration},
            {"blendMode", mixer.blendMode}
        };
        
        gltf["extensions"]["KHR_animation_mixer"].push_back(mixerJson);
    }

    // 添加扩展到扩展列表
    if (gltf.find("extensionsUsed") == gltf.end()) {
        gltf["extensionsUsed"] = nlohmann::json::array();
    }
    if (std::find(gltf["extensionsUsed"].begin(), 
                  gltf["extensionsUsed"].end(), 
                  "KHR_animation_mixer") == gltf["extensionsUsed"].end()) {
        gltf["extensionsUsed"].push_back("KHR_animation_mixer");
    }
}

void Osgb2B3dm::addSkinningDataToGltf(nlohmann::json& gltf,
                                     const SkinData& skinData,
                                     const std::string& meshName) {
    if (skinData.joints.empty() || skinData.weights.empty()) return;

    // 找到对应的网格
    int meshIndex = -1;
    for (size_t i = 0; i < gltf["meshes"].size(); ++i) {
        if (gltf["meshes"][i]["name"] == meshName) {
            meshIndex = i;
            break;
        }
    }
    if (meshIndex == -1) return;

    // 添加关节索引和权重属性
    nlohmann::json& primitive = gltf["meshes"][meshIndex]["primitives"][0];
    
    // 创建关节索引访问器
    std::vector<float> jointIndices;
    jointIndices.reserve(skinData.joints.size());
    for (const auto& index : skinData.joints) {
        jointIndices.push_back(static_cast<float>(index));
    }
    int jointsAccessor = createAccessor(gltf, jointIndices, "VEC4", 5123);  // UNSIGNED_SHORT
    primitive["attributes"]["JOINTS_0"] = jointsAccessor;

    // 创建权重访问器
    int weightsAccessor = createAccessor(gltf, skinData.weights, "VEC4", 5126);  // FLOAT
    primitive["attributes"]["WEIGHTS_0"] = weightsAccessor;
}

// 辅助函数实现
osg::Quat Osgb2B3dm::matrixToQuaternion(const osg::Matrix& matrix) {
    osg::Quat quat;
    matrix.get(quat);
    return quat;
}

osg::Vec3 Osgb2B3dm::matrixToScale(const osg::Matrix& matrix) {
    return matrix.getScale();
}

osg::Vec3 Osgb2B3dm::matrixToTranslation(const osg::Matrix& matrix) {
    return matrix.getTrans();
}

void Osgb2B3dm::normalizeWeights(std::vector<float>& weights) {
    for (size_t i = 0; i < weights.size(); i += 4) {
        float sum = weights[i] + weights[i+1] + weights[i+2] + weights[i+3];
        if (sum > 0.0f) {
            weights[i] /= sum;
            weights[i+1] /= sum;
            weights[i+2] /= sum;
            weights[i+3] /= sum;
        }
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

    // 添加骨骼
    for (const auto& skeleton : _skeletons) {
        addSkeletonToGltf(gltf, skeleton);
    }

    // 添加骨骼动画
    addSkeletonAnimationToGltf(gltf, _skeletonChannels);

    // 添加动画混合器
    addAnimationMixerToGltf(gltf, _animationMixers);

    // 添加蒙皮数据
    for (size_t i = 0; i < _skinData.size(); ++i) {
        addSkinningDataToGltf(gltf, _skinData[i], "mesh_" + std::to_string(i));
    }

    // 添加实例
    addInstancesToGltf(gltf);

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

// 添加辅助函数
int Osgb2B3dm::createAccessor(nlohmann::json& gltf, 
                             const std::vector<float>& data, 
                             const std::string& type,
                             int componentType) {
    if (data.empty()) return -1;

    // 创建缓冲视图
    if (gltf.find("bufferViews") == gltf.end()) {
        gltf["bufferViews"] = nlohmann::json::array();
    }
    int bufferViewIndex = gltf["bufferViews"].size();

    gltf["bufferViews"].push_back({
        {"buffer", 0},
        {"byteOffset", 0},  // 需要在打包二进制数据时更新
        {"byteLength", data.size() * sizeof(float)}
    });

    // 创建访问器
    if (gltf.find("accessors") == gltf.end()) {
        gltf["accessors"] = nlohmann::json::array();
    }
    int accessorIndex = gltf["accessors"].size();

    nlohmann::json accessor = {
        {"bufferView", bufferViewIndex},
        {"componentType", componentType ? componentType : 5126},  // FLOAT
        {"count", type == "MAT4" ? data.size() / 16 : 
                 type == "VEC4" ? data.size() / 4 :
                 type == "VEC3" ? data.size() / 3 :
                 type == "VEC2" ? data.size() / 2 : data.size()},
        {"type", type}
    };

    // 添加最小值和最大值
    if (type != "MAT4") {
        std::vector<float> minValues, maxValues;
        int components = type == "VEC4" ? 4 :
                        type == "VEC3" ? 3 :
                        type == "VEC2" ? 2 : 1;
        
        for (int i = 0; i < components; ++i) {
            float minVal = std::numeric_limits<float>::max();
            float maxVal = std::numeric_limits<float>::lowest();
            
            for (size_t j = i; j < data.size(); j += components) {
                minVal = std::min(minVal, data[j]);
                maxVal = std::max(maxVal, data[j]);
            }
            
            minValues.push_back(minVal);
            maxValues.push_back(maxVal);
        }
        
        accessor["min"] = minValues;
        accessor["max"] = maxValues;
    }

    gltf["accessors"].push_back(accessor);
    return accessorIndex;
}

int Osgb2B3dm::findNodeIndex(const nlohmann::json& gltf, const std::string& nodeName) {
    if (gltf.find("nodes") == gltf.end()) return -1;

    for (size_t i = 0; i < gltf["nodes"].size(); ++i) {
        if (gltf["nodes"][i].find("name") != gltf["nodes"][i].end() &&
            gltf["nodes"][i]["name"] == nodeName) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void Osgb2B3dm::addMorphTargetsToGltf(nlohmann::json& gltf,
                                     const std::vector<MorphTarget>& morphTargets) {
    if (morphTargets.empty()) return;

    // 确保存在 meshes
    if (gltf.find("meshes") == gltf.end() || gltf["meshes"].empty()) return;

    // 获取第一个网格的第一个图元
    nlohmann::json& primitive = gltf["meshes"][0]["primitives"][0];

    // 添加变形目标
    primitive["targets"] = nlohmann::json::array();
    for (const auto& target : morphTargets) {
        nlohmann::json morphTarget;

        // 添加位置
        if (!target.positions.empty()) {
            morphTarget["POSITION"] = createAccessor(gltf, target.positions, "VEC3");
        }

        // 添加法线
        if (!target.normals.empty()) {
            morphTarget["NORMAL"] = createAccessor(gltf, target.normals, "VEC3");
        }

        primitive["targets"].push_back(morphTarget);
    }

    // 添加权重
    if (!morphTargets[0].weights.empty()) {
        gltf["meshes"][0]["weights"] = morphTargets[0].weights;
    }
}

void Osgb2B3dm::addMorphAnimationsToGltf(nlohmann::json& gltf,
                                        const std::vector<Animation>& animations) {
    if (animations.empty()) return;

    if (gltf.find("animations") == gltf.end()) {
        gltf["animations"] = nlohmann::json::array();
    }

    for (const auto& anim : animations) {
        nlohmann::json animation = {
            {"name", anim.name},
            {"channels", nlohmann::json::array()},
            {"samplers", nlohmann::json::array()}
        };

        // 添加采样器和通道
        for (size_t i = 0; i < anim.channels.size() && i < anim.samplers.size(); ++i) {
            const std::string& channelId = anim.channels[i];
            const AnimationSampler& sampler = _animationSamplers[i];

            // 添加采样器
            int samplerIndex = animation["samplers"].size();
            animation["samplers"].push_back({
                {"input", createAccessor(gltf, sampler.input, "SCALAR")},
                {"output", createAccessor(gltf, sampler.output, "SCALAR")},
                {"interpolation", sampler.interpolation}
            });

            // 添加通道
            animation["channels"].push_back({
                {"sampler", samplerIndex},
                {"target", {
                    {"node", 0},  // 假设目标是第一个节点
                    {"path", "weights"}
                }}
            });
        }

        gltf["animations"].push_back(animation);
    }
}

void Osgb2B3dm::addAnimationEventsToGltf(nlohmann::json& gltf,
                                        const std::vector<AnimationEvent>& events) {
    if (events.empty()) return;

    // 添加事件扩展
    if (gltf.find("extensions") == gltf.end()) {
        gltf["extensions"] = nlohmann::json::object();
    }

    gltf["extensions"]["KHR_animation_events"] = {
        {"events", nlohmann::json::array()}
    };

    // 添加事件
    for (const auto& event : events) {
        gltf["extensions"]["KHR_animation_events"]["events"].push_back({
            {"name", event.name},
            {"time", event.time},
            {"type", event.type},
            {"data", event.data}
        });
    }

    // 添加扩展到扩展列表
    if (gltf.find("extensionsUsed") == gltf.end()) {
        gltf["extensionsUsed"] = nlohmann::json::array();
    }
    if (std::find(gltf["extensionsUsed"].begin(),
                  gltf["extensionsUsed"].end(),
                  "KHR_animation_events") == gltf["extensionsUsed"].end()) {
        gltf["extensionsUsed"].push_back("KHR_animation_events");
    }
}

void Osgb2B3dm::extractInstances(osg::Node* node) {
    if (!node) return;

    // 检查是否是实例化组
    if (osg::Group* group = node->asGroup()) {
        processInstanceGroup(group, node->getName());
    }

    // 递归处理子节点
    osg::Group* group = node->asGroup();
    if (group) {
        for (unsigned int i = 0; i < group->getNumChildren(); ++i) {
            extractInstances(group->getChild(i));
        }
    }
}

void Osgb2B3dm::processInstanceGroup(osg::Group* group, const std::string& name) {
    if (!group) return;

    // 检查是否是实例化组
    bool isInstanceGroup = false;
    osg::UserDataContainer* userData = group->getUserDataContainer();
    if (userData) {
        for (unsigned int i = 0; i < userData->getNumUserObjects(); ++i) {
            const osg::Object* obj = userData->getUserObject(i);
            if (obj && obj->getName() == "InstanceGroup") {
                isInstanceGroup = true;
                break;
            }
        }
    }

    if (!isInstanceGroup) return;

    // 创建实例化组
    InstanceGroup instanceGroup;
    instanceGroup.name = name;

    // 获取实例组的变换矩阵
    osg::MatrixList matrices = group->getWorldMatrices();
    if (!matrices.empty()) {
        instanceGroup.baseTransform = matrices[0];
    }

    // 遍历子节点
    for (unsigned int i = 0; i < group->getNumChildren(); ++i) {
        osg::Node* child = group->getChild(i);
        Instance instance;
        
        // 获取实例的变换矩阵
        osg::MatrixList childMatrices = child->getWorldMatrices();
        if (!childMatrices.empty()) {
            instance.transform = childMatrices[0];
        }

        // 检查是否是几何体节点
        if (osg::Geode* geode = child->asGeode()) {
            for (unsigned int j = 0; j < geode->getNumDrawables(); ++j) {
                osg::Geometry* geom = geode->getDrawable(j)->asGeometry();
                if (geom) {
                    // 提取几何体数据
                    std::vector<float> positions;
                    std::vector<float> normals;
                    std::vector<float> texcoords;
                    std::vector<unsigned short> indices;
                    osg::BoundingBox bbox;
                    extractGeometry(geom, positions, normals, texcoords, indices, bbox, instance.transform);

                    // 创建网格
                    nlohmann::json gltf = generateGltfJson(positions, normals, texcoords, indices, bbox, {});
                    instance.meshIndex = gltf["meshes"].size() - 1;

                    // 提取材质
                    if (geom->getStateSet()) {
                        Material material;
                        extractMaterialProperties(geom->getStateSet(), material);
                        _materials.push_back(material);
                        instance.materialIndex = _materials.size() - 1;
                    }
                }
            }
        }

        instanceGroup.instances.push_back(instance);
    }

    _instanceGroups.push_back(instanceGroup);
}

void Osgb2B3dm::addInstancesToGltf(nlohmann::json& gltf) {
    if (_instanceGroups.empty()) return;

    // 添加实例化扩展
    if (gltf.find("extensions") == gltf.end()) {
        gltf["extensions"] = nlohmann::json::object();
    }

    gltf["extensions"]["EXT_mesh_gpu_instancing"] = nlohmann::json::object();

    // 添加实例化组
    for (const auto& group : _instanceGroups) {
        addInstanceNodesToGltf(gltf, group);
        addInstanceMeshesToGltf(gltf, group);
    }

    // 添加扩展到扩展列表
    if (gltf.find("extensionsUsed") == gltf.end()) {
        gltf["extensionsUsed"] = nlohmann::json::array();
    }
    if (std::find(gltf["extensionsUsed"].begin(),
                  gltf["extensionsUsed"].end(),
                  "EXT_mesh_gpu_instancing") == gltf["extensionsUsed"].end()) {
        gltf["extensionsUsed"].push_back("EXT_mesh_gpu_instancing");
    }
}

void Osgb2B3dm::addInstanceNodesToGltf(nlohmann::json& gltf, const InstanceGroup& group) {
    if (group.instances.empty()) return;

    // 确保存在节点数组
    if (gltf.find("nodes") == gltf.end()) {
        gltf["nodes"] = nlohmann::json::array();
    }

    // 创建实例化组节点
    nlohmann::json groupNode = {
        {"name", group.name},
        {"matrix", nlohmann::json::array()}
    };

    // 添加基础变换矩阵
    const osg::Matrix& mat = group.baseTransform;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            groupNode["matrix"].push_back(mat(i,j));
        }
    }

    // 添加实例节点
    groupNode["children"] = nlohmann::json::array();
    for (const auto& instance : group.instances) {
        nlohmann::json instanceNode = {
            {"name", instance.name},
            {"mesh", instance.meshIndex},
            {"matrix", nlohmann::json::array()}
        };

        // 添加变换矩阵
        const osg::Matrix& mat = instance.transform;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                instanceNode["matrix"].push_back(mat(i,j));
            }
        }

        // 添加材质
        if (instance.materialIndex >= 0) {
            instanceNode["material"] = instance.materialIndex;
        }

        // 添加子节点
        if (!instance.children.empty()) {
            instanceNode["children"] = instance.children;
        }

        groupNode["children"].push_back(gltf["nodes"].size());
        gltf["nodes"].push_back(instanceNode);
    }

    gltf["nodes"].push_back(groupNode);
}

void Osgb2B3dm::addInstanceMeshesToGltf(nlohmann::json& gltf, const InstanceGroup& group) {
    if (group.instances.empty()) return;

    // 确保存在网格数组
    if (gltf.find("meshes") == gltf.end()) {
        gltf["meshes"] = nlohmann::json::array();
    }

    // 为每个实例创建网格
    for (const auto& instance : group.instances) {
        if (instance.meshIndex < 0) continue;

        nlohmann::json mesh = {
            {"name", instance.name + "_instance"},
            {"primitives", nlohmann::json::array()}
        };

        // 添加图元
        nlohmann::json primitive = {
            {"attributes", {
                {"POSITION", 0},
                {"NORMAL", 1},
                {"TEXCOORD_0", 2}
            }},
            {"indices", 3},
            {"mode", 4}  // TRIANGLES
        };

        // 添加材质
        if (instance.materialIndex >= 0) {
            primitive["material"] = instance.materialIndex;
        }

        // 添加实例化属性
        primitive["extensions"] = {
            {"EXT_mesh_gpu_instancing", {
                {"attributes", {
                    {"TRANSLATION", 4},
                    {"ROTATION", 5},
                    {"SCALE", 6}
                }}
            }}
        };

        mesh["primitives"].push_back(primitive);
        gltf["meshes"].push_back(mesh);
    }
}
}