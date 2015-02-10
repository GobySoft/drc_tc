add_executable(drc_blackout drc_blackout.cpp)
target_link_libraries(drc_blackout boost_system pthread)
install(TARGETS drc_blackout RUNTIME DESTINATION bin)
