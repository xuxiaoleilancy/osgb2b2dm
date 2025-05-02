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

# 添加库目标
add_library(osgb2b3dm
    src/osgb2b3dm.cpp
)

# 包含目录
target_include_directories(osgb2b3dm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(osgb2b3dm PUBLIC
    ${OPENSCENEGRAPH_LIBRARIES}
    nlohmann_json::nlohmann_json
    z
)

# 添加编译选项
target_compile_definitions(osgb2b3dm PUBLIC
    OSG_ANIMATION_SUPPORT=1
)

# 添加可执行文件
add_executable(osgb2b3dm_cli
    src/main.cpp
)

# 链接主程序
target_link_libraries(osgb2b3dm_cli PRIVATE
    osgb2b3dm
)

# 添加示例
add_subdirectory(examples)