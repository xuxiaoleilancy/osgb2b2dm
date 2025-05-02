#ifndef OSGB2B3DM_H
#define OSGB2B3DM_H

#include <string>
#include <vector>
#include <osg/Node>
#include <osg/Geometry>
#include <osg/Texture>
#include <osg/BoundingBox>
#include <osg/Matrix>
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
        std::vector<float> ambient = {0.2f, 0.2f, 0.2f, 1.0f};
        std::vector<float> diffuse = {0.8f, 0.8f, 0.8f, 1.0f};
        std::vector<float> specular = {0.0f, 0.0f, 0.0f, 1.0f};
        std::vector<float> emission = {0.0f, 0.0f, 0.0f, 1.0f};
        float shininess = 0.0f;
        bool doubleSided = false;
    };

    // 读取 OSGB 文件
    osg::ref_ptr<osg::Node> readOsgb(const std::string& path);

    // 提取几何数据
    void extractNodeData(osg::Node* node,
                        std::vector<float>& positions,
                        std::vector<float>& normals,
                        std::vector<float>& texcoords,
                        std::vector<unsigned short>& indices,
                        osg::BoundingBox& bbox,
                        std::vector<std::string>& texturePaths,
                        const osg::Matrix& parentMatrix = osg::Matrix::identity());

    void extractGeometry(osg::Geometry* geom,
                        std::vector<float>& positions,
                        std::vector<float>& normals,
                        std::vector<float>& texcoords,
                        std::vector<unsigned short>& indices,
                        osg::BoundingBox& bbox,
                        const osg::Matrix& matrix);

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

    std::vector<Material> _materials;  // 存储提取的材质
};

#endif // OSGB2B3DM_H