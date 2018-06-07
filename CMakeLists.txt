cmake_minimum_required(VERSION 2.6.0 FATAL_ERROR)
project(drc_tc)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")

find_package(GOBY REQUIRED)
get_target_property(GOBY_INCLUDE_DIR goby_middleware INTERFACE_INCLUDE_DIRECTORIES)
include_directories(${GOBY_INCLUDE_DIR})

# protocol buffers - http://code.google.com/apis/protocolbuffers/
find_package(ProtobufLocal REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

add_subdirectory(config)
add_subdirectory(src)
