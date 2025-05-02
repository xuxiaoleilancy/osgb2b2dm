#include <gtest/gtest.h>
#include "osgb2b3dm.h"
#include "osgb2b3dm_error.h"

using namespace osgb2b3dm;

class AnimationTest : public ::testing::Test {
protected:
    void SetUp() override {
        converter = std::make_unique<Osgb2B3dm>();
    }

    std::unique_ptr<Osgb2B3dm> converter;
};

TEST_F(AnimationTest, InvalidAnimationData) {
    // 设置一个包含无效动画数据的测试场景
    Animation anim;
    anim.name = "test_animation";
    anim.duration = 1.0;
    // 不设置 channels 和 samplers，使其无效
    converter->addAnimation(anim);
    
    EXPECT_FALSE(converter->processScene());
    EXPECT_EQ(converter->lastError(), make_error_code(ErrorCode::ANIMATION_DATA_INVALID));
}

TEST_F(AnimationTest, MissingAnimationData) {
    // 设置一个缺少必要动画数据的测试场景
    Animation anim;
    anim.name = "test_animation";
    anim.duration = 1.0;
    anim.channels.push_back("channel1");
    // 不设置 samplers，使其缺少必要数据
    converter->addAnimation(anim);
    
    EXPECT_FALSE(converter->processScene());
    EXPECT_EQ(converter->lastError(), make_error_code(ErrorCode::ANIMATION_DATA_INVALID));
}

// ... existing code ... 