add_executable(
  unit_tests 
  Lexer.cpp
)

target_link_libraries(
  unit_tests 
  GTest::gtest_main
)

#Allows using Google test helper commands
include(GoogleTest)
gtest_discover_tests(unit_tests)