add_executable(
  unit_tests_vm
  ../src/vm.cpp
  Main.cpp
)

target_link_libraries(
  unit_tests_vm
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests_vm)