#include <gtest/gtest.h>
#include "osgb2b3dm.h"
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <filesystem>

class Osgb2B3dmTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时目录用于测试
        tempDir = std::filesystem::temp_directory_path() / "osgb2b3dm_test";
        std::filesystem::create_directories(tempDir);
    }

    void TearDown() override {
        // 清理临时文件
        std::filesystem::remove_all(tempDir);
    }

    std::filesystem::path tempDir;
};

// 测试基本转换功能
TEST_F(Osgb2B3dmTest, BasicConversion) {
    // 创建一个简单的OSG场景
    osg::ref_ptr<osg::Group> root = new osg::Group;
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    
    // 创建一个简单的三角形
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(0, 0, 0));
    vertices->push_back(osg::Vec3(1, 0, 0));
    vertices->push_back(osg::Vec3(0, 1, 0));
    geom->setVertexArray(vertices);

    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
    indices->push_back(0);
    indices->push_back(1);
    indices->push_back(2);
    geom->addPrimitiveSet(indices);

    geode->addDrawable(geom);
    root->addChild(geode);

    // 保存为临时OSGB文件
    std::string inputPath = (tempDir / "test.osgb").string();
    osgDB::writeNodeFile(*root, inputPath);

    // 创建转换器
    Osgb2B3dm converter;
    std::string outputPath = (tempDir / "test.b3dm").string();

    // 执行转换
    bool result = converter.convert(inputPath, outputPath);

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(outputPath));
    EXPECT_GT(std::filesystem::file_size(outputPath), 0);
}

// 测试空场景转换
TEST_F(Osgb2B3dmTest, EmptyScene) {
    // 创建一个空场景
    osg::ref_ptr<osg::Group> root = new osg::Group;
    
    // 保存为临时OSGB文件
    std::string inputPath = (tempDir / "empty.osgb").string();
    osgDB::writeNodeFile(*root, inputPath);

    // 创建转换器
    Osgb2B3dm converter;
    std::string outputPath = (tempDir / "empty.b3dm").string();

    // 执行转换
    bool result = converter.convert(inputPath, outputPath);

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(outputPath));
    EXPECT_GT(std::filesystem::file_size(outputPath), 0);
}

// 测试无效输入文件
TEST_F(Osgb2B3dmTest, InvalidInput) {
    Osgb2B3dm converter;
    std::string outputPath = (tempDir / "invalid.b3dm").string();
    
    // 尝试转换不存在的文件
    bool result = converter.convert("nonexistent.osgb", outputPath);
    
    // 验证结果
    EXPECT_FALSE(result);
    EXPECT_FALSE(std::filesystem::exists(outputPath));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 