#pragma once
#include <gtest/gtest.h>
#include "Helper.hpp"

// =============================================================================
// EXPRESSION TESTS - Literal and Constant References
// =============================================================================

TEST(ExpressionTest, LiteralInteger) {
  std::string program = R"(
    program Test;
    var x : Int;
    begin
      x := 42;
      x := 0
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 2);

  auto* stmt1 = dynamic_cast<const AssignmentStatement*>(body->statements()[0].get());
  ASSERT_NE(stmt1, nullptr);
  EXPECT_TRUE(checkVariableAccess(stmt1->var(), "x"));
  ASSERT_NE(stmt1->expr(), nullptr);
  EXPECT_TRUE(checkIntLiteral(stmt1->expr(), 42));

  auto* stmt2 = dynamic_cast<const AssignmentStatement*>(body->statements()[1].get());
  ASSERT_NE(stmt2, nullptr);
  EXPECT_TRUE(checkVariableAccess(stmt2->var(), "x"));
  ASSERT_NE(stmt2->expr(), nullptr);
  EXPECT_TRUE(checkIntLiteral(stmt2->expr(), 0));

}

TEST(ExpressionTest, LiteralReal) {
  std::string program = R"(
    program Test;
    var x : Real;
    begin
      x := 3.14;
      x := 1.0
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 2);

  auto* stmt1 = dynamic_cast<const AssignmentStatement*>(body->statements()[0].get());
  ASSERT_NE(stmt1, nullptr);
  EXPECT_TRUE(checkRealLiteral(stmt1->expr(), Real(3.14)));

  auto* stmt2 = dynamic_cast<const AssignmentStatement*>(body->statements()[1].get());
  ASSERT_NE(stmt2, nullptr);
  EXPECT_TRUE(checkRealLiteral(stmt2->expr(), Real(1.0)));
}

TEST(ExpressionTest, LiteralChar) {
  std::string program = R"(
    program Test;
    var x : Char;
    begin
      x := 'a';
      x := '\n'
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 2);

  auto* stmt1 = dynamic_cast<const AssignmentStatement*>(body->statements()[0].get());
  ASSERT_NE(stmt1, nullptr);
  EXPECT_TRUE(checkCharLiteral(stmt1->expr(), 'a'));

  auto* stmt2 = dynamic_cast<const AssignmentStatement*>(body->statements()[1].get());
  ASSERT_NE(stmt2, nullptr);
  EXPECT_TRUE(checkCharLiteral(stmt2->expr(), '\n'));

}

TEST(ExpressionTest, LiteralBool) {
  std::string program = R"(
    program Test;
    var x : Bool;
    begin
      x := true;
      x := false
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 2);

  auto* stmt1 = dynamic_cast<const AssignmentStatement*>(body->statements()[0].get());
  ASSERT_NE(stmt1, nullptr);
  EXPECT_TRUE(checkBoolLiteral(stmt1->expr(), true));

  auto* stmt2 = dynamic_cast<const AssignmentStatement*>(body->statements()[1].get());
  ASSERT_NE(stmt2, nullptr);
  EXPECT_TRUE(checkBoolLiteral(stmt2->expr(), false));
}

TEST(ExpressionTest, VariableReference) {
  std::string program = R"(
    program Test;
    var x, myVar : Int;
    begin
      x := 10;
      myVar := x
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 2);

  auto* stmt1 = dynamic_cast<const AssignmentStatement*>(body->statements()[0].get());
  ASSERT_NE(stmt1, nullptr);
  EXPECT_TRUE(checkVariableAccess(stmt1->var(), "x"));
  EXPECT_TRUE(checkIntLiteral(stmt1->expr(), 10));

  auto* stmt2 = dynamic_cast<const AssignmentStatement*>(body->statements()[1].get());
  ASSERT_NE(stmt2, nullptr);
  EXPECT_TRUE(checkVariableAccess(stmt2->var(), "myVar"));
  EXPECT_TRUE(checkVariableAccess(stmt2->expr(), "x"));
}

TEST(ExpressionTest, ConstantReference) {
  std::string program = R"(
    program Test;
    const 
      MAX = 100;
      PI = 3.14159;
      NEWLINE = '\n';
      DEBUG = true;
    var 
      x : Int;
      y : Real;
      z : Char;
      w : Bool;
    begin
      x := MAX;
      y := PI;
      z := NEWLINE;
      w := DEBUG
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 4);

  {
    auto* stmt = dynamic_cast<const AssignmentStatement*>(body->statements()[0].get());
    ASSERT_NE(stmt, nullptr);
    // MAX should be a literal expression with value 100
    EXPECT_TRUE(checkIntLiteral(stmt->expr(), 100));
  }

  {
    auto *stmt = dynamic_cast<const AssignmentStatement *>(body->statements()[1].get());
    ASSERT_NE(stmt, nullptr);
    EXPECT_TRUE(checkRealLiteral(stmt->expr(), 3.14159));
  }

  {
    auto *stmt = dynamic_cast<const AssignmentStatement *>(body->statements()[2].get());
    ASSERT_NE(stmt, nullptr);
    EXPECT_TRUE(checkCharLiteral(stmt->expr(), '\n'));
  }

  {
    auto *stmt = dynamic_cast<const AssignmentStatement *>(body->statements()[3].get());
    ASSERT_NE(stmt, nullptr);
    EXPECT_TRUE(checkBoolLiteral(stmt->expr(), true));
  }
}

TEST(ExpressionTest, NoStringType) {
  std::string program = R"(
    program Test;
    const GREETING = "Hello";
    var 
      x : String;
      y : Int;
    begin
      y := 5
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_THROW(parser.parse(), SemanticException);
}

TEST(ExpressionTest, EnumValueReference) {
  std::string program = R"(
    program Test;
    type Color = (Red, Green, Blue);
    var c : Color;
    begin
      c := Red;
      c := Green
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 2);

  auto* stmt1 = dynamic_cast<const AssignmentStatement*>(body->statements()[0].get());
  ASSERT_NE(stmt1, nullptr);
  // Red is 0
  EXPECT_TRUE(checkLiteral<Int>(stmt1->expr(), 0, CONST_CAT::CC_ENUM));

  auto* stmt2 = dynamic_cast<const AssignmentStatement*>(body->statements()[1].get());
  ASSERT_NE(stmt2, nullptr);
  // Green is 1
  EXPECT_TRUE(checkLiteral<Int>(stmt2->expr(), 1, CONST_CAT::CC_ENUM));
}

// =============================================================================
// EXPRESSION TESTS - Unary Operations
// =============================================================================

TEST(ExpressionTest, UnaryOperators)
{
  std::string program = R"(
    program Test;
    const 
      XInt = 10;
      XReal = 3.14;
      CConst = 'a';
    var
      y1, y2, y3, y4 : Int;
      r1 : Real;
      c1, c2 : Char;
    begin
      y1 := +XInt;       
      y2 := +5;
      r1 := +XReal;
      c1 := +CConst;

      y3 := -5;
      y4 := -y3;
      r1 := -3.14;

      c2 := 'a';
      c2 := -c2
    end.
  )";

  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto &body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 9);

  auto checkUnary = [&](size_t idx, UnaryOp op) -> const UnaryExpression *
  {
    const auto *stmt = dynamic_cast<const AssignmentStatement *>(body->statements()[idx].get());
    [&]{ASSERT_NE(stmt, nullptr);}();
    const auto *unary = dynamic_cast<const UnaryExpression *>(stmt->expr());
    [&]{ASSERT_NE(unary, nullptr);}();
    EXPECT_EQ(unary->op(), op);
    return unary;
  };

  // (+XInt)
  {
    auto *unary = checkUnary(0, UnaryOp::Plus);
    EXPECT_TRUE(checkIntLiteral(unary->operand(), 10));
  }

  // (+5)
  {
    auto *unary = checkUnary(1, UnaryOp::Plus);
    EXPECT_TRUE(checkIntLiteral(unary->operand(), 5));
  }

  // (+XReal)
  {
    auto *unary = checkUnary(2, UnaryOp::Plus);
    EXPECT_TRUE(checkRealLiteral(unary->operand(), 3.14));
  }

  // (+CConst)
  {
    auto *unary = checkUnary(3, UnaryOp::Plus);
    EXPECT_TRUE(checkCharLiteral(unary->operand(), 'a'));
  }

  // (-5)
  {
    auto *unary = checkUnary(4, UnaryOp::Minus);
    EXPECT_TRUE(checkIntLiteral(unary->operand(), 5));
  }

  // (-y3)
  {
    auto *unary = checkUnary(5, UnaryOp::Minus);
    EXPECT_TRUE(checkVariableAccess(unary->operand(), "y3"));
  }

  // (-3.14)
  {
    auto *unary = checkUnary(6, UnaryOp::Minus);
    EXPECT_TRUE(checkRealLiteral(unary->operand(), 3.14));
  }

  // (-c2)
  {
    auto *unary = checkUnary(8, UnaryOp::Minus);
    EXPECT_TRUE(checkVariableAccess(unary->operand(), "c2"));
  }
}

TEST(ExpressionTest, UnaryPlusBoolInvalid) {
  std::string program = R"(
    program Test;
    var x : Int;
    var flag : Bool;
    begin
      x := +flag
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_THROW(parser.parse(), SemanticException);
}

TEST(ExpressionTest, UnaryMinusBoolInvalid) {
  std::string program = R"(
    program Test;
    var x : Int;
    const flag = true;
    begin
      x := -flag
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_THROW(parser.parse(), SemanticException);
}

TEST(ExpressionTest, NotBool) {
  std::string program = R"(
    program Test;
    var x, y : Bool;
    begin
      x := false;
      y := not x;
      y := not true
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 3);

  auto* stmt2 = dynamic_cast<const AssignmentStatement*>(body->statements()[1].get());
  ASSERT_NE(stmt2, nullptr);
  auto* unary2 = dynamic_cast<const UnaryExpression*>(stmt2->expr());
  ASSERT_NE(unary2, nullptr);
  EXPECT_EQ(unary2->op(), UnaryOp::Not);
  EXPECT_TRUE(checkVariableAccess(unary2->operand(), "x"));

  auto* stmt3 = dynamic_cast<const AssignmentStatement*>(body->statements()[2].get());
  ASSERT_NE(stmt3, nullptr);
  auto* unary3 = dynamic_cast<const UnaryExpression*>(stmt3->expr());
  ASSERT_NE(unary3, nullptr);
  EXPECT_EQ(unary3->op(), UnaryOp::Not);
  // NOT is not applied on the expression since this is the AST
  EXPECT_TRUE(checkBoolLiteral(unary3->operand(), true));
}

TEST(ExpressionTest, NotIntInvalid) {
  std::string program = R"(
    program Test;
    var x, y : Int;
    begin
      x := 5;
      y := not x
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_THROW(parser.parse(), SemanticException);
}

TEST(ExpressionTest, NotRealInvalid) {
  std::string program = R"(
    program Test;
    var x : Real;
    var y : Bool;
    begin
      x := 3.14;
      y := not x
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_THROW(parser.parse(), SemanticException);
}

// =============================================================================
// EXPRESSION TESTS - Relational and Boolean Operations
// =============================================================================

TEST(ExpressionTest, RelationalOperations) {
  std::string program = R"(
    program Test;
    var
      x, y : Int;
      c1, c2 : Char;
      a, b, result : Bool;
    begin
      { Integer }
      x := 5;
      y := 10;

      result := x = y;   { Index 2 }
      result := x <> y;  { Index 3 }
      result := x < y;   { Index 4 }
      result := x <= y;  { Index 5 }
      result := x > y;   { Index 6 }
      result := x >= y;  { Index 7 }

      { Char }
      c1 := 'a';
      c2 := 'z';

      result := c1 < c2;   { Index 10 }
      result := c1 <= c2;  { Index 11 }
      result := c1 > c2;   { Index 12 }
      result := c1 >= c2;  { Index 13 }
      result := c1 = c2;   { Index 14 }
      result := c1 <> c2;  { Index 15 }

      { Bool }
      a := true;
      b := false;

      result := a = b;   { Index 18 }
      result := a <> b   { Index 19 }
    end.
  )";

  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  EXPECT_EQ(body->statements().size(), 20);

  auto checkRelational = [&](size_t idx, RelOp op, const char* left, const char* right) {
    const auto* stmt = dynamic_cast<const AssignmentStatement*>(body->statements()[idx].get());
    ASSERT_NE(stmt, nullptr);
    
    const auto* bin = dynamic_cast<const BinaryExpression*>(stmt->expr());
    ASSERT_NE(bin, nullptr);
    
    EXPECT_EQ(bin->op(), op);
    EXPECT_TRUE(checkVariableAccess(bin->left(), left));
    EXPECT_TRUE(checkVariableAccess(bin->right(), right));
  };

  checkRelational(2, RelOp::Eq,  "x", "y");
  checkRelational(3, RelOp::Ne,  "x", "y");
  checkRelational(4, RelOp::Lt,  "x", "y");
  checkRelational(5, RelOp::Le,  "x", "y");
  checkRelational(6, RelOp::Gt,  "x", "y");
  checkRelational(7, RelOp::Ge,  "x", "y");

  checkRelational(10, RelOp::Lt,  "c1", "c2");
  checkRelational(11, RelOp::Le,  "c1", "c2");
  checkRelational(12, RelOp::Gt,  "c1", "c2");
  checkRelational(13, RelOp::Ge,  "c1", "c2");
  checkRelational(14, RelOp::Eq,  "c1", "c2");
  checkRelational(15, RelOp::Ne,  "c1", "c2");

  checkRelational(18, RelOp::Eq,  "a", "b");
  checkRelational(19, RelOp::Ne,  "a", "b");
}

TEST(ExpressionTest, LogicalOperations) {
  std::string program = R"(
    program Test;
    var
      a, b, c, d, result : Bool;
    begin
      a := true;
      b := false;
      c := true;
      d := false;

      { Or Operations }
      result := a or b;         { Index 4 }
      result := a or b or c;    { Index 5 }
      result := false or true or false; { Index 6 }

      { And Operations }
      result := a and b;        { Index 7 }
      result := a and b and c;  { Index 8 }

      { Mixed Precedence }
      result := a and b or c;   { Index 9 }
      result := (a and b) or (c and d) { Index 10 }
    end.
  )";

  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 11);

  auto getNExpr = [&](size_t idx) -> const NExpression* {
    const auto* stmt = dynamic_cast<const AssignmentStatement*>(body->statements()[idx].get());
    [&]{ASSERT_NE(stmt, nullptr);}();
    const auto* nexpr = dynamic_cast<const NExpression*>(stmt->expr());
    [&]{ASSERT_NE(nexpr, nullptr);}();
    return nexpr;
  };

  auto checkBoolLiteral = [](const Expression* expr, bool val) -> bool {
    const auto* lit = dynamic_cast<const LiteralExpression*>(expr);
    if (!lit) return false;
    return std::get<bool>(lit->constant()->value()) == val;
  };

  // 4. Check: result := a or b
  {
    auto* nexpr = getNExpr(4);
    EXPECT_EQ(nexpr->ops().size(), 1);
    EXPECT_EQ(nexpr->exprs().size(), 2);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Or);
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[0].get(), "a"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[1].get(), "b"));
  }

  // 5. Check: result := a or b or c
  {
    auto* nexpr = getNExpr(5);
    EXPECT_EQ(nexpr->ops().size(), 2);
    EXPECT_EQ(nexpr->exprs().size(), 3);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Or);
    EXPECT_EQ(nexpr->ops()[1], ALOp::Or);
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[0].get(), "a"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[1].get(), "b"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[2].get(), "c"));
  }

  // 6. Check: result := false or true or false
  {
    auto* nexpr = getNExpr(6);
    EXPECT_EQ(nexpr->ops().size(), 2);
    EXPECT_EQ(nexpr->exprs().size(), 3);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Or);
    EXPECT_EQ(nexpr->ops()[1], ALOp::Or);
    EXPECT_TRUE(checkBoolLiteral(nexpr->exprs()[0].get(), false));
    EXPECT_TRUE(checkBoolLiteral(nexpr->exprs()[1].get(), true));
    EXPECT_TRUE(checkBoolLiteral(nexpr->exprs()[2].get(), false));
  }

  // 7. Check: result := a and b
  {
    auto* nexpr = getNExpr(7);
    EXPECT_EQ(nexpr->ops().size(), 1);
    EXPECT_EQ(nexpr->exprs().size(), 2);
    EXPECT_EQ(nexpr->ops()[0], ALOp::And);
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[0].get(), "a"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[1].get(), "b"));
  }

  // 8. Check: result := a and b and c
  {
    auto* nexpr = getNExpr(8);
    EXPECT_EQ(nexpr->ops().size(), 2);
    EXPECT_EQ(nexpr->exprs().size(), 3);
    EXPECT_EQ(nexpr->ops()[0], ALOp::And);
    EXPECT_EQ(nexpr->ops()[1], ALOp::And);
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[0].get(), "a"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[1].get(), "b"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[2].get(), "c"));
  }

  // 9. Check: result := a and b or c
  {
    auto* nexpr = getNExpr(9);
    EXPECT_EQ(nexpr->ops().size(), 1);
    EXPECT_EQ(nexpr->exprs().size(), 2);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Or);

    // Check Left operand (a and b)
    const auto* left = dynamic_cast<const NExpression*>(nexpr->exprs()[0].get());
    ASSERT_NE(left, nullptr);
    EXPECT_EQ(left->ops().size(), 1);
    EXPECT_EQ(left->ops()[0], ALOp::And);
    EXPECT_TRUE(checkVariableAccess(left->exprs()[0].get(), "a"));
    EXPECT_TRUE(checkVariableAccess(left->exprs()[1].get(), "b"));

    // Check Right operand (c)
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[1].get(), "c"));
  }

  // 10. Check: result := (a and b) or (c and d)
  {
    auto* nexpr = getNExpr(10);
    EXPECT_EQ(nexpr->ops().size(), 1);
    EXPECT_EQ(nexpr->exprs().size(), 2);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Or);

    // Check Left operand (a and b)
    const auto* left = dynamic_cast<const NExpression*>(nexpr->exprs()[0].get());
    ASSERT_NE(left, nullptr);
    EXPECT_EQ(left->ops().size(), 1);
    EXPECT_EQ(left->ops()[0], ALOp::And);
    EXPECT_TRUE(checkVariableAccess(left->exprs()[0].get(), "a"));
    EXPECT_TRUE(checkVariableAccess(left->exprs()[1].get(), "b"));

    // Check Right operand (c and d)
    const auto* right = dynamic_cast<const NExpression*>(nexpr->exprs()[1].get());
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(right->ops().size(), 1);
    EXPECT_EQ(right->ops()[0], ALOp::And);
    EXPECT_TRUE(checkVariableAccess(right->exprs()[0].get(), "c"));
    EXPECT_TRUE(checkVariableAccess(right->exprs()[1].get(), "d"));
  }
}
// =============================================================================
// EXPRESSION TESTS - Arithmetic Operations
// =============================================================================

TEST(ExpressionTest, AdditiveOperations)
{
  std::string program = R"(
    program Test;
    var
      ix, iy, iz : Int;
      rx, ry, rz : Real;
    begin
      { Addition Int }
      ix := 5;
      iy := 10;
      iz := ix + iy;      { Index 2: Var + Var }
      iz := ix + 100;     { Index 3: Var + Literal }

      { Addition Real }
      rx := 3.14;
      ry := 2.86;
      rz := rx + ry;      { Index 6 }

      { Subtraction Int }
      ix := 10;
      iy := 5;
      iz := ix - iy;      { Index 9: Var - Var }
      iz := 100 - iy;     { Index 10: Literal - Var }

      { Subtraction Real }
      rx := 10.5;
      ry := 3.5;
      rz := rx - ry      { Index 13 }
    end.
  )";

  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto &body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 14);

  auto getNExpr = [&](size_t idx) -> const NExpression *
  {
    const auto *stmt = dynamic_cast<const AssignmentStatement *>(body->statements()[idx].get());
    [&]{ASSERT_NE(stmt, nullptr);}();
    const auto *nexpr = dynamic_cast<const NExpression *>(stmt->expr());
    [&]{ASSERT_NE(nexpr, nullptr);}();
    return nexpr;
  };

  // 2. Check: iz := ix + iy
  {
    auto *nexpr = getNExpr(2);
    ASSERT_EQ(nexpr->ops().size(), 1);
    ASSERT_EQ(nexpr->exprs().size(), 2);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Add);
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[0].get(), "ix"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[1].get(), "iy"));
  }

  // 3. Check: iz := ix + 100 (Completing the ignored test from input)
  {
    auto *nexpr = getNExpr(3);
    ASSERT_EQ(nexpr->ops().size(), 1);
    ASSERT_EQ(nexpr->exprs().size(), 2);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Add);
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[0].get(), "ix"));
    EXPECT_TRUE(checkIntLiteral(nexpr->exprs()[1].get(), 100));
  }

  // 6. Check: rz := rx + ry
  {
    auto *nexpr = getNExpr(6);
    ASSERT_EQ(nexpr->ops().size(), 1);
    ASSERT_EQ(nexpr->exprs().size(), 2);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Add);
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[0].get(), "rx"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[1].get(), "ry"));
  }

  // 9. Check: iz := ix - iy
  {
    auto *nexpr = getNExpr(9);
    ASSERT_EQ(nexpr->ops().size(), 1);
    ASSERT_EQ(nexpr->exprs().size(), 2);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Sub);
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[0].get(), "ix"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[1].get(), "iy"));
  }

  // 10. Check: iz := 100 - iy
  {
    auto *nexpr = getNExpr(10);
    ASSERT_EQ(nexpr->ops().size(), 1);
    ASSERT_EQ(nexpr->exprs().size(), 2);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Sub);
    EXPECT_TRUE(checkIntLiteral(nexpr->exprs()[0].get(), 100));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[1].get(), "iy"));
  }

  // 13. Check: rz := rx - ry
  {
    auto *nexpr = getNExpr(13);
    ASSERT_EQ(nexpr->ops().size(), 1);
    ASSERT_EQ(nexpr->exprs().size(), 2);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Sub);
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[0].get(), "rx"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[1].get(), "ry"));
  }
}

TEST(ExpressionTest, ComplexExpressions)
{
  std::string program = R"(
    program Test;
    var
      a, b, c, d, resInt : Int;
      rx, ry, resReal : Real;
    begin
      { Setup }
      a := 10;
      b := 5;
      c := 3;
      d := 2;
      rx := 10.0;
      ry := 2.0;

      { Chained Operations }
      resInt := a + b + c;         { Index 6 }
      resInt := a - b - c;         { Index 7 }
      resInt := a * b * c;         { Index 8 }

      { Division }
      resReal := rx / ry;          { Index 9 }
      resReal := rx / 5.0;         { Index 10 }

      { Precedence & Mixed }
      resInt := a + b * c;         { Index 11 }
      resInt := (a + b) * (c - d); { Index 12 }
      resInt := a + (b * c)        { Index 13 }
    end.
  )";

  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto &body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  EXPECT_EQ(body->statements().size(), 14);

  auto getNExpr = [&](size_t idx) -> const NExpression *
  {
    const auto *stmt = dynamic_cast<const AssignmentStatement *>(body->statements()[idx].get());
    [&]{ASSERT_NE(stmt, nullptr);}();
    const auto *nexpr = dynamic_cast<const NExpression *>(stmt->expr());
    [&]{ASSERT_NE(nexpr, nullptr);}();
    return nexpr;
  };

  // 6. Check: resInt := a + b + c
  {
    auto *nexpr = getNExpr(6);
    ASSERT_EQ(nexpr->ops().size(), 2);
    ASSERT_EQ(nexpr->exprs().size(), 3);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Add);
    EXPECT_EQ(nexpr->ops()[1], ALOp::Add);
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[0].get(), "a"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[1].get(), "b"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[2].get(), "c"));
  }

  // 7. Check: resInt := a - b - c
  {
    auto *nexpr = getNExpr(7);
    ASSERT_EQ(nexpr->ops().size(), 2);
    ASSERT_EQ(nexpr->exprs().size(), 3);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Sub);
    EXPECT_EQ(nexpr->ops()[1], ALOp::Sub);
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[0].get(), "a"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[1].get(), "b"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[2].get(), "c"));
  }

  // 8. Check: resInt := a * b * c
  {
    auto *nexpr = getNExpr(8);
    ASSERT_EQ(nexpr->ops().size(), 2);
    ASSERT_EQ(nexpr->exprs().size(), 3);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Mul);
    EXPECT_EQ(nexpr->ops()[1], ALOp::Mul);
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[0].get(), "a"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[1].get(), "b"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[2].get(), "c"));
  }

  // 9. Check: resReal := rx / ry
  {
    auto *nexpr = getNExpr(9);
    ASSERT_EQ(nexpr->ops().size(), 1);
    ASSERT_EQ(nexpr->exprs().size(), 2);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Div);
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[0].get(), "rx"));
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[1].get(), "ry"));
  }

  // 10. Check: resReal := rx / 5.0
  {
    auto *nexpr = getNExpr(10);
    ASSERT_EQ(nexpr->ops().size(), 1);
    ASSERT_EQ(nexpr->exprs().size(), 2);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Div);
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[0].get(), "rx"));
    EXPECT_TRUE(checkRealLiteral(nexpr->exprs()[1].get(), 5.0));
  }

  // 11. Check: resInt := a + b * c
  {
    auto *nexpr = getNExpr(11);
    ASSERT_EQ(nexpr->ops().size(), 1);
    ASSERT_EQ(nexpr->exprs().size(), 2);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Add);
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[0].get(), "a"));

    // Verify right operand is a multiplication
    auto *right = dynamic_cast<const NExpression *>(nexpr->exprs()[1].get());
    ASSERT_NE(right, nullptr);
    ASSERT_EQ(right->ops().size(), 1);
    EXPECT_EQ(right->ops()[0], ALOp::Mul);
    EXPECT_TRUE(checkVariableAccess(right->exprs()[0].get(), "b"));
    EXPECT_TRUE(checkVariableAccess(right->exprs()[1].get(), "c"));
  }

  // 12. Check: resInt := (a + b) * (c - d)
  {
    auto *nexpr = getNExpr(12);
    ASSERT_EQ(nexpr->ops().size(), 1);
    ASSERT_EQ(nexpr->exprs().size(), 2);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Mul);

    // Check Left (a + b)
    auto *left = dynamic_cast<const NExpression *>(nexpr->exprs()[0].get());
    ASSERT_NE(left, nullptr);
    ASSERT_EQ(left->ops().size(), 1);
    EXPECT_EQ(left->ops()[0], ALOp::Add);
    EXPECT_TRUE(checkVariableAccess(left->exprs()[0].get(), "a"));
    EXPECT_TRUE(checkVariableAccess(left->exprs()[1].get(), "b"));

    // Check Right (c - d)
    auto *right = dynamic_cast<const NExpression *>(nexpr->exprs()[1].get());
    ASSERT_NE(right, nullptr);
    ASSERT_EQ(right->ops().size(), 1);
    EXPECT_EQ(right->ops()[0], ALOp::Sub);
    EXPECT_TRUE(checkVariableAccess(right->exprs()[0].get(), "c"));
    EXPECT_TRUE(checkVariableAccess(right->exprs()[1].get(), "d"));
  }

  // 13. Check: resInt := a + (b * c)
  {
    auto *nexpr = getNExpr(13);
    ASSERT_EQ(nexpr->ops().size(), 1);
    ASSERT_EQ(nexpr->exprs().size(), 2);
    EXPECT_EQ(nexpr->ops()[0], ALOp::Add);
    EXPECT_TRUE(checkVariableAccess(nexpr->exprs()[0].get(), "a"));

    // Verify right operand is parenthesized multiplication
    auto *right = dynamic_cast<const NExpression *>(nexpr->exprs()[1].get());
    ASSERT_NE(right, nullptr);
    ASSERT_EQ(right->ops().size(), 1);
    EXPECT_EQ(right->ops()[0], ALOp::Mul);
    EXPECT_TRUE(checkVariableAccess(right->exprs()[0].get(), "b"));
    EXPECT_TRUE(checkVariableAccess(right->exprs()[1].get(), "c"));
  }
}

// =============================================================================
// EXPRESSION TESTS - Array and Record Access
// =============================================================================

TEST(ExpressionTest, VariableAndFieldAccess) {
  std::string program = R"(
    program Test;
    type Point = record x, y : Int end;
    var
      arr : array[1..10] of Int;
      matrix : array[1..5, 1..5] of Int;
      arrPoints : array[1..10] of Point;
      i, j, value : Int;
      p : Point;
    begin
      i := 1; j := 1;

      { 1D Array Access }
      arr[i] := value;         { Index 2: Variable Index }
      arr[5] := 10;            { Index 3: Literal Index }
      value := arr[3];         { Index 4: Assign }

      { Multi Dimensional }
      matrix[i, j] := value;   { Index 5: 2D Access }

      { Record Field Access }
      p.x := 10;               { Index 6: Write Field }
      p.y := 20;               { Index 7: Write Field }
      value := p.x;            { Index 8: Read Field }

      { Combined Array of Records }
      arrPoints[i].x := 10     { Index 9: Nested Access }
    end.
  )";

  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 10); 

  auto getStmt = [&](size_t idx) -> const AssignmentStatement* {
    return dynamic_cast<const AssignmentStatement*>(body->statements()[idx].get());
  };

  auto getVA = [](const Expression* expr) -> const VariableAccess* {
    return dynamic_cast<const VariableAccess*>(expr);
  };

  // --- Check 2: arr[i] := value ---
  {
    const auto* stmt = getStmt(2);
    ASSERT_NE(stmt, nullptr);
    
    const auto* lhs = getVA(stmt->var());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->base_var()->id(), "arr");
    ASSERT_EQ(lhs->selectors().size(), 1);
    
    const auto* sel = dynamic_cast<const ArraySelector*>(lhs->selectors()[0].get());
    ASSERT_NE(sel, nullptr);
    EXPECT_EQ(sel->indices().size(), 1);
    EXPECT_TRUE(checkVariableAccess(sel->indices()[0].get(), "i"));

    EXPECT_TRUE(checkVariableAccess(stmt->expr(), "value"));
  }

  // --- Check 3: arr[5] := 10 ---
  {
    const auto* stmt = getStmt(3);
    ASSERT_NE(stmt, nullptr);

    const auto* lhs = getVA(stmt->var());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->base_var()->id(), "arr");
    ASSERT_EQ(lhs->selectors().size(), 1);

    const auto* sel = dynamic_cast<const ArraySelector*>(lhs->selectors()[0].get());
    ASSERT_NE(sel, nullptr);
    EXPECT_EQ(sel->indices().size(), 1);
    EXPECT_TRUE(checkIntLiteral(sel->indices()[0].get(), 5));

    EXPECT_TRUE(checkIntLiteral(stmt->expr(), 10));
  }

  // --- Check 4: value := arr[3] ---
  {
    const auto* stmt = getStmt(4);
    ASSERT_NE(stmt, nullptr);

    EXPECT_TRUE(checkVariableAccess(stmt->var(), "value"));

    const auto* rhs = getVA(stmt->expr());
    ASSERT_NE(rhs, nullptr);
    EXPECT_EQ(rhs->base_var()->id(), "arr");
    
    const auto* sel = dynamic_cast<const ArraySelector*>(rhs->selectors()[0].get());
    ASSERT_NE(sel, nullptr);
    EXPECT_TRUE(checkIntLiteral(sel->indices()[0].get(), 3));
  }

  // --- Check 5: matrix[i, j] := value ---
  {
    const auto* stmt = getStmt(5);
    ASSERT_NE(stmt, nullptr);

    const auto* lhs = getVA(stmt->var());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->base_var()->id(), "matrix");
    
    ASSERT_EQ(lhs->selectors().size(), 1);
    const auto* sel = dynamic_cast<const ArraySelector*>(lhs->selectors()[0].get());
    ASSERT_NE(sel, nullptr);
    ASSERT_EQ(sel->indices().size(), 2);
    
    EXPECT_TRUE(checkVariableAccess(sel->indices()[0].get(), "i"));
    EXPECT_TRUE(checkVariableAccess(sel->indices()[1].get(), "j"));
  }

  // --- Check 6: p.x := 10 ---
  {
    const auto* stmt = getStmt(6);
    ASSERT_NE(stmt, nullptr);

    const auto* lhs = getVA(stmt->var());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->base_var()->id(), "p");
    ASSERT_EQ(lhs->selectors().size(), 1);

    const auto* sel = dynamic_cast<const FieldSelector*>(lhs->selectors()[0].get());
    ASSERT_NE(sel, nullptr);
    EXPECT_EQ(sel->field(), "x");
  }

  // --- Check 7: p.y := 20 ---
  {
    const auto* stmt = getStmt(7);
    ASSERT_NE(stmt, nullptr);

    const auto* lhs = getVA(stmt->var());
    ASSERT_NE(lhs, nullptr);
    const auto* sel = dynamic_cast<const FieldSelector*>(lhs->selectors()[0].get());
    ASSERT_NE(sel, nullptr);
    EXPECT_EQ(sel->field(), "y");
  }

  // --- Check 8: value := p.x ---
  {
    const auto* stmt = getStmt(8);
    ASSERT_NE(stmt, nullptr);

    const auto* rhs = getVA(stmt->expr());
    ASSERT_NE(rhs, nullptr);
    EXPECT_EQ(rhs->base_var()->id(), "p");
    
    const auto* sel = dynamic_cast<const FieldSelector*>(rhs->selectors()[0].get());
    ASSERT_NE(sel, nullptr);
    EXPECT_EQ(sel->field(), "x");
  }

  // --- Check 9: arrPoints[i].x := 10 (Nested Access) ---
  {
    const auto* stmt = getStmt(9);
    ASSERT_NE(stmt, nullptr);

    const auto* lhs = getVA(stmt->var());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->base_var()->id(), "arrPoints");
    
    // Should have 2 selectors(): Array index, then Field access
    ASSERT_EQ(lhs->selectors().size(), 2);

    const auto* arrSel = dynamic_cast<const ArraySelector*>(lhs->selectors()[0].get());
    ASSERT_NE(arrSel, nullptr);
    EXPECT_EQ(arrSel->indices().size(), 1);
    EXPECT_TRUE(checkVariableAccess(arrSel->indices()[0].get(), "i"));

    const auto* fieldSel = dynamic_cast<const FieldSelector*>(lhs->selectors()[1].get());
    ASSERT_NE(fieldSel, nullptr);
    EXPECT_EQ(fieldSel->field(), "x");
  }
}

TEST(ExpressionTest, ArrayAccessWrongIndexType) {
  std::string program = R"(
    program Test;
    var arr : array[1..10] of Int;
    var i : Char;
    begin
      arr[i] := 5
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_THROW(parser.parse(), SemanticException);
}

TEST(ExpressionTest, ArrayAccessWrongDimensions) {
  std::string program = R"(
    program Test;
    var arr : array[1..10] of Int;
    var i, j : Int;
    begin
      arr[i][j] := 5
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_THROW(parser.parse(), SemanticException);
}

TEST(ExpressionTest, InvalidFieldAccessOnNonRecord) {
  std::string program = R"(
    program Test;
    var x : Int;
    var y : Int;
    begin
      y := x.y
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_THROW(parser.parse(), SemanticException);
}

TEST(ExpressionTest, InvalidFieldName) {
  std::string program = R"(
    program Test;
    type Point = record x, y : Int end;
    var p : Point;
    begin
      p.z := 10
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_THROW(parser.parse(), SemanticException);
}

TEST(ExpressionTest, MixedTypeComparisonInvalid) {
  std::string program = R"(
    program Test;
    var x : Int;
    var y : Real;
    var result : Bool;
    begin
      x := 5;
      y := 3.14;
      result := x = y
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_THROW(parser.parse(), SemanticException);
}

TEST(ExpressionTest, MixedTypeAdditionInvalid) {
  std::string program = R"(
    program Test;
    var x : Int;
    var y : Real;
    var z : Int;
    begin
      x := 5;
      y := 3.14;
      z := x + y
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_THROW(parser.parse(), SemanticException);
}

// =============================================================================
// EXPRESSION TESTS - Function Calls
// =============================================================================

TEST(FunctionCallTest, NoArguments)
{
  std::string program = R"(
    program Test;
    function getPi : Real;
    begin
      getPi := 3.14159
    end.
    var x : Real;
    begin
      x := getPi()
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 1);

  auto* stmt = dynamic_cast<const AssignmentStatement*>(body->statements()[0].get());
  ASSERT_NE(stmt, nullptr);
  EXPECT_TRUE(checkVariableAccess(stmt->var(), "x"));

  const auto* call = dynamic_cast<const FunctionCall*>(stmt->expr());
  ASSERT_NE(call, nullptr);
  EXPECT_EQ(call->function()->id(), "getPi");
  EXPECT_EQ(call->args().size(), 0);
  EXPECT_NE(call->type(), nullptr);
}

TEST(FunctionCallTest, SingleArgument)
{
  std::string program = R"(
    program Test;
    function square(x : Int) : Int;
    begin
      square := x * x
    end.
    var y : Int;
    begin
      y := square(5)
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);

  auto* stmt = dynamic_cast<const AssignmentStatement*>(body->statements()[0].get());
  ASSERT_NE(stmt, nullptr);

  const auto* call = dynamic_cast<const FunctionCall*>(stmt->expr());
  ASSERT_NE(call, nullptr);
  EXPECT_EQ(call->function()->id(), "square");
  EXPECT_EQ(call->args().size(), 1);
  EXPECT_TRUE(checkIntLiteral(call->args()[0].get(), 5));
}

TEST(FunctionCallTest, MultipleArguments)
{
  std::string program = R"(
    program Test;
    function add(a, b : Int) : Int;
    begin
      add := a + b
    end.
    function combine(x, y, z : Int) : Int;
    begin
      combine := x + y + z
    end.
    var r1, r2 : Int;
    begin
      r1 := add(10, 20);
      r2 := combine(1, 2, 3)
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 2);

  // Check r1 := add(10, 20)
  {
    auto* stmt = dynamic_cast<const AssignmentStatement*>(body->statements()[0].get());
    ASSERT_NE(stmt, nullptr);
    const auto* call = dynamic_cast<const FunctionCall*>(stmt->expr());
    ASSERT_NE(call, nullptr);
    EXPECT_EQ(call->function()->id(), "add");
    EXPECT_EQ(call->args().size(), 2);
    EXPECT_TRUE(checkIntLiteral(call->args()[0].get(), 10));
    EXPECT_TRUE(checkIntLiteral(call->args()[1].get(), 20));
  }

  // Check r2 := combine(1, 2, 3)
  {
    auto* stmt = dynamic_cast<const AssignmentStatement*>(body->statements()[1].get());
    ASSERT_NE(stmt, nullptr);
    const auto* call = dynamic_cast<const FunctionCall*>(stmt->expr());
    ASSERT_NE(call, nullptr);
    EXPECT_EQ(call->function()->id(), "combine");
    EXPECT_EQ(call->args().size(), 3);
    EXPECT_TRUE(checkIntLiteral(call->args()[0].get(), 1));
    EXPECT_TRUE(checkIntLiteral(call->args()[1].get(), 2));
    EXPECT_TRUE(checkIntLiteral(call->args()[2].get(), 3));
  }
}

TEST(FunctionCallTest, NestedCalls)
{
  std::string program = R"(
    program Test;
    function square(x : Int) : Int;
    begin
      square := x * x
    end.
    function sum(a, b : Int) : Int;
    begin
      sum := a + b
    end.
    var result : Int;
    begin
      result := sum(square(2), square(3))
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 1);

  auto* stmt = dynamic_cast<const AssignmentStatement*>(body->statements()[0].get());
  ASSERT_NE(stmt, nullptr);

  const auto* call = dynamic_cast<const FunctionCall*>(stmt->expr());
  ASSERT_NE(call, nullptr);
  EXPECT_EQ(call->function()->id(), "sum");
  EXPECT_EQ(call->args().size(), 2);

  // Check first arg: square(2)
  {
    const auto* innerCall = dynamic_cast<const FunctionCall*>(call->args()[0].get());
    ASSERT_NE(innerCall, nullptr);
    EXPECT_EQ(innerCall->function()->id(), "square");
    EXPECT_EQ(innerCall->args().size(), 1);
    EXPECT_TRUE(checkIntLiteral(innerCall->args()[0].get(), 2));
  }

  // Check second arg: square(3)
  {
    const auto* innerCall = dynamic_cast<const FunctionCall*>(call->args()[1].get());
    ASSERT_NE(innerCall, nullptr);
    EXPECT_EQ(innerCall->function()->id(), "square");
    EXPECT_EQ(innerCall->args().size(), 1);
    EXPECT_TRUE(checkIntLiteral(innerCall->args()[0].get(), 3));
  }
}

TEST(FunctionCallTest, LiteralArguments)
{
  std::string program = R"(
    program Test;
    function max(a, b : Int) : Int;
    begin
      if a > b then max := a else max := b
    end.
    var result : Int;
    begin
      result := max(10, 20)
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);

  auto* stmt = dynamic_cast<const AssignmentStatement*>(body->statements()[0].get());
  ASSERT_NE(stmt, nullptr);

  const auto* call = dynamic_cast<const FunctionCall*>(stmt->expr());
  ASSERT_NE(call, nullptr);
  EXPECT_TRUE(checkFunctionCallWithArgs(stmt->expr(), "max", 2));
  EXPECT_TRUE(checkIntLiteral(call->args()[0].get(), 10));
  EXPECT_TRUE(checkIntLiteral(call->args()[1].get(), 20));
}

TEST(FunctionCallTest, VariableArguments)
{
  std::string program = R"(
    program Test;
    function add(a, b : Int) : Int;
    begin
      add := a + b
    end.
    var x, y, result : Int;
    begin
      x := 10;
      y := 20;
      result := add(x, y)
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);

  auto* stmt = dynamic_cast<const AssignmentStatement*>(body->statements()[2].get());
  ASSERT_NE(stmt, nullptr);

  const auto* call = dynamic_cast<const FunctionCall*>(stmt->expr());
  ASSERT_NE(call, nullptr);
  EXPECT_TRUE(checkFunctionCallWithArgs(stmt->expr(), "add", 2));
  EXPECT_TRUE(checkVariableAccess(call->args()[0].get(), "x"));
  EXPECT_TRUE(checkVariableAccess(call->args()[1].get(), "y"));
}

TEST(FunctionCallTest, DifferentReturnTypes)
{
  std::string program = R"(
    program Test;
    function intFunc : Int;
    begin
      intFunc := 42
    end.
    function realFunc : Real;
    begin
      realFunc := 3.14
    end.
    function boolFunc : Bool;
    begin
      boolFunc := true
    end.
    function charFunc : Char;
    begin
      charFunc := 'a'
    end.
    var i : Int;
    var r : Real;
    var b : Bool;
    var c : Char;
    begin
      i := intFunc();
      r := realFunc();
      b := boolFunc();
      c := charFunc()
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 4);

  // Check i := intFunc
  {
    auto* stmt = dynamic_cast<const AssignmentStatement*>(body->statements()[0].get());
    ASSERT_NE(stmt, nullptr);
    EXPECT_TRUE(checkFunctionCall(stmt->expr(), "intFunc"));
  }

  // Check r := realFunc
  {
    auto* stmt = dynamic_cast<const AssignmentStatement*>(body->statements()[1].get());
    ASSERT_NE(stmt, nullptr);
    EXPECT_TRUE(checkFunctionCall(stmt->expr(), "realFunc"));
  }

  // Check b := boolFunc
  {
    auto* stmt = dynamic_cast<const AssignmentStatement*>(body->statements()[2].get());
    ASSERT_NE(stmt, nullptr);
    EXPECT_TRUE(checkFunctionCall(stmt->expr(), "boolFunc"));
  }

  // Check c := charFunc
  {
    auto* stmt = dynamic_cast<const AssignmentStatement*>(body->statements()[3].get());
    ASSERT_NE(stmt, nullptr);
    EXPECT_TRUE(checkFunctionCall(stmt->expr(), "charFunc"));
  }
}

TEST(FunctionCallTest, WrongArgumentCountTooFew)
{
  std::string program = R"(
    program Test;
    function add(a, b : Int) : Int;
    begin
      add := a + b
    end.
    var result : Int;
    begin
      result := add(5)
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_THROW(parser.parse(), SemanticException);
}

TEST(FunctionCallTest, WrongArgumentCountTooMany)
{
  std::string program = R"(
    program Test;
    function square(x : Int) : Int;
    begin
      square := x * x
    end.
    var result : Int;
    begin
      result := square(5, 10)
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_THROW(parser.parse(), SemanticException);
}

TEST(FunctionCallTest, WrongArgumentType)
{
  std::string program = R"(
    program Test;
    function expectsInt(x : Int) : Int;
    begin
      expectsInt := x
    end.
    var result : Int;
    var s : Real;
    begin
      s := "hello";
      result := expectsInt(s)
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_THROW(parser.parse(), SemanticException);
}

TEST(FunctionCallTest, VarParameterValid)
{
  std::string program = R"(
    program Test;
    procedure swap(var a, b : Int);
    var temp : Int;
    begin
      temp := a;
      a := b;
      b := temp
    end.
    var x, y : Int;
    begin
      x := 10;
      y := 20;
      swap(x, y)
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 3);

  // Check swap(x, y) statement
  auto* stmt = dynamic_cast<const ProcedureCall*>(body->statements()[2].get());
  ASSERT_NE(stmt, nullptr);
  EXPECT_TRUE(checkProcedureCall(stmt, "swap"));
  EXPECT_EQ(stmt->args().size(), 2);
  EXPECT_TRUE(checkVariableAccess(stmt->args()[0].get(), "x"));
  EXPECT_TRUE(checkVariableAccess(stmt->args()[1].get(), "y"));
}

TEST(FunctionCallTest, VarParameterInvalidExpression)
{
  std::string program = R"(
    program Test;
    procedure swap(var a, b : Int);
    var temp : Int;
    begin
      temp := a;
      a := b;
      b := temp
    end.
    var x, y : Int;
    begin
      x := 10;
      y := 20;
      swap(x + 5, y)
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_THROW(parser.parse(), SemanticException);
}

TEST(FunctionCallTest, FunctionCalledAsExpression)
{
  std::string program = R"(
    program Test;
    function square(x : Int) : Int;
    begin
      square := x * x
    end.
    var result : Int;
    begin
      result := square(5)
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);

  auto* stmt = dynamic_cast<const AssignmentStatement*>(body->statements()[0].get());
  ASSERT_NE(stmt, nullptr);

  // Verify it's a FunctionCall (expression) not a ProcedureCall
  const auto* call = dynamic_cast<const FunctionCall*>(stmt->expr());
  ASSERT_NE(call, nullptr);
  EXPECT_EQ(call->function()->id(), "square");
  ASSERT_NE(call->type(), nullptr); // Has return type
}

TEST(FunctionCallTest, ProcedureCalledAsStatement)
{
  std::string program = R"(
    program Test;
    procedure pr;
    var tmp : Int;
    begin
      tmp := 2
    end.
    begin
      pr()
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 1);

  // Verify it's a ProcedureCall (statement) not a FunctionCall
  auto* stmt = dynamic_cast<const ProcedureCall*>(body->statements()[0].get());
  ASSERT_NE(stmt, nullptr);
  EXPECT_EQ(stmt->procedure()->id(), "pr");
  EXPECT_EQ(stmt->args().size(), 0);
}

TEST(FunctionCallTest, ParenthesesWithNoArgs)
{
  std::string program = R"(
    program Test;
    function getPi : Real;
    begin
      getPi := 3.14159
    end.
    var x : Real;
    begin
      x := getPi()
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());
}