cmake_minimum_required(VERSION 3.10)
project(osgb2b3dm)

# 设置 CMake 策略
cmake_policy(SET CMP0079 NEW)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(OpenSceneGraph REQUIRED COMPONENTS 
    osg 
    osgDB 
    osgAnimation 
    osgUtil 
    OpenThreads
)
find_package(nlohmann_json REQUIRED)
find_package(GTest REQUIRED)
find_package(ZLIB REQUIRED)

# 添加包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
)

# 添加编译定义
add_definitions(-DOSG_ANIMATION_SUPPORT=1)

# 创建库
add_library(osgb2b3dm_lib
    src/osgb2b3dm.cpp
    src/osgb2b3dm_error.cpp
)

# 链接库
target_link_libraries(osgb2b3dm_lib
    ${OPENSCENEGRAPH_LIBRARIES}
    nlohmann_json::nlohmann_json
    ZLIB::ZLIB
)

# 添加命令行工具
add_executable(osgb2b3dm_cli
    src/main.cpp
)

target_link_libraries(osgb2b3dm_cli
    osgb2b3dm_lib
    ${OPENSCENEGRAPH_LIBRARIES}
)

# 添加测试和示例
enable_testing()
add_subdirectory(tests)
add_subdirectory(examples)