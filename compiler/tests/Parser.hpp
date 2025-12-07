#include <gtest/gtest.h>
#include "../src/Parser.hpp"

using namespace pascal_compiler;

using ConstType = decltype(Const::m_val);

TEST(ParserTest, Labels){

  Parser parser("program LabelTest; label ; .");
  EXPECT_THROW(parser.parse(), SyntaxException);

  auto test_label = [](const char* code, std::initializer_list<std::string_view> ids){
    
    Parser p(code);
    p.parse();

    EXPECT_EQ(p.m_current_block->m_labels.size(), ids.size());

    for(auto id : ids){
      const Label* label = Block::get(id, p.m_current_block->m_labels); 
      EXPECT_NE(label, nullptr);
      EXPECT_EQ(label->m_id, id);
    }

  };

  test_label(
    "program LabelTest; label 1; .",
    {"1"}
  );

  test_label(
    "program LabelTest; label exit; .",
    {"exit"}
  );

  test_label(
    "program LabelTest; label 1, 2, 3, 100, 999; .",
    {"1", "2", "3", "100", "999"}
  );

  test_label(
    "program LabelTest; label start, middle, exit, error; .",
    {"start", "middle", "exit", "error"}
  );

  test_label(
    "program LabelTest; label 1, exit, 100, retry, 999; .",
    {"1", "exit", "100", "retry", "999"}
  );

}

TEST(ParserTest, Enums){

  
  Parser p("program EnumTest; type EmptyEnum = (); .");
  EXPECT_THROW(p.parse(), SyntaxException);

  auto test_enums = [](const Parser& p, std::string_view name ,std::initializer_list<std::string_view> enum_vals){

    const Type* type = Block::get(name, p.m_current_block->m_types);
    EXPECT_NE(type, nullptr);
    EXPECT_EQ(type->m_type, TYPE_CAT::TC_ENUM);

    const Enum* e = static_cast<const Enum*>(type);
    EXPECT_EQ(e->m_values.size(), enum_vals.size());

    for(auto enum_val : enum_vals){
      EXPECT_TRUE(e->m_values.contains(enum_val));
      EXPECT_TRUE(p.m_current_block->m_enums_vals.contains(enum_val));
    }

  };

  p = Parser("program EnumTest; type SingleEnum = (only); .");
  p.parse();

  test_enums(
    p, "SingleEnum",
    {"only"}
  );

  
  p = Parser("program EnumTest; type Colors = (red, green, blue); Days = (monday, tuesday, wednesday, thursday, friday, saturday, sunday); .");
  p.parse();

  test_enums(
    p, "Colors",
    {"red", "green", "blue"}
  );

  test_enums(
    p, "Days",
    {"monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "sunday"}
  );
}

TEST(ParserTest, Consts){
  // \t + -
  // CONST_NAME INT REAL Enum Char Bool String

  auto test = [](const char* code, std::string_view name, auto value){
    Parser p(std::string("program TestConst; ") + code + '.');
    p.parse();

    EXPECT_EQ(p.m_current_block->m_consts.size(), 1);

    const Const* v = Block::get(name, p.m_current_block->m_consts);
    EXPECT_NE(v, nullptr);
    EXPECT_EQ(std::get<decltype(value)>(v->m_val), value);
  };

  auto test_complex = []<typename T>(const char *code, std::initializer_list<std::pair<std::string_view, T>> l)
  {
    Parser p(std::string("program TestConst; ") + code + '.');
    p.parse();

    EXPECT_EQ(p.m_current_block->m_consts.size(), l.size());

    for (const auto &[name, value] : l)
    {
      const Const *v = Block::get(name, p.m_current_block->m_consts);
      EXPECT_NE(v, nullptr);
      if constexpr(std::is_same_v<T, ConstType>){
        EXPECT_EQ(v->m_val, v->m_val);
      }else {
        EXPECT_EQ(std::get<T>(v->m_val), value);
      }
    }
  };

  auto test_throws = []<class T>(const char* code){
    Parser p(std::string("program TestConst; ") + code + '.');
    EXPECT_THROW(p.parse(), T);
  };

  test("const a = 2;", "a", Int(2));
  test_complex.template operator()<Int>(
    "const a = -2; b = a;c = 2; d = -a; e = +a;", 
    {{"b", Int(-2)}, {"a", Int(-2)}, {"c", Int(2)}, {"d", Int(2)}, {"e", Int(-2)}}
  );

  test("const _a = 4.5;", "_a", Real(4.5));
  test_complex.template operator()<Real>(
    "const _a = -4.5; b = _a; c = 4.5; d = -_a; e = +_a;",
    {{"b", Real(-4.5)}, {"_a", Real(-4.5)}, {"c", Real(4.5)}, {"d", Real(4.5)}, {"e", Real(-4.5)}}
  );

  test_complex.template operator()<Int>(
    "type C = (A, B); const b2 = A; b3 = B;", 
    {{"b2", Int(0)}, {"b3", Int(1)}}
  );

  test_throws.template operator()<SemanticException>("type C = (A, B); const b2 = -A;");

  test("const b = '?';", "b", '?');
  test_throws.template operator()<SyntaxException>("const b = +'?';");
  test_complex.template operator()<char>(
    "const b = '?'; c = b;",
    {{"c",char('?')}, {"b",char('?')}}
  );

  test("const exists = false;", "exists", false);
  test_complex.template operator()<bool> (
    "const exists = false; a1 = exists;",
    {{"exists", false}, {"a1", false}}
  );
  test_throws.template operator()<SyntaxException>("const exists = -false;");

  test("const exists = true;", "exists", true);
  test_throws.template operator()<SyntaxException>("const exists = +true;");

  test_complex.template operator()<std::string>(
    "const text = \"Hello !\\n\"; text2 = text;", 
    {{"text", std::string("Hello !\n")}, {"text2", std::string("Hello !\n")}}
  );
  test_throws.template operator()<SyntaxException>("const text = -\"Hello !\\n\";");

  test_complex.template operator()<ConstType>(
    R"( 
    type Color = (Red, Green, Blue);
    const
      a = 10;
      b = -20;
      c = 3.14;
      d = 'X';
      e = true;
      f = "test";
      g = Green;
      h = +100;
      i = -3.5e10;)",
    {
      {"a", Int(10)},
      {"b", Int(-20)},
      {"c", Real(3.14)},
      {"d", 'X'},
      {"e", true},
      {"f", std::string("test")},
      {"g", Int(1)},
      {"h", Int(100)},
      {"i", Real(-3.5e10)},
    }
  );
}

TEST(ParserTest, Aliases){

  auto test = [](const char* code, std::initializer_list<std::pair<std::string_view, std::string_view>> l){
    Parser p(std::string("program TestConst; ") + code + '.');
    p.parse();

    for(const auto& [id, ref] : l){
      const Type* type0 = Block::get(ref, p.m_current_block->m_types);
      EXPECT_NE(type0, nullptr);

      const Type *type1 = Block::get(id, p.m_current_block->m_types);
      EXPECT_NE(type1, nullptr);

      EXPECT_EQ(type0->m_type, type1->m_type);
      EXPECT_EQ(type0, type1);
    }
  };

  test("type a = Int;", {{"a", "Int"}});

  test(
    R"(TYPE
      Col = (Black, White);
      MyColor = Col;
      MyInt = Int;
      MyReal = Real;
      MyString = String;
      MyBool = Bool;
    )",
    {
      {"MyColor", "Col"},
      {"MyInt", "Int"},
      {"MyReal", "Real"},
      {"MyString", "String"},
      {"MyBool", "Bool"},
    }
  );
}

TEST(ParserTest, Subranges){

  auto test = [](const char* code, std::initializer_list<std::pair<std::string_view, std::pair<ConstType, ConstType>>> l){
    Parser p(std::string("program TestConst; ") + code + '.');
    p.parse();

    for(const auto& [id, vals] : l){
      const Type* _type = Block::get(id, p.m_current_block->m_types);
      EXPECT_NE(_type, nullptr);
      EXPECT_EQ(_type->m_type, TYPE_CAT::TC_SUBRANGE);

      const Subrange* sub = static_cast<const Subrange*>(_type);
      EXPECT_EQ(sub->m_beg.m_val, vals.first);
      EXPECT_EQ(sub->m_end.m_val, vals.second);
    }
  };

  auto test_throw = [](const char* code){
    Parser p(std::string("program TestConst; ") + code + '.');
    EXPECT_THROW(p.parse(), SemanticException);
  };

  test("type one = 1..10;", {{"one",{Int(1), Int(10)}}});

  test(
    R"(
    const min = -1; max = 5;
    TYPE
    Range1 = 1..100;
    Range2 = -10..10;
    Color = (January, February, March, April);
    Range3 = 0..max;
    Range4 = min..max;
    Letters = 'A'..'Z';
    Digits = '0'..'9';
    Month = January..March;
    )",
    {
      {"Range1",  {Int(1), Int(100)}},
      {"Range2",  {Int(-10), Int(10)}},
      {"Range3",  {Int(0), Int(5)}},
      {"Range4",  {Int(-1), Int(5)}},
      {"Letters", {'A', 'Z'}},
      {"Digits",  {'0', '9'}},
      {"Month",   {Int(0), Int(2)}}
    }
  );

  test_throw("type range = \"A\"..\"D\";");
  test_throw("type range = false..true;");
  test_throw("type range = 1.2 .. 3.5;");
  test_throw("type a = (A, B); range = A..5;");
  test_throw("type range = 5..1;");
}

TEST(ParserTest, Arrays){
  
}