#include "osgb2b3dm.h"
#include <iostream>
#include <string>
#include <osg/Node>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

/**
 * 实例化处理示例
 * 展示了如何创建和处理实例化模型
 */

// 创建一个简单的实例化场景
osg::ref_ptr<osg::Node> createInstancedScene() {
    // 创建根节点
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->setName("InstanceGroup");

    // 创建一些实例
    const int numInstances = 5;
    for (int i = 0; i < numInstances; ++i) {
        // 创建变换节点
        osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;
        transform->setName("Instance_" + std::to_string(i));

        // 设置变换矩阵（这里简单地在X轴上平移）
        osg::Matrix matrix;
        matrix.makeTranslate(i * 2.0, 0.0, 0.0);  // 每个实例间隔2个单位
        transform->setMatrix(matrix);

        // 添加到根节点
        root->addChild(transform);
    }

    return root.release();
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "用法: " << argv[0] << " <input.osgb> <output.b3dm>" << std::endl;
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputPath = argv[2];

    // 读取输入模型
    std::cout << "读取OSGB文件: " << inputPath << std::endl;
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(inputPath);
    
    if (!model) {
        std::cerr << "无法读取OSGB文件" << std::endl;
        return 1;
    }

    // 创建实例化场景
    osg::ref_ptr<osg::Group> instancedScene = new osg::Group;
    osg::ref_ptr<osg::Node> instances = createInstancedScene();
    
    // 将实例添加到场景中
    for (unsigned int i = 0; i < instances->asGroup()->getNumChildren(); ++i) {
        osg::MatrixTransform* transform = 
            dynamic_cast<osg::MatrixTransform*>(instances->asGroup()->getChild(i));
        if (transform) {
            transform->addChild(model);
        }
    }
    instancedScene->addChild(instances);

    // 保存临时OSGB文件
    std::string tempFile = "temp_instanced.osgb";
    if (!osgDB::writeNodeFile(*instancedScene, tempFile)) {
        std::cerr << "无法保存实例化场景" << std::endl;
        return 1;
    }

    // 创建转换器实例
    Osgb2B3dm converter;

    // 执行转换
    std::cout << "\n开始转换..." << std::endl;
    bool success = converter.convert(tempFile, outputPath);

    // 清理临时文件
    std::remove(tempFile.c_str());

    if (success) {
        std::cout << "转换成功!" << std::endl;
        std::cout << "注意: B3DM文件中的实例化可以在Cesium中使用 3D Tiles 实例化功能显示" << std::endl;
        return 0;
    } else {
        std::cerr << "转换失败!" << std::endl;
        return 1;
    }
} 