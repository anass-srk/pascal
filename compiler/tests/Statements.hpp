#pragma once
#include <gtest/gtest.h>
#include "Helper.hpp"

// =============================================================================
// LABELED STATEMENT TESTS
// =============================================================================

TEST(LabeledStatementTest, BasicLabeledStatement) {
  std::string program = R"(
    program Test;
    label asd;
    var x : Int;
    begin
      asd: x := 42
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 1);

  auto *labeled = dynamic_cast<const LabeledStatement *>(body->statements()[0].get());
  ASSERT_NE(labeled, nullptr);
  ASSERT_NE(labeled->label(), nullptr);
  EXPECT_EQ(labeled->label()->id(), "asd");
  ASSERT_NE(labeled->statement(), nullptr);
}

TEST(LabeledStatementTest, MultipleLabeledStatements) {
  std::string program = R"(
    program Test;
    label 10, 20, 30;
    var x, y, z : Int;
    begin
      10: x := 1;
      20: y := 2;
      30: z := 3
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 3);

  for (size_t i = 0; i < 3; ++i) {
    auto* labeled = dynamic_cast<const LabeledStatement*>(body->statements()[i].get());
    ASSERT_NE(labeled, nullptr);
    EXPECT_EQ(labeled->label()->id(), std::to_string((i + 1) * 10));
  }
}

// =============================================================================
// WHILE STATEMENT TESTS
// =============================================================================

TEST(WhileStatementTest, SimpleWhileLoop) {
  std::string program = R"(
    program Test;
    var x : Int;
    begin
      while x < 10 do x := x + 1
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 1);

  auto* whileStmt = dynamic_cast<const WhileStatement*>(body->statements()[0].get());
  ASSERT_NE(whileStmt, nullptr);
  EXPECT_TRUE(checkCondition(whileStmt->condition()));
  ASSERT_NE(whileStmt->body(), nullptr);
}

TEST(WhileStatementTest, WhileLoopWithBooleanCondition) {
  std::string program = R"(
    program Test;
    var flag : Bool;
    begin
      while not flag do flag := true
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 1);

  auto* whileStmt = dynamic_cast<const WhileStatement*>(body->statements()[0].get());
  ASSERT_NE(whileStmt, nullptr);
  EXPECT_TRUE(checkCondition(whileStmt->condition()));
  ASSERT_NE(whileStmt->body(), nullptr);
}

TEST(WhileStatementTest, NestedWhileLoops) {
  std::string program = R"(
    program Test;
    var i, j : Int;
    begin
      while i < 10 do
        while j < 10 do j := j + 1
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 1);

  auto* outerWhile = dynamic_cast<const WhileStatement*>(body->statements()[0].get());
  ASSERT_NE(outerWhile, nullptr);
  EXPECT_TRUE(checkCondition(outerWhile->condition()));
  ASSERT_NE(outerWhile->body(), nullptr);

  auto* innerWhile = dynamic_cast<const WhileStatement*>(outerWhile->body());
  ASSERT_NE(innerWhile, nullptr);
  EXPECT_TRUE(checkCondition(innerWhile->condition()));
  ASSERT_NE(innerWhile->body(), nullptr);
}

// =============================================================================
// REPEAT STATEMENT TESTS
// =============================================================================

TEST(RepeatStatementTest, SimpleRepeatUntilLoop) {
  std::string program = R"(
    program Test;
    var x : Int;
    begin
      repeat x := x + 1 until x >= 10
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 1);

  auto* repeatStmt = dynamic_cast<const RepeatStatement*>(body->statements()[0].get());
  ASSERT_NE(repeatStmt, nullptr);
  EXPECT_TRUE(checkCondition(repeatStmt->condition()));
  EXPECT_EQ(repeatStmt->body().size(), 1);
}

TEST(RepeatStatementTest, RepeatLoopWithMultipleStatements) {
  std::string program = R"(
    program Test;
    var x, y : Int;
    begin
      repeat
        x := x + 1;
        y := y + 1
      until x + y >= 10
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 1);

  auto* repeatStmt = dynamic_cast<const RepeatStatement*>(body->statements()[0].get());
  ASSERT_NE(repeatStmt, nullptr);
  EXPECT_TRUE(checkCondition(repeatStmt->condition()));
  EXPECT_EQ(repeatStmt->body().size(), 2);
}

// =============================================================================
// FOR STATEMENT TESTS
// =============================================================================

template <typename T>
static void checkFor(const ForStatement *stmt, std::string_view id, bool increasing, T beg, T end)
  requires std::is_same_v<T, Int> || std::is_same_v<T, char> || std::is_same_v<T, bool> 
{
  ASSERT_NE(stmt, nullptr);
  EXPECT_NE(stmt->body(), nullptr);
  EXPECT_EQ(stmt->increasing(), increasing);
  EXPECT_TRUE(checkVariableAccess(stmt->var(), id));
  EXPECT_EQ(stmt->var()->type()->get_underlying_type(), stmt->start().type());
  EXPECT_EQ(stmt->end().type(), stmt->start().type());
  ASSERT_TRUE(std::holds_alternative<T>(stmt->start().value()));
  ASSERT_TRUE(std::holds_alternative<T>(stmt->end().value()));
  EXPECT_EQ(std::get<T>(stmt->start().value()), beg);
  EXPECT_EQ(std::get<T>(stmt->end().value()), end);
}


TEST(ForStatementTest, ForLoopWithToIncreasing) {
  std::string program = R"(
    program Test;
    var i : Int;
    begin
      for i := 1 to 10 do i := i
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 1);

  auto* forStmt = dynamic_cast<const ForStatement*>(body->statements()[0].get());
  checkFor<Int>(forStmt, "i", true, 1, 10);
}

TEST(ForStatementTest, ForLoopWithDownToDecreasing) {
  std::string program = R"(
    program Test;
    var i : Int;
    begin
      for i := 10 downto 1 do i := i
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 1);

  auto* forStmt = dynamic_cast<const ForStatement*>(body->statements()[0].get());
  checkFor<Int>(forStmt, "i", false, 10, 1);
}

TEST(ForStatementTest, ForLoopWithCharType) {
  std::string program = R"(
    program Test;
    var c : Char;
    begin
      for c := 'a' to 'z' do c := c
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 1);

  auto* forStmt = dynamic_cast<const ForStatement*>(body->statements()[0].get());
  checkFor(forStmt, "c", true, 'a', 'z');
}

TEST(ForStatementTest, ForLoopWithBoolType) {
  std::string program = R"(
    program Test;
    var b : Bool;
    begin
      for b := false to true do b := b
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  const auto& body = parser.m_current_block->body;
  ASSERT_NE(body, nullptr);
  ASSERT_EQ(body->statements().size(), 1);

  auto* forStmt = dynamic_cast<const ForStatement*>(body->statements()[0].get());
  checkFor(forStmt, "b", true, false, true);
}

TEST(ForStatementTest, ForLoopWithInvalidOrder)
{
  std::string program = R"(
    program Test;
    var i : Int;
    begin
      for i := 10 to 1 do i := i
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_THROW(parser.parse(), SemanticException);
}

  // =============================================================================
  // IF STATEMENT TESTS
  // =============================================================================

  TEST(IfStatementTest, SimpleIfWithoutElse)
  {
    std::string program = R"(
    program Test;
    var x : Int;
    begin
      if x > 0 then x := 1
    end.
  )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const auto &body = parser.m_current_block->body;
    ASSERT_NE(body, nullptr);
    ASSERT_EQ(body->statements().size(), 1);

    auto *ifStmt = dynamic_cast<const IfStatement *>(body->statements()[0].get());
    ASSERT_NE(ifStmt, nullptr);
    EXPECT_TRUE(checkCondition(ifStmt->condition()));
    ASSERT_NE(ifStmt->then_stmt(), nullptr);
    ASSERT_EQ(ifStmt->else_stmt(), nullptr);
  }

  TEST(IfStatementTest, IfWithElseBranch)
  {
    std::string program = R"(
    program Test;
    var x : Int;
    begin
      if x > 0 then x := 1 else x := -1
    end.
  )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const auto &body = parser.m_current_block->body;
    ASSERT_NE(body, nullptr);
    ASSERT_EQ(body->statements().size(), 1);

    auto *ifStmt = dynamic_cast<const IfStatement *>(body->statements()[0].get());
    ASSERT_NE(ifStmt, nullptr);
    EXPECT_TRUE(checkCondition(ifStmt->condition()));
    ASSERT_NE(ifStmt->then_stmt(), nullptr);
    ASSERT_NE(ifStmt->else_stmt(), nullptr);
  }

  TEST(IfStatementTest, NestedIfStatements)
  {
    std::string program = R"(
    program Test;
    var x : Int;
    begin
      if x > 0 then
        if x > 10 then x := 10 else x := x
    end.
  )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const auto &body = parser.m_current_block->body;
    ASSERT_NE(body, nullptr);
    ASSERT_EQ(body->statements().size(), 1);

    auto *outerIf = dynamic_cast<const IfStatement *>(body->statements()[0].get());
    ASSERT_NE(outerIf, nullptr);
    EXPECT_TRUE(checkCondition(outerIf->condition()));
    ASSERT_NE(outerIf->then_stmt(), nullptr);
    ASSERT_EQ(outerIf->else_stmt(), nullptr);
    // else part belongs to the inner if-statement
    auto *innerIf = dynamic_cast<const IfStatement *>(outerIf->then_stmt());
    ASSERT_NE(innerIf, nullptr);
    EXPECT_TRUE(checkCondition(innerIf->condition()));
    ASSERT_NE(outerIf->then_stmt(), nullptr);
    ASSERT_NE(innerIf->else_stmt(), nullptr);
  }

  // =============================================================================
  // CASE STATEMENT TESTS
  // =============================================================================

  template <typename T>
  static void checkCase(const CaseStatement *stmt, std::initializer_list<std::initializer_list<T>> ll)
    requires std::is_same_v<T, Int> || std::is_same_v<T, char> || std::is_same_v<T, bool>
  {
    ASSERT_NE(stmt, nullptr);
    ASSERT_EQ(stmt->alternatives().size(), ll.size());
    int index = 0;
    for (const auto &l : ll)
    {
      const auto &alt = stmt->alternatives()[index++];
      ASSERT_EQ(alt.labels().size(), l.size());
      EXPECT_NE(alt.statement(), nullptr);

      int i = 0;
      for (auto v : l)
      {
        ASSERT_TRUE(std::holds_alternative<T>(alt.labels()[i].value()));
        EXPECT_EQ(v, std::get<T>(alt.labels()[i].value()));
        ++i;
      }
    }
  }

  TEST(CaseStatementTest, SimpleCaseWithIntegerSelector)
  {
    std::string program = R"(
    program Test;
    var x : Int;
    begin
      case x of
        1: x := 10;
        2: x := 20
      end
    end.
  )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const auto &body = parser.m_current_block->body;
    ASSERT_NE(body, nullptr);
    ASSERT_EQ(body->statements().size(), 1);

    auto *caseStmt = dynamic_cast<const CaseStatement *>(body->statements()[0].get());
    checkCase<Int>(caseStmt, {{1}, {2}});
  }

  TEST(CaseStatementTest, CaseWithMultipleLabelsPerAlternative)
  {
    std::string program = R"(
    program Test;
    var x : Int;
    begin
      case x of
        1, 3, 5: x := 1;
        2, 4, 6: x := 0
      end
    end.
  )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const auto &body = parser.m_current_block->body;
    ASSERT_NE(body, nullptr);
    ASSERT_EQ(body->statements().size(), 1);

    auto *caseStmt = dynamic_cast<const CaseStatement *>(body->statements()[0].get());
    checkCase<Int>(caseStmt, {{1, 3, 5}, {2, 4, 6}});
  }

  TEST(CaseStatementTest, CaseWithCharSelector)
  {
    std::string program = R"(
    program Test;
    var c : Char;
    begin
      case c of
        'a': c := 'z';
        'z': c := 'a'
      end
    end.
  )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const auto &body = parser.m_current_block->body;
    ASSERT_NE(body, nullptr);
    ASSERT_EQ(body->statements().size(), 1);

    auto *caseStmt = dynamic_cast<const CaseStatement *>(body->statements()[0].get());
    checkCase(caseStmt, {{'a'}, {'z'}});
  }

  TEST(CaseStatementTest, CaseWithBooleanSelector)
  {
    std::string program = R"(
    program Test;
    var b : Bool;
    begin
      case b of
        true: b := false;
        false: b := true
      end
    end.
  )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const auto &body = parser.m_current_block->body;
    ASSERT_NE(body, nullptr);
    ASSERT_EQ(body->statements().size(), 1);

    auto *caseStmt = dynamic_cast<const CaseStatement *>(body->statements()[0].get());
    checkCase(caseStmt, {{true}, {false}});
  }

  // =============================================================================
  // COMPOUND STATEMENT TESTS
  // =============================================================================

  TEST(CompoundStatementTest, NestedCompoundStatements)
  {
    std::string program = R"(
    program Test;
    var x : Int;
    begin
      begin
        begin
          x := 1
        end;
        x := 2
      end;
      x := 3
    end.
  )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const auto &body = parser.m_current_block->body;
    ASSERT_NE(body, nullptr);
    ASSERT_EQ(body->statements().size(), 2);

    auto *outerComp = dynamic_cast<const CompoundStatement *>(body->statements()[0].get());
    ASSERT_NE(outerComp, nullptr);
    EXPECT_EQ(outerComp->statements().size(), 2);

    auto *innerComp = dynamic_cast<const CompoundStatement *>(outerComp->statements()[0].get());
    ASSERT_NE(innerComp, nullptr);
    EXPECT_EQ(innerComp->statements().size(), 1);
  }

  TEST(CompoundStatementTest, NestedCompoundWithSingleStatement)
  {
    std::string program = R"(
    program Test;
    var x : Int;
    begin
      begin x := 1 end;
      x := 2
    end.
  )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const auto &body = parser.m_current_block->body;
    ASSERT_NE(body, nullptr);
    ASSERT_EQ(body->statements().size(), 2);

    auto *stmt1 = dynamic_cast<const CompoundStatement *>(body->statements()[0].get());
    ASSERT_NE(stmt1, nullptr);
    EXPECT_EQ(stmt1->statements().size(), 1);
  }

  // =============================================================================
  // GOTO STATEMENT TESTS
  // =============================================================================

  TEST(GotoStatementTest, BasicGotoToLabel)
  {
    std::string program = R"(
    program Test;
    label 10;
    var x : Int;
    begin
      goto 10;
      10: x := 1
    end.
  )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const auto &body = parser.m_current_block->body;
    ASSERT_NE(body, nullptr);
    ASSERT_EQ(body->statements().size(), 2);

    auto *gotoStmt = dynamic_cast<const GotoStatement *>(body->statements()[0].get());
    ASSERT_NE(gotoStmt, nullptr);
    EXPECT_EQ(gotoStmt->label()->id(), "10");

    auto *labelStmt = dynamic_cast<const LabeledStatement *>(body->statements()[1].get());
    ASSERT_NE(labelStmt, nullptr);
    EXPECT_EQ(labelStmt->label(), gotoStmt->label());
  }

  TEST(GotoStatementTest, GotoWithLabelDeclaration)
  {
    std::string program = R"(
    program Test;
    label 10, 20;
    var x, y : Int;
    begin
      goto 10;
      10: x := 1;
      goto 20;
      20: y := 2
    end.
  )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const auto &body = parser.m_current_block->body;
    ASSERT_NE(body, nullptr);
    ASSERT_EQ(body->statements().size(), 4);

    auto *gotoStmt = dynamic_cast<const GotoStatement *>(body->statements()[0].get());
    ASSERT_NE(gotoStmt, nullptr);
    EXPECT_EQ(gotoStmt->label()->id(), "10");

    auto *labelStmt = dynamic_cast<const LabeledStatement *>(body->statements()[1].get());
    ASSERT_NE(labelStmt, nullptr);
    EXPECT_EQ(labelStmt->label(), gotoStmt->label());

    auto *gotoStmt2 = dynamic_cast<const GotoStatement *>(body->statements()[2].get());
    ASSERT_NE(gotoStmt2, nullptr);
    EXPECT_EQ(gotoStmt2->label()->id(), "20");

    auto *labelStmt2 = dynamic_cast<const LabeledStatement *>(body->statements()[3].get());
    ASSERT_NE(labelStmt2, nullptr);
    EXPECT_EQ(labelStmt2->label(), gotoStmt2->label());
  }

  // =============================================================================
  // COMPOSITE STATEMENT TESTS (Multiple statement types in one test)
  // =============================================================================

  TEST(StatementIntegrationTest, MixedStatements)
  {
    std::string program = R"(
    program Test;
    var x, i : Int;
    b : Bool;
    begin
      x := 0;
      while x < 10 do x := x + 1;
      if b then x := 10 else x := 0;
      for i := 1 to 5 do x := x + i;
      repeat x := x - 1 until x <= 0
    end.
  )";
    Parser parser(std::move(program));
    EXPECT_NO_THROW(parser.parse());

    const auto &body = parser.m_current_block->body;
    ASSERT_NE(body, nullptr);
    ASSERT_EQ(body->statements().size(), 5);

    EXPECT_TRUE(dynamic_cast<const AssignmentStatement *>(body->statements()[0].get()) != nullptr);
    EXPECT_TRUE(dynamic_cast<const WhileStatement *>(body->statements()[1].get()));
    EXPECT_TRUE(dynamic_cast<const IfStatement *>(body->statements()[2].get()));
    EXPECT_TRUE(dynamic_cast<const ForStatement *>(body->statements()[3].get()));
    EXPECT_TRUE(dynamic_cast<const RepeatStatement *>(body->statements()[4].get()));
  }
