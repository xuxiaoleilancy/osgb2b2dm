#include <gtest/gtest.h>
#include "osgb2b3dm.h"
#include "osgb2b3dm_error.h"

using namespace osgb2b3dm;

class BasicConversionTest : public ::testing::Test {
protected:
    void SetUp() override {
        converter = std::make_unique<Osgb2B3dm>();
    }

    std::unique_ptr<Osgb2B3dm> converter;
};

TEST_F(BasicConversionTest, EmptyInputPath) {
    EXPECT_FALSE(converter->convert("", "output.b3dm"));
    EXPECT_EQ(converter->lastError(), make_error_code(ErrorCode::INVALID_INPUT));
}

TEST_F(BasicConversionTest, EmptyOutputPath) {
    EXPECT_FALSE(converter->convert("input.osgb", ""));
    EXPECT_EQ(converter->lastError(), make_error_code(ErrorCode::INVALID_OUTPUT));
}

TEST_F(BasicConversionTest, NonExistentInputFile) {
    EXPECT_FALSE(converter->convert("nonexistent.osgb", "output.b3dm"));
    EXPECT_EQ(converter->lastError(), make_error_code(ErrorCode::FILE_READ_ERROR));
}

// ... existing code ... 