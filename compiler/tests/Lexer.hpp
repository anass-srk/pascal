#include <gtest/gtest.h>
#include "../src/Lexer.hpp"

using namespace pascal_compiler;

TEST(LexerTest, Keywords)
{

  Lexer lexer("program begin end if then else while do");

  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::PROGRAM_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::BEGIN_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::END_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::IF_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::THEN_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::ELSE_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::WHILE_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::DO_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::EOF_TOKEN);
}

TEST(LexerTest, Identifiers)
{

  Lexer lexer("myVar _test a a1   ab\n\t a2c ad3 _a1 _ab _a2c _ad3 Do");

  auto id_test = [&lexer](const char* id)
  {
    EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::ID_TOKEN);
    EXPECT_EQ(lexer.getToken().m_id, id);
  };

  id_test("myVar");
  id_test("_test");
  id_test("a");
  id_test("a1");
  id_test("ab");
  id_test("a2c");
  id_test("ad3");
  id_test("_a1");
  id_test("_ab");
  id_test("_a2c");
  id_test("_ad3");

  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::DO_TOKEN);
}

TEST(LexerTest, Numbers)
{
  // 1 13
  //   .9 .74
  //   E
  //   -
  //   33 6

  Lexer lexer(R"(1 1E-33 1E6 1e33 1e+6 1.9 1.9E-33 1.9E6 1.9e33 1.9e+6 
  1.74 1.74E-33 1.74E6 1.74e33 1.74e+6 13.9 13.9E-33 13.9E6 13.9e33 13.9e+6 
  13.74 13.74E-33 13.74E6 13.74e33 13.74e+6 13 13E-33 13E6 13e33  13e+6)");

  auto int_test = [&lexer](long long i)
  {
    EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::NUM_INT_TOKEN);
    EXPECT_EQ(lexer.getToken().m_ival, i);
  };

  auto real_test = [&lexer](double d)
  {
    EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::NUM_REAL_TOKEN);
    EXPECT_DOUBLE_EQ(lexer.getToken().m_dval, d);
  };

  int_test(1);
  real_test(1E-33);
  real_test(1E6);
  real_test(1e33);
  real_test(1e+6);
  real_test(1.9);
  real_test(1.9E-33);
  real_test(1.9E6);
  real_test(1.9e33);
  real_test(1.9e+6);
  real_test(1.74);
  real_test(1.74E-33);
  real_test(1.74E6);
  real_test(1.74e33);
  real_test(1.74e+6);
  real_test(13.9);
  real_test(13.9E-33);
  real_test(13.9E6);
  real_test(13.9e33);
  real_test(13.9e+6);
  real_test(13.74);
  real_test(13.74E-33);
  real_test(13.74E6);
  real_test(13.74e33);
  real_test(13.74e+6);
  int_test(13);
  real_test(13E-33);
  real_test(13E6);
  real_test(13e33);
  real_test(13e+6);

}

TEST(LexerTest, StringsAndChars)
{

  Lexer lexer(" \"\" \"hello\" \"world\t\" 'a' '\\n' ");

  auto str_test = [&lexer](const char* str)
  {
    EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::STRING_LITERAL_TOKEN);
    EXPECT_EQ(lexer.getToken().to_string_literal(), str);
  };

  std::string s;
  auto char_test = [&lexer, &s](char c)
  {
    EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::CHAR_LITERAL_TOKEN);
    s = lexer.getToken().to_string_literal();
    EXPECT_EQ(s.length(), 1);
    EXPECT_EQ(s[0], c);
  };

  str_test("");
  str_test("hello");
  str_test("world\t");
  char_test('a');
  char_test('\n');
}

TEST(LexerTest, Operators)
{

  Lexer lexer(":= + - * / = <> < <= > >=");

  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::ASSIGN_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::PLUS_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::MINUS_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::STAR_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::SLASH_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::EQ_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::NEQ_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::LT_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::LE_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::GT_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::GE_TOKEN);
}

TEST(LexerTest, Comments)
{

  Lexer lexer("program { this is a comment } test");

  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::PROGRAM_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::ID_TOKEN);
  EXPECT_EQ(lexer.getToken().m_id, "test");
}

TEST(LexerTest, ComplexExpression)
{

  Lexer lexer("x := (a + b) * c - 10.5;");

  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::ID_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::ASSIGN_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::LP_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::ID_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::PLUS_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::ID_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::RP_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::STAR_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::ID_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::MINUS_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::NUM_REAL_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::SEMI_TOKEN);
}

TEST(LexerTest, SimpleProgram)
{

  Lexer lexer(
    "program Test;\n"
    "var x: integer;\n"
    "begin\n"
    "  x := 42;\n"
    "end."
  );

  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::PROGRAM_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::ID_TOKEN);
  EXPECT_EQ(lexer.getToken().m_id, "Test");
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::SEMI_TOKEN);

  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::VAR_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::ID_TOKEN);
  EXPECT_EQ(lexer.getToken().m_id, "x");
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::COLON_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::ID_TOKEN);
  EXPECT_EQ(lexer.getToken().m_id, "integer");
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::SEMI_TOKEN);

  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::BEGIN_TOKEN);

  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::ID_TOKEN);
  EXPECT_EQ(lexer.getToken().m_id, "x");
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::ASSIGN_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::NUM_INT_TOKEN);
  EXPECT_EQ(lexer.getToken().m_ival, 42);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::SEMI_TOKEN);

  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::END_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::DOT_TOKEN);
  EXPECT_EQ(lexer.next_sym().m_type, TOKEN_TYPE::EOF_TOKEN);

}