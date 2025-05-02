cmake_minimum_required(VERSION 3.10)
project(osgb2b3dm)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(OpenSceneGraph REQUIRED COMPONENTS
    osg
    osgDB
    osgAnimation
    osgUtil
)
find_package(nlohmann_json REQUIRED)

# 添加主库
add_library(osgb2b3dm STATIC
    src/osgb2b3dm.cpp
)

# 添加命令行工具
add_executable(osgb2b3dm_cli
    src/main.cpp
)

# 添加示例
add_subdirectory(examples)

# 添加测试
enable_testing()
find_package(GTest REQUIRED)

# 添加测试可执行文件
add_executable(test_osgb2b3dm
    tests/test_osgb2b3dm.cpp
    tests/test_animation.cpp
    tests/test_instancing.cpp
)

# 设置包含目录
target_include_directories(osgb2b3dm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
    ${nlohmann_json_INCLUDE_DIRS}
)

target_include_directories(osgb2b3dm_cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(test_osgb2b3dm PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GTEST_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(osgb2b3dm
    ${OPENSCENEGRAPH_LIBRARIES}
    nlohmann_json::nlohmann_json
)

target_link_libraries(osgb2b3dm_cli
    osgb2b3dm
    ${OPENSCENEGRAPH_LIBRARIES}
)

target_link_libraries(test_osgb2b3dm
    osgb2b3dm
    ${OPENSCENEGRAPH_LIBRARIES}
    GTest::GTest
    GTest::Main
)

# 添加测试
add_test(NAME test_osgb2b3dm COMMAND test_osgb2b3dm)

# 设置编译定义
target_compile_definitions(osgb2b3dm PUBLIC
    OSG_ANIMATION_SUPPORT=1
)