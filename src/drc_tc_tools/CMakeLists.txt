include_directories(${CMAKE_BINARY_DIR}/src/liaison)
add_executable(drc_tc_apply drc_tc_apply.cpp)
target_link_libraries(drc_tc_apply liaison_plugins_drc_tc)

add_executable(drc_tc_parse drc_tc_parse.cpp)
target_link_libraries(drc_tc_parse liaison_plugins_drc_tc)

install(TARGETS drc_tc_apply drc_tc_parse RUNTIME DESTINATION bin)
