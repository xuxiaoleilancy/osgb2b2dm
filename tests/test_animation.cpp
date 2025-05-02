#include <gtest/gtest.h>
#include "osgb2b3dm.h"
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgAnimation/BasicAnimationManager>
#include <osgAnimation/Animation>
#include <osgAnimation/Channel>
#include <osgAnimation/Sampler>
#include <filesystem>

class AnimationTest : public ::testing::Test {
protected:
    void SetUp() override {
        tempDir = std::filesystem::temp_directory_path() / "osgb2b3dm_animation_test";
        std::filesystem::create_directories(tempDir);
    }

    void TearDown() override {
        std::filesystem::remove_all(tempDir);
    }

    std::filesystem::path tempDir;
};

// 测试基本动画转换
TEST_F(AnimationTest, BasicAnimation) {
    // 创建带有动画的场景
    osg::ref_ptr<osg::Group> root = new osg::Group;
    osg::ref_ptr<osgAnimation::BasicAnimationManager> animManager = new osgAnimation::BasicAnimationManager;
    root->setUpdateCallback(animManager);

    // 创建一个简单的几何体
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    
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

    // 创建动画
    osg::ref_ptr<osgAnimation::Animation> animation = new osgAnimation::Animation;
    animation->setName("test_animation");
    animation->setPlayMode(osgAnimation::Animation::LOOP);

    // 创建位置通道
    osg::ref_ptr<osgAnimation::Vec3LinearSampler> sampler = new osgAnimation::Vec3LinearSampler;
    osg::ref_ptr<osgAnimation::Vec3KeyframeContainer> keyframes = new osgAnimation::Vec3KeyframeContainer;
    keyframes->push_back(osgAnimation::Vec3Keyframe(0.0, osg::Vec3(0, 0, 0)));
    keyframes->push_back(osgAnimation::Vec3Keyframe(1.0, osg::Vec3(1, 1, 1)));
    sampler->setKeyframeContainer(keyframes);

    osg::ref_ptr<osgAnimation::Vec3LinearChannel> channel = new osgAnimation::Vec3LinearChannel;
    channel->setName("position");
    channel->setTargetName("test_target");
    channel->setSampler(sampler);

    animation->addChannel(channel);
    animManager->registerAnimation(animation);

    // 保存为临时OSGB文件
    std::string inputPath = (tempDir / "animated.osgb").string();
    osgDB::writeNodeFile(*root, inputPath);

    // 创建转换器
    Osgb2B3dm converter;
    std::string outputPath = (tempDir / "animated.b3dm").string();

    // 执行转换
    bool result = converter.convert(inputPath, outputPath);

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(outputPath));
    EXPECT_GT(std::filesystem::file_size(outputPath), 0);
}

// 测试多动画转换
TEST_F(AnimationTest, MultipleAnimations) {
    // 创建带有多个动画的场景
    osg::ref_ptr<osg::Group> root = new osg::Group;
    osg::ref_ptr<osgAnimation::BasicAnimationManager> animManager = new osgAnimation::BasicAnimationManager;
    root->setUpdateCallback(animManager);

    // 创建两个几何体
    for (int i = 0; i < 2; ++i) {
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
        
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

        // 为每个几何体创建动画
        osg::ref_ptr<osgAnimation::Animation> animation = new osgAnimation::Animation;
        animation->setName("animation_" + std::to_string(i));
        animation->setPlayMode(osgAnimation::Animation::LOOP);

        osg::ref_ptr<osgAnimation::Vec3LinearSampler> sampler = new osgAnimation::Vec3LinearSampler;
        osg::ref_ptr<osgAnimation::Vec3KeyframeContainer> keyframes = new osgAnimation::Vec3KeyframeContainer;
        keyframes->push_back(osgAnimation::Vec3Keyframe(0.0, osg::Vec3(0, 0, 0)));
        keyframes->push_back(osgAnimation::Vec3Keyframe(1.0, osg::Vec3(i+1, i+1, i+1)));
        sampler->setKeyframeContainer(keyframes);

        osg::ref_ptr<osgAnimation::Vec3LinearChannel> channel = new osgAnimation::Vec3LinearChannel;
        channel->setName("position");
        channel->setTargetName("target_" + std::to_string(i));
        channel->setSampler(sampler);

        animation->addChannel(channel);
        animManager->registerAnimation(animation);
    }

    // 保存为临时OSGB文件
    std::string inputPath = (tempDir / "multiple_animated.osgb").string();
    osgDB::writeNodeFile(*root, inputPath);

    // 创建转换器
    Osgb2B3dm converter;
    std::string outputPath = (tempDir / "multiple_animated.b3dm").string();

    // 执行转换
    bool result = converter.convert(inputPath, outputPath);

    // 验证结果
    EXPECT_TRUE(result);
    EXPECT_TRUE(std::filesystem::exists(outputPath));
    EXPECT_GT(std::filesystem::file_size(outputPath), 0);
} 