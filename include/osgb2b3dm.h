#ifndef OSGB2B3DM_H
#define OSGB2B3DM_H

#include <string>
#include <vector>
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
#include <nlohmann/json.hpp>

class Osgb2B3dm {
public:
    Osgb2B3dm();
    ~Osgb2B3dm();

    // 转换入口函数
    bool convert(const std::string& inputPath, const std::string& outputPath);

private:
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

    // 动画结构体
    struct Animation {
        std::string name;
        double duration = 0.0;
        std::vector<std::string> channels;
        std::vector<std::string> samplers;
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

    std::vector<Material> _materials;  // 存储提取的材质
    std::vector<Animation> _animations; // 存储提取的动画
    std::vector<AnimationChannel> _animationChannels; // 存储动画通道
    std::vector<AnimationSampler> _animationSamplers; // 存储动画采样器
    std::vector<MorphTarget> _morphTargets; // 存储变形目标
    std::vector<AnimationEvent> _animationEvents; // 存储动画事件
};

#endif // OSGB2B3DM_H