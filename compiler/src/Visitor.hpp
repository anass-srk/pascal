#pragma once
#include "Semantics.hpp"

namespace pascal_compiler {

using Context = Block;

struct LiteralExpression;
struct UnaryExpression;
struct BinaryExpression;
struct NExpression;
struct VariableAccess;
struct FunctionCall;

// Expression visitor - visits all Expression subclasses
class ExpressionVisitor {
public:
  virtual void visit(const LiteralExpression&, const Context& ctx) {}
  virtual void visit(const UnaryExpression&, const Context& ctx) {}
  virtual void visit(const BinaryExpression&, const Context& ctx) {}
  virtual void visit(const NExpression&, const Context& ctx) {}
  virtual void visit(const VariableAccess&, const Context& ctx) {}
  virtual void visit(const FunctionCall&, const Context& ctx) {}
  virtual ~ExpressionVisitor() = default;
};

struct LabeledStatement;
struct AssignmentStatement;
struct ProcedureCall;
struct ReadStatement;
struct WriteStatement;
struct GotoStatement;
struct CompoundStatement;
struct WhileStatement;
struct RepeatStatement;
struct ForStatement;
struct IfStatement;
struct CaseStatement;

// Statement visitor - visits all Statement subclasses
class StatementVisitor {
public:
  virtual void visit(const LabeledStatement&, const Context& ctx) {}
  virtual void visit(const AssignmentStatement&, const Context& ctx) {}
  virtual void visit(const ProcedureCall&, const Context& ctx) {}
  virtual void visit(const ReadStatement&, const Context& ctx) {}
  virtual void visit(const WriteStatement&, const Context& ctx) {}
  virtual void visit(const GotoStatement&, const Context& ctx) {}
  virtual void visit(const CompoundStatement&, const Context& ctx) {}
  virtual void visit(const WhileStatement&, const Context& ctx) {}
  virtual void visit(const RepeatStatement&, const Context& ctx) {}
  virtual void visit(const ForStatement&, const Context& ctx) {}
  virtual void visit(const IfStatement&, const Context& ctx) {}
  virtual void visit(const CaseStatement&, const Context& ctx) {}
  virtual ~StatementVisitor() = default;
};

struct ArraySelector;
struct FieldSelector;

// SelectorVisitor - visits Selector types
class SelectorVisitor {
public:
  virtual void visit(const ArraySelector&, const Context& ctx) {}
  virtual void visit(const FieldSelector&, const Context& ctx) {}
  virtual ~SelectorVisitor() = default;
};

// CombinedVisitor - convenience base inheriting all visitor interfaces
class CombinedVisitor 
  : public ExpressionVisitor,
    public StatementVisitor,
    public SelectorVisitor {
public:
  using ExpressionVisitor::visit;
  using SelectorVisitor::visit;
  using StatementVisitor::visit;
  virtual ~CombinedVisitor() = default;
};

} // namespace pascal_compiler
