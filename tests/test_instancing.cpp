#include <gtest/gtest.h>
#include "osgb2b3dm.h"
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/MatrixTransform>
#include <filesystem>

class InstancingTest : public ::testing::Test {
protected:
    void SetUp() override {
        tempDir = std::filesystem::temp_directory_path() / "osgb2b3dm_instancing_test";
        std::filesystem::create_directories(tempDir);
    }

    void TearDown() override {
        std::filesystem::remove_all(tempDir);
    }

    std::filesystem::path tempDir;
};

// 测试基本实例化
TEST_F(InstancingTest, BasicInstancing) {
    // 创建基础几何体
    osg::ref_ptr<osg::Geode> baseGeode = new osg::Geode;
    osg::ref_ptr<osg::Geometry> baseGeom = new osg::Geometry;
    
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(0, 0, 0));
    vertices->push_back(osg::Vec3(1, 0, 0));
    vertices->push_back(osg::Vec3(0, 1, 0));
    baseGeom->setVertexArray(vertices);

    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
    indices->push_back(0);
    indices->push_back(1);
    indices->push_back(2);
    baseGeom->addPrimitiveSet(indices);

    baseGeode->addDrawable(baseGeom);

    // 创建实例组
    osg::ref_ptr<osg::Group> instanceGroup = new osg::Group;
    instanceGroup->setName("instance_group");
    instanceGroup->addChild(baseGeode);

    // 添加实例
    for (int i = 0; i < 3; ++i) {
        osg::ref_ptr<osg::MatrixTransform> instance = new osg::MatrixTransform;
        instance->setName("instance_" + std::to_string(i));
        instance->setMatrix(osg::Matrix::translate(i * 2.0, 0.0, 0.0));
        instanceGroup->addChild(instance);
    }

    // 保存为临时OSGB文件
    std::string inputPath = (tempDir / "instanced.osgb").string();
    osgDB::writeNodeFile(*instanceGroup, inputPath);

    // 创建转换器
    Osgb2B3dm converter;
    std::string outputPath = (tempDir / "instanced.b3dm").string();

    // 执行转换
    bool result = converter.convert(inputPath, outputPath);

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(outputPath));
    EXPECT_GT(std::filesystem::file_size(outputPath), 0);
}

// 测试多层实例化
TEST_F(InstancingTest, NestedInstancing) {
    // 创建基础几何体
    osg::ref_ptr<osg::Geode> baseGeode = new osg::Geode;
    osg::ref_ptr<osg::Geometry> baseGeom = new osg::Geometry;
    
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(0, 0, 0));
    vertices->push_back(osg::Vec3(1, 0, 0));
    vertices->push_back(osg::Vec3(0, 1, 0));
    baseGeom->setVertexArray(vertices);

    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
    indices->push_back(0);
    indices->push_back(1);
    indices->push_back(2);
    baseGeom->addPrimitiveSet(indices);

    baseGeode->addDrawable(baseGeom);

    // 创建嵌套的实例组
    osg::ref_ptr<osg::Group> rootGroup = new osg::Group;
    rootGroup->setName("root_group");

    // 第一层实例组
    for (int i = 0; i < 2; ++i) {
        osg::ref_ptr<osg::Group> instanceGroup = new osg::Group;
        instanceGroup->setName("instance_group_" + std::to_string(i));
        instanceGroup->addChild(baseGeode);

        // 第二层实例
        for (int j = 0; j < 2; ++j) {
            osg::ref_ptr<osg::MatrixTransform> instance = new osg::MatrixTransform;
            instance->setName("instance_" + std::to_string(i) + "_" + std::to_string(j));
            instance->setMatrix(osg::Matrix::translate(i * 2.0, j * 2.0, 0.0));
            instanceGroup->addChild(instance);
        }

        rootGroup->addChild(instanceGroup);
    }

    // 保存为临时OSGB文件
    std::string inputPath = (tempDir / "nested_instanced.osgb").string();
    osgDB::writeNodeFile(*rootGroup, inputPath);

    // 创建转换器
    Osgb2B3dm converter;
    std::string outputPath = (tempDir / "nested_instanced.b3dm").string();

    // 执行转换
    bool result = converter.convert(inputPath, outputPath);

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(outputPath));
    EXPECT_GT(std::filesystem::file_size(outputPath), 0);
} 