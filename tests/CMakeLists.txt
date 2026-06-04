include(GoogleTest)

mark_as_advanced(BUILD_GMOCK BUILD_GTEST gtest_hide_internal_symbols)

add_executable(unit_tests
  UniqueHandleTests.cpp
  ErrorTests.cpp
  BufferTests.cpp
)

target_link_libraries(unit_tests gmock_main gtest glrlib)
gtest_discover_tests(unit_tests DISCOVERY_MODE PRE_TEST)
