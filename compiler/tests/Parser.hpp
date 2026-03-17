#pragma once
#include <gtest/gtest.h>
#include "../src/Parser.hpp"

using namespace pascal_compiler;

using ConstType = std::variant<Int, Real, std::string, char, bool>;

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
      EXPECT_EQ(label->id(), id);
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
    EXPECT_EQ(type->category(), TYPE_CAT::TC_ENUM);

    const Enum* e = static_cast<const Enum*>(type);
    EXPECT_EQ(e->values().size(), enum_vals.size());

    for(auto enum_val : enum_vals){
      EXPECT_TRUE(e->values().contains(enum_val));
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
    EXPECT_EQ(std::get<decltype(value)>(v->value()), value);
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
        EXPECT_EQ(v->value(), v->value());
      }else {
        EXPECT_EQ(std::get<T>(v->value()), value);
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

      EXPECT_EQ(type0->category(), type1->category());
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

  auto test = [](const char* code, std::initializer_list<std::pair<std::string_view, std::pair<Int, Int>>> l){
    Parser p(std::string("program TestConst; ") + code + '.');
    p.parse();

    for(const auto& [id, vals] : l){
      const Type* _type = Block::get(id, p.m_current_block->m_types);
      EXPECT_NE(_type, nullptr);
      EXPECT_EQ(_type->category(), TYPE_CAT::TC_SUBRANGE);

      const Subrange* sub = static_cast<const Subrange*>(_type);
      EXPECT_EQ(sub->start(), vals.first);
      EXPECT_EQ(sub->end(), vals.second);
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
  test_throw("type range = 1.2 .. 3.5;");
  test_throw("type a = (A, B); range = A..5;");
  test_throw("type range = 5..1;");
}

TEST(ParserTest, Arrays){
  Parser parser(R"(
    program Test;
    label a, b123; Const v = 2; v2 = -1;_1 = +v; h = 'h'; 
    hello = "Hello\n";PI=3.14;b = true; type t = Int; enum = (Blue, Red, Green);
    enum2 = (Monday); sub = 1..10; y = sub; ar1 = array [y] of Int; 
    ar2 = array [Blue..Green] of Char; ar_ = array [1..4, 1..4] of enum;
    ar3 = array [1..4, 1..4] of array [0..5] of Bool;
    ar4 = array [(small, medium, big), sub] of String;
    .)");
  parser.parse();

  auto check_subrange = []<typename T>(const Subrange* subrange, T a, T b) {
    EXPECT_EQ(subrange->start(), a);
    EXPECT_EQ(subrange->end(), b);
  };

  auto check_enum = [](const Enum* e, std::initializer_list<std::pair<std::string_view, Int>> enum_vals){

    EXPECT_EQ(e->values().size(), enum_vals.size());
    for(auto const [name, val] : enum_vals)
    {
      EXPECT_TRUE(e->values().contains(name));
      EXPECT_EQ(e->values().at(name), val);
    }
  };

  auto ar1 = static_cast<const Array *>(Block::get("ar1", parser.m_current_block->m_types));
  EXPECT_EQ(ar1->category(), TYPE_CAT::TC_ARRAY);
  EXPECT_EQ(ar1->index_types().size(), 1);
  EXPECT_EQ(ar1->index_types()[0]->id(), "sub");
  EXPECT_EQ(ar1->index_types()[0]->category(), TYPE_CAT::TC_SUBRANGE);
  EXPECT_EQ(ar1->element_type()->id(), "Int");
  EXPECT_EQ(ar1->element_type()->category(), TYPE_CAT::TC_BASIC);

  auto ar2 = static_cast<const Array *>(Block::get("ar2", parser.m_current_block->m_types));
  EXPECT_EQ(ar2->category(), TYPE_CAT::TC_ARRAY);
  EXPECT_EQ(ar2->index_types().size(), 1);
  check_subrange(static_cast<const Subrange*>(ar2->index_types()[0]), Int(0), Int(2));
  EXPECT_EQ(ar2->index_types()[0]->category(), TYPE_CAT::TC_SUBRANGE);
  EXPECT_EQ(ar2->element_type()->id(), "Char");
  EXPECT_EQ(ar2->element_type()->category(), TYPE_CAT::TC_BASIC);

  auto ar_ = static_cast<const Array *>(Block::get("ar_", parser.m_current_block->m_types));
  EXPECT_EQ(ar_->category(), TYPE_CAT::TC_ARRAY);
  EXPECT_EQ(ar_->index_types().size(), 2);
  check_subrange(static_cast<const Subrange *>(ar_->index_types()[0]), Int(1), Int(4));
  check_subrange(static_cast<const Subrange *>(ar_->index_types()[1]), Int(1), Int(4));
  EXPECT_EQ(ar_->index_types()[0]->category(), TYPE_CAT::TC_SUBRANGE);
  EXPECT_EQ(ar_->element_type()->id(), "enum");
  EXPECT_EQ(ar_->element_type()->category(), TYPE_CAT::TC_ENUM);

  auto ar4 = static_cast<const Array *>(Block::get("ar4", parser.m_current_block->m_types));
  EXPECT_EQ(ar4->category(), TYPE_CAT::TC_ARRAY);
  EXPECT_EQ(ar4->index_types().size(), 2);
  check_enum(
    static_cast<const Enum*>(ar4->index_types()[0]),
    {
      {"small", Int(0)},
      {"medium", Int(1)},
      {"big", Int(2)}
    }
  );
  EXPECT_EQ(ar4->index_types()[1]->id(), "sub");
  EXPECT_EQ(ar4->element_type()->id(), "String");
  EXPECT_EQ(ar4->element_type()->category(), TYPE_CAT::TC_BASIC);

  auto ar3 = static_cast<const Array *>(Block::get("ar3", parser.m_current_block->m_types));
  EXPECT_EQ(ar3->category(), TYPE_CAT::TC_ARRAY);
  EXPECT_EQ(ar3->index_types().size(), 2);
  check_subrange(static_cast<const Subrange *>(ar3->index_types()[0]), Int(1), Int(4));
  check_subrange(static_cast<const Subrange *>(ar3->index_types()[1]), Int(1), Int(4));
  auto _ar = static_cast<const Array *>(ar3->element_type());
  EXPECT_EQ(_ar->category(), TYPE_CAT::TC_ARRAY);
  EXPECT_EQ(_ar->index_types().size(), 1);
  check_subrange(static_cast<const Subrange *>(_ar->index_types()[0]), Int(0), Int(5));
  EXPECT_EQ(_ar->element_type()->id(), "Bool");
  EXPECT_EQ(_ar->element_type()->category(), TYPE_CAT::TC_BASIC);
}

TEST(ParserTest, Records){
  Parser parser(R"(
    program Test;
    label a, b123; Const v = 2; v2 = -1;_1 = +v; h = 'h'; 
    hello = "Hello\n";PI=3.14;b = true; type t = Int; enum = (Blue, Red, Green);
    enum2 = (Monday); sub = 1..10; y = sub; ar1 = array [y] of Int; 
    ar2 = array [Blue..Green] of Char; ar_ = array [1..4, 1..4] of enum;
    person = record 
      name: String;
      age, birthday: Int
    end ;
    Point = record
      x, y: Int;
    end;
    Address = record
      Street: String;
      City: String
    end;
    Person = record
      Name: String;
      Addr: Address
    end;
    ShapeKind = (Circle, Rectangle);
    Shape = record
      case Kind: ShapeKind of
        Circle: (Radius: Real);
        Rectangle: (Width, Height: Int)
    end;
    VariantType = 0..3;
    Variant = record
      name: String;
      kind: VariantType
      case VariantType of
        0: (val1: Int);
        1: (val2: Real);
        2, 3: (val3: array [-2..2] of Char)
    end;
    .)");
  parser.parse();

  auto check = [&](std::string_view id, std::initializer_list<std::pair<std::string_view, std::string_view>> l){
    const Record* rec = nullptr;
    if(auto it = parser.m_current_block->m_types.find(id); it != parser.m_current_block->m_types.end()){
      rec =  static_cast<const Record*>(it->second.get());
    }

    EXPECT_NE(rec, nullptr);

    for(const auto& [name, type_id] : l){
      auto it = rec->attributes().find(name);
      EXPECT_NE(it, rec->attributes().end());

      auto type = parser.m_current_block->m_types.find(type_id);
      EXPECT_NE(type, parser.m_current_block->m_types.end());

      EXPECT_EQ(it->second.type(), type->second.get());
    }

  };

  check("person",{
    {"name", "String"},
    {"age", "Int"},
    {"birthday", "Int"}
  });

  check("Point",{
    {"x", "Int"},
    {"y", "Int"}
  });

  check("Address",{
    {"Street", "String"},
    {"City", "String"}
  });

  check("Person",{
    {"Name", "String"},
    {"Addr", "Address"}
  });

  check("Shape",{
    {"Kind", "ShapeKind"},
    {"Radius", "Real"},
    {"Width", "Int"},
    {"Height", "Int"}
  });

  check("Variant",{
      {"name", "String"},
      {"kind", "VariantType"},
      {"val1", "Int"},
      {"val2", "Real"}
  });

  {
    const Record* rec = nullptr;
    if(auto it = parser.m_current_block->m_types.find("Variant"); it != parser.m_current_block->m_types.end()){
      rec = static_cast<const Record*>(it->second.get());
    }
    EXPECT_NE(rec, nullptr);

    const Array* arr = nullptr;
    if(auto it = rec->attributes().find("val3"); it != rec->attributes().end()){
      EXPECT_NE(it->second.type(), nullptr);
      EXPECT_EQ(it->second.type()->category(), TYPE_CAT::TC_ARRAY);
      arr = static_cast<const Array*>(it->second.type());
    }
    EXPECT_NE(arr, nullptr);

    EXPECT_EQ(arr->index_types().size(), 1);
    EXPECT_EQ(arr->index_types()[0]->category(), TYPE_CAT::TC_SUBRANGE);
    auto sub = static_cast<const Subrange*>(arr->index_types()[0]);
    EXPECT_EQ(sub->start(), -2);
    EXPECT_EQ(sub->end(), 2);
    
    EXPECT_EQ(arr->element_type()->id(), "Char");
  }

}

TEST(ParserTest, Variables)
{
  // single variable
  {
    std::string program = R"(
      program Test;
      var x : Int;
      .
    )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    auto &vars = parser.m_current_block->m_vars;
    ASSERT_EQ(vars.size(), 1);
    auto it = vars.find("x");
    ASSERT_NE(it, vars.end());
    const Var &var = it->second;
    EXPECT_EQ(var.id(), "x");
    EXPECT_EQ(var.type()->id(), "Int");
    EXPECT_EQ(var.type()->category(), TYPE_CAT::TC_BASIC);
  }

  // Multiple variables
  {
    std::string program = R"(
      program Test;
      var
        x, y : Int;
        a : Int;
        b : Real;
        c : Char;
        d : Bool;
      .
    )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    auto &vars = parser.m_current_block->m_vars;
    ASSERT_EQ(vars.size(), 6);

    auto test = [&](std::initializer_list<std::pair<std::string_view, std::string_view>> l){
      for(const auto [name, type] : l){
        auto it = vars.find(name);
        ASSERT_NE(it, vars.end());
        EXPECT_EQ(it->second.type()->id(), type);
      }
    };

    test(
      {
        {"x", "Int"},
        {"y", "Int"},
        {"a", "Int"},
        {"b", "Real"},
        {"c", "Char"},
        {"d", "Bool"},
      }
    );
  }

  // In-place type definitions
  {
    std::string program = R"(
        program Test;
        var
          arr : array[1..10] of Int;
          rec : record
            f1 : Int;
            f2 : Char;
            end;
        type color = (red, green, blue);
        var c : color;
        .
    )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const auto *block = parser.m_current_block;

    auto &vars = block->m_vars;
    ASSERT_EQ(vars.size(), 3);

    const Type *type = vars.at("arr").type();
    ASSERT_EQ(type->category(), TYPE_CAT::TC_ARRAY);
    const Array *arrType = static_cast<const Array *>(type);

    ASSERT_EQ(arrType->index_types().size(), 1);
    const Type *idxType = arrType->index_types()[0];
    ASSERT_EQ(idxType->category(), TYPE_CAT::TC_SUBRANGE);
    const Subrange *sub = static_cast<const Subrange *>(idxType);
    EXPECT_EQ(sub->start(), 1);
    EXPECT_EQ(sub->end(), 10);
    EXPECT_EQ(sub->category(), CONST_CAT::CC_INT);

    const Type *elemType = arrType->element_type();
    EXPECT_EQ(elemType->id(), "Int");
    EXPECT_EQ(elemType->category(), TYPE_CAT::TC_BASIC);

    type = vars.at("rec").type();
    ASSERT_EQ(type->category(), TYPE_CAT::TC_RECORD);
    const Record *recType = static_cast<const Record *>(type);

    EXPECT_EQ(recType->attributes().at("f1").type()->id(), "Int");
    EXPECT_EQ(recType->attributes().at("f2").type()->id(), "Char");

    type = vars.at("c").type();
    ASSERT_EQ(type->category(), TYPE_CAT::TC_ENUM);
    EXPECT_EQ(type->id(), "color");

    EXPECT_EQ(block->m_enums_vals.at("red").int_value(), 0);
    EXPECT_EQ(block->m_enums_vals.at("green").int_value(), 1);
    EXPECT_EQ(block->m_enums_vals.at("blue").int_value(), 2);
  }
}

TEST(ParserTest, FunctionProcedureTypes) {
  // Simple procedure type (no parameters)
  {
    std::string program = R"(
      program Test;
      type
        SimpleProcType = procedure;
      .
    )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const Type* type = Block::get("SimpleProcType", parser.m_current_block->m_types);
    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->category(), TYPE_CAT::TC_FUNCTION);
    const FunctionType* ft = static_cast<const FunctionType*>(type);
    EXPECT_EQ(ft->return_type(), nullptr);
    EXPECT_TRUE(ft->args().empty());
  }

  // Simple function type with return type (no parameters)
  {
    std::string program = R"(
      program Test;
      type
        SimpleFuncType = function : Int;
        StringFunc = function : String;
        BoolFunc = function : Bool;
      .
    )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const FunctionType* ft = static_cast<const FunctionType*>(Block::get("SimpleFuncType", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->category(), TYPE_CAT::TC_FUNCTION);
    EXPECT_EQ(ft->return_type()->id(), "Int");
    EXPECT_TRUE(ft->args().empty());

    ft = static_cast<const FunctionType*>(Block::get("StringFunc", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->return_type()->id(), "String");

    ft = static_cast<const FunctionType*>(Block::get("BoolFunc", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->return_type()->id(), "Bool");
  }

  // Procedure type with value parameters
  {
    std::string program = R"(
      program Test;
      type
        IntProc = procedure (x : Int);
        TwoIntProc = procedure (a, b : Int);
        MixedParams = procedure (x : Int; s : String; flag : Bool);
      .
    )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const FunctionType* ft = static_cast<const FunctionType*>(Block::get("IntProc", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->category(), TYPE_CAT::TC_FUNCTION);
    EXPECT_EQ(ft->return_type(), nullptr);
    EXPECT_EQ(ft->args().size(), 1);
    EXPECT_EQ(ft->args()[0].id(), "x");
    EXPECT_EQ(ft->args()[0].type()->id(), "Int");
    EXPECT_FALSE(ft->args()[0].is_ref());

    ft = static_cast<const FunctionType*>(Block::get("TwoIntProc", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->args().size(), 2);
    EXPECT_EQ(ft->args()[0].id(), "a");
    EXPECT_EQ(ft->args()[1].id(), "b");

    ft = static_cast<const FunctionType*>(Block::get("MixedParams", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->args().size(), 3);
    EXPECT_EQ(ft->args()[0].id(), "x");
    EXPECT_EQ(ft->args()[1].id(), "s");
    EXPECT_EQ(ft->args()[2].id(), "flag");
  }

  // Function type with value parameters and return type
  {
    std::string program = R"(
      program Test;
      type
        IntFunc = function (x : Int) : Int;
        RealFunc = function (x : Int; y : Real) : Real;
        StringFunc = function (a, b : Char) : String;
      .
    )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const FunctionType* ft = static_cast<const FunctionType*>(Block::get("IntFunc", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->category(), TYPE_CAT::TC_FUNCTION);
    EXPECT_EQ(ft->return_type()->id(), "Int");
    EXPECT_EQ(ft->args().size(), 1);
    EXPECT_EQ(ft->args()[0].id(), "x");
    EXPECT_EQ(ft->args()[0].type()->id(), "Int");
    EXPECT_FALSE(ft->args()[0].is_ref());

    ft = static_cast<const FunctionType*>(Block::get("RealFunc", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->return_type()->id(), "Real");
    EXPECT_EQ(ft->args().size(), 2);
    EXPECT_EQ(ft->args()[0].id(), "x");
    EXPECT_EQ(ft->args()[1].id(), "y");

    ft = static_cast<const FunctionType*>(Block::get("StringFunc", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->return_type()->id(), "String");
    EXPECT_EQ(ft->args().size(), 2);
    EXPECT_EQ(ft->args()[0].id(), "a");
    EXPECT_EQ(ft->args()[1].id(), "b");
  }

  // Procedure type with VAR (reference) parameters
  {
    std::string program = R"(
      program Test;
      type
        VarIntProc = procedure (var x : Int);
        MixedRefProc = procedure (x : Int; var y : Real; flag : Bool);
        AllRefPtrs = procedure (var a, b : Int; var s : String);
      .
    )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const FunctionType* ft = static_cast<const FunctionType*>(Block::get("VarIntProc", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->category(), TYPE_CAT::TC_FUNCTION);
    EXPECT_EQ(ft->args().size(), 1);
    EXPECT_EQ(ft->args()[0].id(), "x");
    EXPECT_TRUE(ft->args()[0].is_ref());

    ft = static_cast<const FunctionType*>(Block::get("MixedRefProc", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->args().size(), 3);
    EXPECT_FALSE(ft->args()[0].is_ref());
    EXPECT_TRUE(ft->args()[1].is_ref());
    EXPECT_FALSE(ft->args()[2].is_ref());

    ft = static_cast<const FunctionType*>(Block::get("AllRefPtrs", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->args().size(), 3);
    EXPECT_TRUE(ft->args()[0].is_ref());
    EXPECT_TRUE(ft->args()[1].is_ref());
    EXPECT_TRUE(ft->args()[2].is_ref());
  }

  // Function type with VAR parameters and return type
  {
    std::string program = R"(
      program Test;
      type
        VarIntFunc = function (var x : Int) : Int;
        ComplexFunc = function (a : Int; var b : Real; var c : String) : String;
      .
    )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const FunctionType* ft = static_cast<const FunctionType*>(Block::get("VarIntFunc", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->category(), TYPE_CAT::TC_FUNCTION);
    EXPECT_EQ(ft->return_type()->id(), "Int");
    EXPECT_EQ(ft->args().size(), 1);
    EXPECT_TRUE(ft->args()[0].is_ref());

    ft = static_cast<const FunctionType*>(Block::get("ComplexFunc", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->return_type()->id(), "String");
    EXPECT_EQ(ft->args().size(), 3);
    EXPECT_FALSE(ft->args()[0].is_ref());
    EXPECT_TRUE(ft->args()[1].is_ref());
    EXPECT_TRUE(ft->args()[2].is_ref());
  }

  // Function/procedure types with complex types (arrays, records, enums)
  {
    std::string program = R"(
      program Test;
      type
        Color = (Red, Green, Blue);
        ArrType = array[1..10] of Int;
        RecType = record x, y : Int end;
        ArrayProc = procedure (arr : ArrType);
        RecordFunc = function (rec : RecType) : Int;
        EnumProc = procedure (c : Color);
        ComplexFunc = function (arr : ArrType; rec : RecType) : Color;
      .
    )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const FunctionType* ft = static_cast<const FunctionType*>(Block::get("ArrayProc", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->args().size(), 1);
    EXPECT_EQ(ft->args()[0].id(), "arr");
    EXPECT_EQ(ft->args()[0].type()->id(), "ArrType");

    ft = static_cast<const FunctionType*>(Block::get("RecordFunc", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->return_type()->id(), "Int");
    EXPECT_EQ(ft->args().size(), 1);
    EXPECT_EQ(ft->args()[0].id(), "rec");
    EXPECT_EQ(ft->args()[0].type()->id(), "RecType");

    ft = static_cast<const FunctionType*>(Block::get("EnumProc", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->args().size(), 1);
    EXPECT_EQ(ft->args()[0].id(), "c");
    EXPECT_EQ(ft->args()[0].type()->id(), "Color");

    ft = static_cast<const FunctionType*>(Block::get("ComplexFunc", parser.m_current_block->m_types));
    ASSERT_NE(ft, nullptr);
    EXPECT_EQ(ft->return_type()->id(), "Color");
    EXPECT_EQ(ft->args().size(), 2);
    EXPECT_EQ(ft->args()[0].id(), "arr");
    EXPECT_EQ(ft->args()[1].id(), "rec");
  }
}