# CMake generated Testfile for 
# Source directory: /home/srajimon/airspan/asn-codec
# Build directory: /home/srajimon/airspan/asn-codec/build-fuzz
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_rrc_codec "/home/srajimon/airspan/asn-codec/build-fuzz/test_codec")
set_tests_properties(test_rrc_codec PROPERTIES  _BACKTRACE_TRIPLES "/home/srajimon/airspan/asn-codec/CMakeLists.txt;151;add_test;/home/srajimon/airspan/asn-codec/CMakeLists.txt;0;")
add_test(test_x2ap_codec "/home/srajimon/airspan/asn-codec/build-fuzz/test_x2ap_codec")
set_tests_properties(test_x2ap_codec PROPERTIES  _BACKTRACE_TRIPLES "/home/srajimon/airspan/asn-codec/CMakeLists.txt;152;add_test;/home/srajimon/airspan/asn-codec/CMakeLists.txt;0;")
add_test(test_ngap_codec "/home/srajimon/airspan/asn-codec/build-fuzz/test_ngap_codec")
set_tests_properties(test_ngap_codec PROPERTIES  _BACKTRACE_TRIPLES "/home/srajimon/airspan/asn-codec/CMakeLists.txt;153;add_test;/home/srajimon/airspan/asn-codec/CMakeLists.txt;0;")
add_test(test_xnap_codec "/home/srajimon/airspan/asn-codec/build-fuzz/test_xnap_codec")
set_tests_properties(test_xnap_codec PROPERTIES  _BACKTRACE_TRIPLES "/home/srajimon/airspan/asn-codec/CMakeLists.txt;154;add_test;/home/srajimon/airspan/asn-codec/CMakeLists.txt;0;")
