add_executable(
  compiler
  Lexer.cpp
  Semantics.cpp
  Parser.cpp
  Ast.cpp
  ../../vm/src/vm.cpp
  Generator.cpp
  Main.cpp
)

target_compile_definitions(
  compiler
  PRIVATE
  DEBUG_VM=0
)