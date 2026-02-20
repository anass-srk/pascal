#pragma once
#include "Semantics.hpp"

namespace pascal_compiler
{

struct SourceLocation
{
  size_t line, col;
};

class AstNode
{
public:
  SourceLocation loc;
  AstNode(SourceLocation loc = SourceLocation{.line=0, .col=0}) : loc(loc) {};
  virtual ~AstNode() = default;
};

class LiteralExpression : public AstNode
{
public:
  Const value;
  LiteralExpression(Const c, SourceLocation loc) : AstNode(loc), value(std::move(c)) {}
};

class IdentifierExpression : public AstNode
{
public:
  std::string_view name;

  // one is non-null
  const Const *constant = nullptr;
  const EnumValue *enumValue = nullptr;
  const Var *variable = nullptr;
  const Function *function = nullptr;

  IdentifierExpression(std::string_view n, SourceLocation loc) : AstNode(loc), name(n) {}
};

enum class UnaryOp { Plus, Minus, Not};
enum class BinaryOp {
  Eq, Ne, Lt, Le, Gt, Ge,   // relational
  Add, Sub, Or,              // addition
  Mul, Div, DivInt, And       // multiplication
};

class UnaryExpression : public AstNode
{
public:
  UnaryOp op;
  std::unique_ptr<AstNode> operand;
  const Type *resultType = nullptr; // set during type checking
  UnaryExpression(UnaryOp o, std::unique_ptr<AstNode> e, SourceLocation loc) 
    : AstNode(loc), op(o), operand(std::move(e)) {}
};

class BinaryExpression : public AstNode
{
public:
  BinaryOp op;
  std::unique_ptr<AstNode> left, right;
  const Type *resultType = nullptr;
  BinaryExpression(BinaryOp o, std::unique_ptr<AstNode> l, std::unique_ptr<AstNode> r, SourceLocation loc)
    : AstNode(loc), op(o), left(std::move(l)), right(std::move(r)) {}
};

class Selector : public AstNode
{
public:
  virtual ~Selector() = default;
};

class ArraySelector : public Selector
{
public:
  std::unique_ptr<AstNode> index;
  ArraySelector(std::unique_ptr<AstNode> idx, SourceLocation loc)
      : index(std::move(idx)) { this->loc = loc; }
};

class FieldSelector : public Selector
{
public:
  std::string_view field;
  FieldSelector(std::string_view f, SourceLocation loc) : field(f) { this->loc = loc; }
};

class VariableAccess : public AstNode
{
public:
  const Var *baseVar = nullptr;
  std::vector<std::unique_ptr<Selector>> selectors;
  // The type after applying all selectors
  const Type *accessType = nullptr;

  VariableAccess(SourceLocation loc) : AstNode(loc) {}
};

class FunctionCall : public AstNode
{
public:
  const Function *function = nullptr;
  std::vector<std::unique_ptr<AstNode>> arguments;
  const Type *resultType = nullptr;

  FunctionCall(SourceLocation loc) : AstNode(loc) {}
};

class Statement : public AstNode {};

class LabeledStatement : public Statement
{
public:
  Label label;
  std::unique_ptr<Statement> stmt;
  LabeledStatement(Label lbl, std::unique_ptr<Statement> s, SourceLocation loc)
    : label(lbl), stmt(std::move(s)) { this->loc = loc; }
};

class AssignmentStatement : public Statement
{
public:
  std::unique_ptr<VariableAccess> lhs;
  std::unique_ptr<AstNode> rhs; // expression
  AssignmentStatement(std::unique_ptr<VariableAccess> l, std::unique_ptr<AstNode> r, SourceLocation loc)
      : lhs(std::move(l)), rhs(std::move(r)) { this->loc = loc; }
};

class ProcedureCallStatement : public Statement
{
public:
  std::string procName;
  const Function *procedure = nullptr; // (null for 'write'/'read')
  std::vector<std::unique_ptr<AstNode>> arguments;
  ProcedureCallStatement(std::string name, SourceLocation loc) : procName(std::move(name)) {this->loc = loc;}
};

class ReadStatement : public Statement
{
public:
  std::vector<std::unique_ptr<VariableAccess>> arguments;
  ReadStatement(SourceLocation loc) {this->loc = loc;}
};

class GotoStatement : public Statement
{
public:
  const LabeledStatement *stmt = nullptr;
  GotoStatement(const LabeledStatement* lbl, SourceLocation loc) : stmt(lbl) {this->loc = loc;}
};

};