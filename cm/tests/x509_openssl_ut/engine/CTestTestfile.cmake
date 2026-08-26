# CMake generated Testfile for 
# Source directory: /session/w683b/tests/x509_openssl_ut/engine
# Build directory: /session/w683b/cm/tests/x509_openssl_ut/engine
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(x509_openssl_ut_engine "/session/w683b/cm/tests/x509_openssl_ut/engine/x509_openssl_ut_engine_exe")
set_tests_properties(x509_openssl_ut_engine PROPERTIES  _BACKTRACE_TRIPLES "/session/w683b/configs/azure_c_shared_utilityFunctions.cmake;466;add_test;/session/w683b/configs/azure_c_shared_utilityFunctions.cmake;545;c_linux_unittests_add_exe;/session/w683b/tests/x509_openssl_ut/engine/CMakeLists.txt;28;build_c_test_artifacts;/session/w683b/tests/x509_openssl_ut/engine/CMakeLists.txt;0;")
add_test(x509_openssl_ut_engine_valgrind "valgrind" "--gen-suppressions=all" "--num-callers=100" "--error-exitcode=1" "--leak-check=full" "--track-origins=yes" "/session/w683b/cm/tests/x509_openssl_ut/engine/x509_openssl_ut_engine_exe")
set_tests_properties(x509_openssl_ut_engine_valgrind PROPERTIES  TIMEOUT "3000" _BACKTRACE_TRIPLES "/session/w683b/configs/azure_c_shared_utilityFunctions.cmake;473;add_test;/session/w683b/configs/azure_c_shared_utilityFunctions.cmake;545;c_linux_unittests_add_exe;/session/w683b/tests/x509_openssl_ut/engine/CMakeLists.txt;28;build_c_test_artifacts;/session/w683b/tests/x509_openssl_ut/engine/CMakeLists.txt;0;")
add_test(x509_openssl_ut_engine_helgrind "valgrind" "--tool=helgrind" "--gen-suppressions=all" "--num-callers=100" "--error-exitcode=1" "/session/w683b/cm/tests/x509_openssl_ut/engine/x509_openssl_ut_engine_exe")
set_tests_properties(x509_openssl_ut_engine_helgrind PROPERTIES  TIMEOUT "3000" _BACKTRACE_TRIPLES "/session/w683b/configs/azure_c_shared_utilityFunctions.cmake;474;add_test;/session/w683b/configs/azure_c_shared_utilityFunctions.cmake;545;c_linux_unittests_add_exe;/session/w683b/tests/x509_openssl_ut/engine/CMakeLists.txt;28;build_c_test_artifacts;/session/w683b/tests/x509_openssl_ut/engine/CMakeLists.txt;0;")
add_test(x509_openssl_ut_engine_drd "valgrind" "--tool=drd" "--gen-suppressions=all" "--num-callers=100" "--error-exitcode=1" "/session/w683b/cm/tests/x509_openssl_ut/engine/x509_openssl_ut_engine_exe")
set_tests_properties(x509_openssl_ut_engine_drd PROPERTIES  TIMEOUT "3000" _BACKTRACE_TRIPLES "/session/w683b/configs/azure_c_shared_utilityFunctions.cmake;475;add_test;/session/w683b/configs/azure_c_shared_utilityFunctions.cmake;545;c_linux_unittests_add_exe;/session/w683b/tests/x509_openssl_ut/engine/CMakeLists.txt;28;build_c_test_artifacts;/session/w683b/tests/x509_openssl_ut/engine/CMakeLists.txt;0;")
