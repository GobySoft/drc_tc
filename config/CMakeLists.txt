configure_file(drc_goby_liaison.pb.cfg.in ${CMAKE_CURRENT_BINARY_DIR}/drc_goby_liaison.pb.cfg @ONLY)

configure_file(drc_tc.pb.cfg.in ${CMAKE_CURRENT_BINARY_DIR}/drc_tc.pb.cfg @ONLY)


configure_file(drc_tc.conf.in ${CMAKE_CURRENT_BINARY_DIR}/drc_tc.conf @ONLY)
configure_file(drc_tc.service.in ${CMAKE_CURRENT_BINARY_DIR}/drc_tc.service @ONLY)
configure_file(drc_tc_link2.conf.in ${CMAKE_CURRENT_BINARY_DIR}/drc_tc_link2.conf @ONLY)

configure_file(drc_tc_gui.conf.in ${CMAKE_CURRENT_BINARY_DIR}/drc_tc_gui.conf @ONLY)
configure_file(drc_tc_gui.service.in ${CMAKE_CURRENT_BINARY_DIR}/drc_tc_gui.service @ONLY)


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/drc_goby_liaison.pb.cfg ${CMAKE_CURRENT_BINARY_DIR}/drc_tc.pb.cfg blackout-file-noop.csv blackout-file-sample.csv DESTINATION /etc/drc_tc)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/drc_tc.conf ${CMAKE_CURRENT_BINARY_DIR}/drc_tc_link2.conf ${CMAKE_CURRENT_BINARY_DIR}/drc_tc_gui.conf DESTINATION /etc/init)
