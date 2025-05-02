#include <gtest/gtest.h>
#include "osgb2b3dm.h"
#include "osgb2b3dm_error.h"

using namespace osgb2b3dm;

class InstancingTest : public ::testing::Test {
protected:
    void SetUp() override {
        converter = std::make_unique<Osgb2B3dm>();
    }

    std::unique_ptr<Osgb2B3dm> converter;
};

TEST_F(InstancingTest, InvalidInstanceData) {
    // 设置一个包含无效实例数据的测试场景
    InstanceGroup group;
    group.name = "test_group";
    Instance instance;
    instance.name = "test_instance";
    instance.meshIndex = -1;  // 设置无效的网格索引
    group.instances.push_back(instance);
    converter->addInstanceGroup(group);
    
    EXPECT_FALSE(converter->processScene());
    EXPECT_EQ(converter->lastError(), make_error_code(ErrorCode::INSTANCE_DATA_INVALID));
}

TEST_F(InstancingTest, MissingInstanceData) {
    // 设置一个缺少必要实例数据的测试场景
    InstanceGroup group;
    group.name = "test_group";
    // 不添加任何实例，使其缺少必要数据
    converter->addInstanceGroup(group);
    
    EXPECT_FALSE(converter->processScene());
    EXPECT_EQ(converter->lastError(), make_error_code(ErrorCode::INSTANCE_PROCESSING_ERROR));
}

// ... existing code ... 