include_directories(${CMAKE_BINARY_DIR}/src/liaison)
add_executable(drc_tc_apply drc_tc_apply.cpp)
target_link_libraries(drc_tc_apply liaison_plugins_drc_tc boost_system)

add_executable(drc_tc_parse drc_tc_parse.cpp)
target_link_libraries(drc_tc_parse liaison_plugins_drc_tc boost_system)

configure_file(drc_gen.sh.in ${CMAKE_CURRENT_BINARY_DIR}/drc_gen.sh @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/drc_gen.sh
  DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  PERMISSIONS WORLD_READ WORLD_EXECUTE)

install(TARGETS drc_tc_apply drc_tc_parse RUNTIME DESTINATION bin)

