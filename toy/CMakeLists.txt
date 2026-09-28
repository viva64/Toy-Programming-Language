cmake_minimum_required(VERSION 3.23)

project(derp CXX)

add_executable(toy pch.h src.cpp input.txt)

target_include_directories(toy PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(toy PRIVATE ${OPT_STD})

set_target_properties(toy PROPERTIES
                      VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:toy>")

target_precompile_headers(toy PRIVATE pch.h)

source_group(src FILES src.cpp)