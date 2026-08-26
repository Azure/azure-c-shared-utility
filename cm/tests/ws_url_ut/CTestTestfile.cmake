# CMake generated Testfile for 
# Source directory: /session/w683b/tests/ws_url_ut
# Build directory: /session/w683b/cm/tests/ws_url_ut
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(ws_url_ut "/session/w683b/cm/tests/ws_url_ut/ws_url_ut_exe")
set_tests_properties(ws_url_ut PROPERTIES  _BACKTRACE_TRIPLES "/session/w683b/configs/azure_c_shared_utilityFunctions.cmake;466;add_test;/session/w683b/configs/azure_c_shared_utilityFunctions.cmake;545;c_linux_unittests_add_exe;/session/w683b/tests/ws_url_ut/CMakeLists.txt;18;build_c_test_artifacts;/session/w683b/tests/ws_url_ut/CMakeLists.txt;0;")
add_test(ws_url_ut_valgrind "valgrind" "--gen-suppressions=all" "--num-callers=100" "--error-exitcode=1" "--leak-check=full" "--track-origins=yes" "/session/w683b/cm/tests/ws_url_ut/ws_url_ut_exe")
set_tests_properties(ws_url_ut_valgrind PROPERTIES  TIMEOUT "3000" _BACKTRACE_TRIPLES "/session/w683b/configs/azure_c_shared_utilityFunctions.cmake;473;add_test;/session/w683b/configs/azure_c_shared_utilityFunctions.cmake;545;c_linux_unittests_add_exe;/session/w683b/tests/ws_url_ut/CMakeLists.txt;18;build_c_test_artifacts;/session/w683b/tests/ws_url_ut/CMakeLists.txt;0;")
add_test(ws_url_ut_helgrind "valgrind" "--tool=helgrind" "--gen-suppressions=all" "--num-callers=100" "--error-exitcode=1" "/session/w683b/cm/tests/ws_url_ut/ws_url_ut_exe")
set_tests_properties(ws_url_ut_helgrind PROPERTIES  TIMEOUT "3000" _BACKTRACE_TRIPLES "/session/w683b/configs/azure_c_shared_utilityFunctions.cmake;474;add_test;/session/w683b/configs/azure_c_shared_utilityFunctions.cmake;545;c_linux_unittests_add_exe;/session/w683b/tests/ws_url_ut/CMakeLists.txt;18;build_c_test_artifacts;/session/w683b/tests/ws_url_ut/CMakeLists.txt;0;")
add_test(ws_url_ut_drd "valgrind" "--tool=drd" "--gen-suppressions=all" "--num-callers=100" "--error-exitcode=1" "/session/w683b/cm/tests/ws_url_ut/ws_url_ut_exe")
set_tests_properties(ws_url_ut_drd PROPERTIES  TIMEOUT "3000" _BACKTRACE_TRIPLES "/session/w683b/configs/azure_c_shared_utilityFunctions.cmake;475;add_test;/session/w683b/configs/azure_c_shared_utilityFunctions.cmake;545;c_linux_unittests_add_exe;/session/w683b/tests/ws_url_ut/CMakeLists.txt;18;build_c_test_artifacts;/session/w683b/tests/ws_url_ut/CMakeLists.txt;0;")
