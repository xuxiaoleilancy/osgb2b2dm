cmake_minimum_required(VERSION 3.10)
project(osgb2b3dm)

# 设置 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖包
find_package(OpenSceneGraph REQUIRED COMPONENTS osg osgDB osgUtil osgAnimation)
find_package(ZLIB REQUIRED)

# 添加库目标
add_library(osgb2b3dm_lib
    src/osgb2b3dm.cpp
    src/osgb2b3dm_error.cpp
)

# 设置库的包含目录
target_include_directories(osgb2b3dm_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
)

# 链接依赖库
target_link_libraries(osgb2b3dm_lib PUBLIC
    ${OPENSCENEGRAPH_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

# 添加可执行文件
add_executable(osgb2b3dm_cli src/main.cpp)
target_link_libraries(osgb2b3dm_cli PRIVATE osgb2b3dm_lib)

# 添加子目录
add_subdirectory(examples)
add_subdirectory(tests)

# 启用测试
enable_testing()