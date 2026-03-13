#include "Lexer.hpp"
#include "Parser.hpp"
#include "Expressions.hpp"
#include "Statements.hpp"
#include "Visitor.hpp"

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}