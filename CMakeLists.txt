cmake_minimum_required(VERSION 3.23)

set(OPT_STD "cxx_std_23")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER cmake)

project(toy_lang CXX)

add_subdirectory(toy_lib)
add_subdirectory(toy)
target_link_libraries(toy toy_lib)