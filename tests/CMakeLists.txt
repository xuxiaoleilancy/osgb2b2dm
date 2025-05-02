# 查找 GTest
find_package(GTest REQUIRED)

# 添加测试可执行文件
add_executable(test_osgb2b3dm
    basic_conversion_test.cpp
    animation_test.cpp
    instancing_test.cpp
)

# 链接库
target_link_libraries(test_osgb2b3dm
    osgb2b3dm_lib
    ${OPENSCENEGRAPH_LIBRARIES}
    GTest::GTest
    GTest::Main
)

# 添加测试
add_test(NAME test_osgb2b3dm COMMAND test_osgb2b3dm) 