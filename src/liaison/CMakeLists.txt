
protobuf_include_dirs(${GOBY_INCLUDE_DIRS})
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR} config.proto)

add_library(liaison_plugins_drc_tc SHARED drc_tc_gui.cpp ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(liaison_plugins_drc_tc ${PROTOBUF_LIBRARIES} ${GOBY_LIBRARIES} wt)