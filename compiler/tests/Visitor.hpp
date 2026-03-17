#pragma once
#include <gtest/gtest.h>
#include <vector>
#include "Helper.hpp"

class NodeCounter : public CombinedVisitor
{
public:
  int expressions = 0;
  int statements = 0;

  // =============================================================================
  // Expression visitors - each visits its nested expressions
  // =============================================================================

  void visit(const LiteralExpression &node, const Context &ctx) override
  {
    expressions++;
  }

  void visit(const UnaryExpression &node, const Context &ctx) override
  {
    expressions++;
    node.operand()->accept(*this, ctx);
  }

  void visit(const BinaryExpression &node, const Context &ctx) override
  {
    expressions++;
    node.left()->accept(*this, ctx);
    node.right()->accept(*this, ctx);
  }

  void visit(const NExpression &node, const Context &ctx) override
  {
    expressions++;
    for (const auto &expr : node.exprs())
    {
      expr->accept(*this, ctx);
    }
  }

  void visit(const VariableAccess &node, const Context &ctx) override
  {
    expressions++;
    // Visit any selectors (array indices, field accesses)
    for (const auto &sel : node.selectors())
    {
      sel->accept(*this, ctx);
    }
  }

  void visit(const FunctionCall &node, const Context &ctx) override
  {
    expressions++;
    for (const auto &arg : node.args())
    {
      arg->accept(*this, ctx);
    }
  }

  // =============================================================================
  // Statement visitors - each visits its nested expressions and statements
  // =============================================================================

  void visit(const LabeledStatement &node, const Context &ctx) override
  {
    statements++;
    node.statement()->accept(*this, ctx);
  }

  void visit(const AssignmentStatement &node, const Context &ctx) override
  {
    statements++;
    node.var()->accept(*this, ctx); // VariableAccess
    node.expr()->accept(*this, ctx); // Expression
  }

  void visit(const ProcedureCall &node, const Context &ctx) override
  {
    statements++;
    for (const auto &arg : node.args())
    {
      arg->accept(*this, ctx);
    }
  }

  void visit(const ReadStatement &node, const Context &ctx) override
  {
    statements++;
    for (const auto &arg : node.args())
    {
      arg->accept(*this, ctx);
    }
  }

  void visit(const WriteStatement &node, const Context &ctx) override
  {
    statements++;
    for (const auto &arg : node.args())
    {
      arg->accept(*this, ctx);
    }
  }

  void visit(const GotoStatement &node, const Context &ctx) override
  {
    statements++;
    // No nested expressions or statements
  }

  void visit(const CompoundStatement &node, const Context &ctx) override
  {
    statements++;
    for (const auto &stmt : node.statements())
    {
      stmt->accept(*this, ctx);
    }
  }

  void visit(const WhileStatement &node, const Context &ctx) override
  {
    statements++;
    node.condition()->accept(*this, ctx);
    node.body()->accept(*this, ctx);
  }

  void visit(const RepeatStatement &node, const Context &ctx) override
  {
    statements++;
    for (const auto &stmt : node.body())
    {
      stmt->accept(*this, ctx);
    }
    node.condition()->accept(*this, ctx);
  }

  void visit(const ForStatement &node, const Context &ctx) override
  {
    statements++;
    node.var()->accept(*this, ctx);
    node.body()->accept(*this, ctx);
  }

  void visit(const IfStatement &node, const Context &ctx) override
  {
    statements++;
    node.condition()->accept(*this, ctx);
    node.then_stmt()->accept(*this, ctx);
    if (node.else_stmt())
    {
      node.else_stmt()->accept(*this, ctx);
    }
  }

  void visit(const CaseStatement &node, const Context &ctx) override
  {
    statements++;
    node.selector()->accept(*this, ctx);
    for (const auto &alt : node.alternatives())
    {
      alt.statement()->accept(*this, ctx);
    }
  }

  // =============================================================================
  // Select visitors - Selectors
  // =============================================================================

  void visit(const ArraySelector &node, const Context &ctx) override
  {
    for (const auto &idx : node.indices())
    {
      idx->accept(*this, ctx);
    }
  }

  void visit(const FieldSelector &node, const Context &ctx) override
  {
    // No expression to visit, just a field name
  }
};

template <typename T>
class TypeExtractor : public CombinedVisitor
{
public:
  std::vector<const T *> nodes;

  // =============================================================================
  // Expression visitors - traverse and extract matching types
  // =============================================================================

  void visit(const LiteralExpression &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, LiteralExpression>)
    {
      nodes.push_back(&node);
    }
    // No children to visit
  }

  void visit(const UnaryExpression &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, UnaryExpression>)
    {
      nodes.push_back(&node);
    }
    node.operand()->accept(*this, ctx);
  }

  void visit(const BinaryExpression &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, BinaryExpression>)
    {
      nodes.push_back(&node);
    }
    node.left()->accept(*this, ctx);
    node.right()->accept(*this, ctx);
  }

  void visit(const NExpression &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, NExpression>)
    {
      nodes.push_back(&node);
    }
    for (const auto &expr : node.exprs())
    {
      expr->accept(*this, ctx);
    }
  }

  void visit(const VariableAccess &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, VariableAccess>)
    {
      nodes.push_back(&node);
    }
    for (const auto &sel : node.selectors())
    {
      sel->accept(*this, ctx);
    }
  }

  void visit(const FunctionCall &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, FunctionCall>)
    {
      nodes.push_back(&node);
    }
    for (const auto &arg : node.args())
    {
      arg->accept(*this, ctx);
    }
  }

  // =============================================================================
  // Statement visitors - traverse and extract matching types
  // =============================================================================

  void visit(const LabeledStatement &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, LabeledStatement>)
    {
      nodes.push_back(&node);
    }
    node.statement()->accept(*this, ctx);
  }

  void visit(const AssignmentStatement &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, AssignmentStatement>)
    {
      nodes.push_back(&node);
    }
    node.var()->accept(*this, ctx);
    node.expr()->accept(*this, ctx);
  }

  void visit(const ProcedureCall &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, ProcedureCall>)
    {
      nodes.push_back(&node);
    }
    for (const auto &arg : node.args())
    {
      arg->accept(*this, ctx);
    }
  }

  void visit(const ReadStatement &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, ReadStatement>)
    {
      nodes.push_back(&node);
    }
    for (const auto &arg : node.args())
    {
      arg->accept(*this, ctx);
    }
  }

  void visit(const WriteStatement &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, WriteStatement>)
    {
      nodes.push_back(&node);
    }
    for (const auto &arg : node.args())
    {
      arg->accept(*this, ctx);
    }
  }

  void visit(const GotoStatement &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, GotoStatement>)
    {
      nodes.push_back(&node);
    }
  }

  void visit(const CompoundStatement &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, CompoundStatement>)
    {
      nodes.push_back(&node);
    }
    for (const auto &stmt : node.statements())
    {
      stmt->accept(*this, ctx);
    }
  }

  void visit(const WhileStatement &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, WhileStatement>)
    {
      nodes.push_back(&node);
    }
    node.condition()->accept(*this, ctx);
    node.body()->accept(*this, ctx);
  }

  void visit(const RepeatStatement &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, RepeatStatement>)
    {
      nodes.push_back(&node);
    }
    for (const auto &stmt : node.body())
    {
      stmt->accept(*this, ctx);
    }
    node.condition()->accept(*this, ctx);
  }

  void visit(const ForStatement &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, ForStatement>)
    {
      nodes.push_back(&node);
    }
    node.var()->accept(*this, ctx);
    node.body()->accept(*this, ctx);
  }

  void visit(const IfStatement &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, IfStatement>)
    {
      nodes.push_back(&node);
    }
    node.condition()->accept(*this, ctx);
    node.then_stmt()->accept(*this, ctx);
    if (node.else_stmt())
    {
      node.else_stmt()->accept(*this, ctx);
    }
  }

  void visit(const CaseStatement &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, CaseStatement>)
    {
      nodes.push_back(&node);
    }
    node.selector()->accept(*this, ctx);
    for (const auto &alt : node.alternatives())
    {
      alt.statement()->accept(*this, ctx);
    }
  }

  // =============================================================================
  // Selector visitors
  // =============================================================================

  void visit(const ArraySelector &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, ArraySelector>)
    {
      nodes.push_back(&node);
    }
    for (const auto &idx : node.indices())
    {
      idx->accept(*this, ctx);
    }
  }

  void visit(const FieldSelector &node, const Context &ctx) override
  {
    if constexpr (std::is_same_v<T, FieldSelector>)
    {
      nodes.push_back(&node);
    }
  }
};

TEST(VisitorTest, Pattern_NodeCounting)
{
  std::string program = R"(
    program Test;
    var x, y : Int;
    begin
      x := 1;
      y := 2;
      x := x + y
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  NodeCounter counter;
  parser.m_current_block->body->accept(counter, *parser.m_current_block);

  EXPECT_EQ(counter.statements, 4);
  EXPECT_EQ(counter.expressions, 8);
}

TEST(VisitorTest, Pattern_TypeExtractor_IntLiterals)
{
  std::string program = R"(
    program Test;
    var x, y : Int;
    begin
      x := 1;
      y := 2;
      x := 42
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  TypeExtractor<LiteralExpression> extractor;
  parser.m_current_block->body->accept(extractor, *parser.m_current_block);

  ASSERT_EQ(extractor.nodes.size(), 3);
  EXPECT_EQ(std::get<Int>(extractor.nodes[0]->constant()->value()), 1);
  EXPECT_EQ(std::get<Int>(extractor.nodes[1]->constant()->value()), 2);
  EXPECT_EQ(std::get<Int>(extractor.nodes[2]->constant()->value()), 42);
}

TEST(VisitorTest, Pattern_TypeExtractor_Assignments)
{
  std::string program = R"(
    program Test;
    var x, y, z : Int;
    begin
      x := 1;
      y := 2;
      z := 3
    end.
  )";
  Parser parser(std::move(program));
  EXPECT_NO_THROW(parser.parse());

  TypeExtractor<AssignmentStatement> extractor;
  parser.m_current_block->body->accept(extractor, *parser.m_current_block);

  ASSERT_EQ(extractor.nodes.size(), 3);
  EXPECT_TRUE(checkVariableAccess(extractor.nodes[0]->var(), "x"));
  EXPECT_TRUE(checkVariableAccess(extractor.nodes[1]->var(), "y"));
  EXPECT_TRUE(checkVariableAccess(extractor.nodes[2]->var(), "z"));
}
