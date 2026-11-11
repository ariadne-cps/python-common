cmake_minimum_required(VERSION 3.15)

project(AriadnePythonCommon VERSION 3.0 LANGUAGES CXX)

add_library(pyariadne-python-common INTERFACE)
target_include_directories(pyariadne-python-common INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/ariadne-python-common>
)
