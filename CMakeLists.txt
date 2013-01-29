cmake_minimum_required(VERSION 2.6.0 FATAL_ERROR)
project(drc_tc)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

find_package(Goby REQUIRED)

add_subdirectory(src)
