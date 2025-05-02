#include "osgb2b3dm.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <osg/Node>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

using namespace osgb2b3dm;
namespace fs = std::filesystem;

/**
 * 实例化处理示例
 * 展示了如何创建和处理实例化模型
 */

// 创建一个简单的实例化场景
osg::ref_ptr<osg::Node> createInstancedScene() {
    osg::ref_ptr<osg::Group> root = new osg::Group;
    
    // 创建一个基础变换组
    osg::ref_ptr<osg::MatrixTransform> baseTransform = new osg::MatrixTransform;
    baseTransform->setMatrix(osg::Matrix::translate(0.0f, 0.0f, 0.0f));
    root->addChild(baseTransform);

    // 添加实例
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            osg::ref_ptr<osg::MatrixTransform> instance = new osg::MatrixTransform;
            instance->setMatrix(osg::Matrix::translate(i * 2.0f, j * 2.0f, 0.0f));
            baseTransform->addChild(instance);
        }
    }

    return root;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <output.b3dm>" << std::endl;
        return 1;
    }

    // 创建临时 OSGB 文件
    fs::path temp_osgb = fs::temp_directory_path() / "temp.osgb";
    
    // 创建实例化场景并保存为 OSGB
    osg::ref_ptr<osg::Node> scene = createInstancedScene();
    if (!osgDB::writeNodeFile(*scene, temp_osgb.string())) {
        std::cerr << "Error: Failed to write temporary OSGB file" << std::endl;
        return 1;
    }

    // 转换为 B3DM
    Osgb2B3dm converter;
    converter.setVerbose(true);

    if (!converter.convert(temp_osgb.string(), argv[1])) {
        std::cerr << "Error: " << converter.lastErrorMessage() << std::endl;
        fs::remove(temp_osgb);
        return 1;
    }

    // 清理临时文件
    fs::remove(temp_osgb);

    std::cout << "Successfully created instanced B3DM file!" << std::endl;
    return 0;
} 