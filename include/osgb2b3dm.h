#pragma once

#include <string>
#include <vector>
#include <map>
#include <limits>
#include <osg/Node>
#include <osg/Geometry>
#include <osg/Texture>
#include <osg/BoundingBox>
#include <osg/Matrix>
#include <osg/TexMat>
#include <osg/AnimationPath>
#include <osg/MatrixTransform>
#include <osgAnimation/MorphGeometry>
#include <osgAnimation/AnimationUpdateCallback>
#include <osgAnimation/Skeleton>
#include <osgAnimation/Bone>
#include <osgAnimation/RigGeometry>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/Animation>
#include <osgAnimation/Channel>
#include <osgAnimation/Sampler>
#include <nlohmann/json.hpp>
#include <memory>
#include "osgb2b3dm_error.h"

namespace {
    // 骨骼动画相关类型定义
    typedef osgAnimation::TemplateKeyframeContainer<osg::Vec3d> Vec3KeyframeContainer;
    typedef osgAnimation::TemplateKeyframeContainer<osg::Quat> QuatKeyframeContainer;
    typedef osgAnimation::TemplateSphericalLinearInterpolator<osg::Quat> QuatInterpolator;
    typedef osgAnimation::TemplateLinearInterpolator<osg::Vec3d> Vec3Interpolator;
}

namespace osgb2b3dm {

// 前向声明
class Osgb2B3dm;

// 动画结构体
struct Animation {
    std::string name;
    double duration = 0.0;
    std::vector<std::string> channels;
    std::vector<std::string> samplers;
};

// 实例化结构体
struct Instance {
    std::string name;
    osg::Matrix transform;
    int meshIndex = -1;
    int materialIndex = -1;
    std::vector<int> children;
    std::vector<std::string> userData;
};

struct InstanceGroup {
    std::string name;
    std::vector<Instance> instances;
    osg::Matrix baseTransform;
    std::vector<std::string> userData;
};

class Osgb2B3dm {
public:
    Osgb2B3dm();
    ~Osgb2B3dm();

    // 转换函数
    bool convert(const std::string& inputPath, const std::string& outputPath);

    // 获取错误信息
    std::error_code lastError() const { return lastError_; }
    std::string lastErrorMessage() const { return lastErrorMessage_; }

    // 设置选项
    void setVerbose(bool verbose) { verbose_ = verbose; }
    void setValidateInput(bool validate) { validateInput_ = validate; }
    void setValidateOutput(bool validate) { validateOutput_ = validate; }

    // 测试用函数
    bool processScene();
    bool validateInput();
    bool validateOutput();

    // 测试用数据设置函数
    void addAnimation(const Animation& anim) { _animations.push_back(anim); }
    void addInstanceGroup(const InstanceGroup& group) { _instanceGroups.push_back(group); }

protected:
    // 内部实现
    bool loadInputFile(const std::string& path);
    bool saveOutputFile(const std::string& path);

    // 错误处理
    void setError(ErrorCode code, const std::string& message = "");
    void clearError();

private:
    // 成员变量
    std::error_code lastError_;
    std::string lastErrorMessage_;
    bool verbose_ = false;
    bool validateInput_ = true;
    bool validateOutput_ = true;

    // 材质结构体
    struct Material {
        // 基础颜色属性
        std::vector<float> ambient = {0.2f, 0.2f, 0.2f, 1.0f};
        std::vector<float> diffuse = {0.8f, 0.8f, 0.8f, 1.0f};
        std::vector<float> specular = {0.0f, 0.0f, 0.0f, 1.0f};
        std::vector<float> emission = {0.0f, 0.0f, 0.0f, 1.0f};
        
        // PBR 材质属性
        float metallic = 0.0f;
        float roughness = 1.0f;
        float occlusion = 1.0f;
        float emissiveIntensity = 1.0f;
        
        // 透明度和混合
        float opacity = 1.0f;
        std::string blendMode = "OPAQUE"; // OPAQUE, MASK, BLEND
        
        // 双面渲染
        bool doubleSided = false;
        
        // 纹理信息
        struct TextureInfo {
            std::string path;
            int texCoord = 0;
            float scale = 1.0f;
            std::vector<float> offset = {0.0f, 0.0f};
            std::vector<float> rotation = {0.0f, 0.0f, 0.0f};
        };
        
        // 各种贴图
        TextureInfo baseColorTexture;
        TextureInfo normalTexture;
        TextureInfo metallicRoughnessTexture;
        TextureInfo occlusionTexture;
        TextureInfo emissiveTexture;
        
        // 法线贴图强度
        float normalScale = 1.0f;
        
        // 材质名称
        std::string name = "default";
    };

    // 动画通道结构体
    struct AnimationChannel {
        std::string targetNode;
        std::string path; // translation, rotation, scale, weights
        std::vector<float> times;
        std::vector<float> values;
        std::string interpolation = "LINEAR";
    };

    // 动画采样器结构体
    struct AnimationSampler {
        std::vector<float> input;  // 时间点
        std::vector<float> output; // 值
        std::string interpolation = "LINEAR";
    };

    // 变形目标结构体
    struct MorphTarget {
        std::string name;
        std::vector<float> positions;
        std::vector<float> normals;
        std::vector<float> weights;
    };

    // 动画事件结构体
    struct AnimationEvent {
        std::string name;
        double time;
        std::string type;
        nlohmann::json data;
    };

    // 骨骼结构体
    struct Joint {
        std::string name;
        int parentIndex = -1;
        osg::Matrix inverseBindMatrix;
        osg::Matrix localMatrix;
        std::vector<int> children;
        osgAnimation::Bone* bone = nullptr;  // 对应的OSG骨骼节点
    };

    // 骨骼动画通道
    struct SkeletonAnimationChannel {
        std::string jointName;
        std::string path;  // translation, rotation, scale
        std::vector<float> times;
        std::vector<float> values;
        std::string interpolation = "LINEAR";
    };

    // 骨骼结构
    struct Skeleton {
        std::string name;
        std::vector<Joint> joints;
        int rootJoint = -1;
        std::vector<osg::Matrix> inverseBindMatrices;
        osgAnimation::Skeleton* osgSkeleton = nullptr;  // 对应的OSG骨骼
    };

    // 动画混合器
    struct AnimationMixer {
        std::string name;
        std::vector<std::string> animations;
        std::vector<float> weights;
        float duration = 0.0f;
        std::string blendMode = "ADDITIVE";  // ADDITIVE or OVERRIDE
    };

    // 蒙皮数据
    struct SkinData {
        std::vector<int> joints;
        std::vector<float> weights;
        int maxInfluences = 4;  // 每个顶点最大影响骨骼数
    };

    // 读取 OSGB 文件
    osg::ref_ptr<osg::Node> readOsgb(const std::string& path);

    // 提取几何数据
    void extractGeometry(osg::Geometry* geom,
                        std::vector<float>& positions,
                        std::vector<float>& normals,
                        std::vector<float>& texcoords,
                        std::vector<unsigned short>& indices,
                        osg::BoundingBox& bbox,
                        const osg::Matrix& matrix);

    // 提取变形动画数据
    void extractMorphData(osg::Node* node);
    void processMorphGeometry(osgAnimation::MorphGeometry* morphGeom);
    void extractMorphAnimation(osg::Node* node, const std::string& nodeName);

    // 提取动画事件
    void extractAnimationEvents(osg::Node* node);
    void processAnimationCallback(osg::Node* node, const std::string& nodeName);

    // 添加变形目标到glTF
    void addMorphTargetsToGltf(nlohmann::json& gltf, 
                              const std::vector<MorphTarget>& morphTargets);

    // 添加变形动画到glTF
    void addMorphAnimationsToGltf(nlohmann::json& gltf,
                                 const std::vector<Animation>& animations);

    // 添加动画事件到glTF
    void addAnimationEventsToGltf(nlohmann::json& gltf,
                                 const std::vector<AnimationEvent>& events);

    // 生成 glTF JSON
    nlohmann::json generateGltfJson(const std::vector<float>& positions,
                                   const std::vector<float>& normals,
                                   const std::vector<float>& texcoords,
                                   const std::vector<unsigned short>& indices,
                                   const osg::BoundingBox& bbox,
                                   const std::vector<std::string>& texturePaths);

    // 生成 B3DM 文件
    bool writeB3dm(const std::string& path, const nlohmann::json& gltfJson,
                  const std::vector<unsigned char>& binaryData);

    // 打包二进制数据
    std::vector<unsigned char> packBinaryData(const std::vector<float>& positions,
                                            const std::vector<float>& normals,
                                            const std::vector<float>& texcoords,
                                            const std::vector<unsigned short>& indices);

    // 提取材质属性
    void extractMaterialProperties(osg::StateSet* stateSet, Material& material);
    
    // 提取纹理信息
    void extractTextureInfo(osg::StateSet* stateSet, Material::TextureInfo& textureInfo, 
                          osg::StateAttribute::Type type, int unit);

    // 辅助函数：获取纹理索引
    int getTextureIndex(const std::vector<std::string>& texturePaths, 
                       const std::string& path);

    // 骨骼动画相关函数
    void extractSkeletonData(osg::Node* node);
    void processSkeletonAnimation(osg::Node* node, const std::string& nodeName);
    void extractJointHierarchy(osgAnimation::Skeleton* skeleton, Skeleton& outSkeleton);
    void processAnimationMixer(osg::Node* node);
    void extractSkinningData(osgAnimation::RigGeometry* rigGeometry, SkinData& outSkinData);
    void processBoneAnimation(osgAnimation::Bone* bone, const std::string& skeletonName);
    
    // 添加骨骼动画到glTF
    void addSkeletonToGltf(nlohmann::json& gltf, const Skeleton& skeleton);
    void addSkeletonAnimationToGltf(nlohmann::json& gltf, 
                                   const std::vector<SkeletonAnimationChannel>& channels);
    void addAnimationMixerToGltf(nlohmann::json& gltf, 
                                const std::vector<AnimationMixer>& mixers);
    void addSkinningDataToGltf(nlohmann::json& gltf, 
                              const SkinData& skinData,
                              const std::string& meshName);

    // 辅助函数
    int createAccessor(nlohmann::json& gltf, 
                      const std::vector<float>& data, 
                      const std::string& type,
                      int componentType = 0);
    int findNodeIndex(const nlohmann::json& gltf, const std::string& nodeName);
    osg::Quat matrixToQuaternion(const osg::Matrix& matrix);
    osg::Vec3 matrixToScale(const osg::Matrix& matrix);
    osg::Vec3 matrixToTranslation(const osg::Matrix& matrix);
    void normalizeWeights(std::vector<float>& weights);

    // 实例化相关函数
    void extractInstances(osg::Node* node);
    void processInstanceGroup(osg::Group* group, const std::string& name);
    void addInstancesToGltf(nlohmann::json& gltf);
    void addInstanceNodesToGltf(nlohmann::json& gltf, const InstanceGroup& group);
    void addInstanceMeshesToGltf(nlohmann::json& gltf, const InstanceGroup& group);

    std::vector<Material> _materials;  // 存储提取的材质
    std::vector<Animation> _animations; // 存储提取的动画
    std::vector<AnimationChannel> _animationChannels; // 存储动画通道
    std::vector<AnimationSampler> _animationSamplers; // 存储动画采样器
    std::vector<MorphTarget> _morphTargets; // 存储变形目标
    std::vector<AnimationEvent> _animationEvents; // 存储动画事件
    std::vector<Skeleton> _skeletons;
    std::vector<SkeletonAnimationChannel> _skeletonChannels;
    std::vector<AnimationMixer> _animationMixers;
    std::vector<SkinData> _skinData;
    std::vector<InstanceGroup> _instanceGroups;  // 存储实例化组
};

} // namespace osgb2b3dm