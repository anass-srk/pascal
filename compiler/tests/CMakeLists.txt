add_executable(
  unit_tests_compiler
  ../src/Lexer.cpp
  Lexer.hpp
  ../src/Semantics.cpp
  ../src/Parser.cpp
  ../src/Ast.cpp
  Parser.hpp
  Helper.hpp
  Expressions.hpp
  Main.cpp
)

target_link_libraries(
  unit_tests_compiler 
  GTest::gtest_main
)

#Allows using Google test helper commands
include(GoogleTest)
gtest_discover_tests(unit_tests_compiler)