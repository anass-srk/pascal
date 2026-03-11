#pragma once
#include "Semantics.hpp"
#include "Visitor.hpp"

namespace pascal_compiler
{

struct AstNode
{
  Lexeme token;
  virtual ~AstNode() = default;
};

// Expressions
struct Expression : public AstNode
{
  Expression(Lexeme token = Lexeme{}) { this->token = token; }
  const Type* exprType = nullptr;
  virtual void validate() = 0;
  virtual void accept(ExpressionVisitor&, const Block&) const = 0;
};

struct LiteralExpression : public Expression
{
  std::unique_ptr<Const> value;
  LiteralExpression(std::unique_ptr<Const> c, Lexeme token) : Expression(token), value(std::move(c)) {}
  void validate() override;

  void accept(ExpressionVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

// Unary operations (+, -, not)
enum struct UnaryOp { Plus, Minus, Not };

struct UnaryExpression : public Expression
{
  UnaryOp op;
  std::unique_ptr<Expression> operand;

  UnaryExpression(UnaryOp o, std::unique_ptr<Expression> e, Lexeme token)
    : Expression(token), op(o), operand(std::move(e)) {}
  void validate() override;

  void accept(ExpressionVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

// Binary operations
enum struct BinaryOp 
{
  Eq, Ne, Lt, Le, Gt, Ge, // relational
  Add, Sub, Or, // addition
  Mul, Div, And // multiplication
};

static const char* BINARY_OP_NAME[] =
{
  "=", "<>", "<", "<=", ">", ">=", // relational
  "+", "-", "or", // addition
  "*", "(/ or div)", "and" // multiplication
};
// Only for relational operations
struct BinaryExpression : public Expression
{
  BinaryOp op;
  std::unique_ptr<Expression> left, right;

  BinaryExpression(BinaryOp o, std::unique_ptr<Expression> l, std::unique_ptr<Expression> r, Lexeme token, const Type* bool_type)
    : Expression(token), op(o), left(std::move(l)), right(std::move(r)) {this->exprType = bool_type;}
  void validate() override;

  void accept(ExpressionVisitor &visitor, const Block &ctx) const override{
    visitor.visit(*this, ctx);
  }
};

struct NExpression : public Expression
{
  std::vector<BinaryOp> ops;
  std::vector<std::unique_ptr<Expression>> exprs;

  NExpression(std::unique_ptr<Expression> first, Lexeme token) : Expression(token)
  {exprs.push_back(std::move(first));}

  void add(BinaryOp op, std::unique_ptr<Expression> exp)
  {
    ops.push_back(op);
    exprs.push_back(std::move(exp));
  }

  void validate() override;

  void accept(ExpressionVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

// Selectors for array and record accesses
struct Selector : public AstNode
{
  virtual ~Selector() = default;
  virtual const Type* apply(const Type*) = 0;

  virtual void accept(SelectorVisitor&, const Block&) const = 0;
};

struct ArraySelector : public Selector
{
  std::vector<std::unique_ptr<Expression>> indices;
  ArraySelector(std::vector<std::unique_ptr<Expression>>&& idx, Lexeme token)
  : indices(std::move(idx)){ this->token = token; }
  const Type* apply(const Type*) override;
  
  void accept(SelectorVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

struct FieldSelector : public Selector
{
  std::string_view field;
  FieldSelector(std::string_view f, Lexeme token) : field(f) { this->token = token; }
  const Type* apply(const Type*) override;

  void accept(SelectorVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

// Variable access (l‑value)
struct VariableAccess : public Expression
{
  const Var* baseVar = nullptr;               // the base variable (must be a Var)
  std::vector<std::unique_ptr<Selector>> selectors;
  VariableAccess(const Var *v, std::vector<std::unique_ptr<Selector>>&& sels, Lexeme token)
  : Expression(token), selectors(std::move(sels)),  baseVar(v) {}
  void validate() override;

  void accept(ExpressionVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

// Function call (returns a value)
struct FunctionCall : public Expression
{
  const Function* function = nullptr;         // resolved function
  std::vector<std::unique_ptr<Expression>> args;
  FunctionCall(const Function *func, std::vector<std::unique_ptr<Expression>> &&arguments, Lexeme token)
      : Expression(token), args(std::move(arguments)), function(func) {}
  void validate() override;

  void accept(ExpressionVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};



// Statements

struct Statement : public AstNode
{
  Statement(Lexeme token = Lexeme{}) { this->token = token; }
  virtual void validate() = 0;
  virtual void accept(StatementVisitor&, const Block&) const = 0;
};

struct LabeledStatement : public Statement
{
  std::string_view label;
  std::unique_ptr<Statement> stmt;
  LabeledStatement(std::string_view lbl, std::unique_ptr<Statement> s, Lexeme token)
    : Statement(token), label(lbl), stmt(std::move(s)) {}
  void validate() override;

  void accept(StatementVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

struct AssignmentStatement : public Statement
{
  std::unique_ptr<VariableAccess> lhs;
  std::unique_ptr<Expression> rhs;
  AssignmentStatement(std::unique_ptr<VariableAccess> l, std::unique_ptr<Expression> r, Lexeme token)
    : Statement(token), lhs(std::move(l)), rhs(std::move(r)) {}
  void validate() override;

  void accept(StatementVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

struct ProcedureCall : public Statement
{
  const Function* procedure = nullptr;
  std::vector<std::unique_ptr<Expression>> args;
  ProcedureCall(const Function *func, std::vector<std::unique_ptr<Expression>> &&arguments, Lexeme token)
      : Statement(token), args(std::move(arguments)), procedure(func) {}
  void validate() override;

  void accept(StatementVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

// Read statement (built‑in)
struct ReadStatement : public Statement
{
  std::vector<std::unique_ptr<VariableAccess>> arguments;
  ReadStatement(Lexeme token, std::vector<std::unique_ptr<VariableAccess>>&& args)
    : Statement(token), arguments(std::move(args)) {}
  void validate() override;

  void accept(StatementVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

// Write statement (built‑in)
struct WriteStatement : public Statement
{
  std::vector<std::unique_ptr<Expression>> arguments;
  WriteStatement(Lexeme token, std::vector<std::unique_ptr<Expression>>&& args)
    : Statement(token), arguments(std::move(args)) {}
  void validate() override;

  void accept(StatementVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

struct GotoStatement : public Statement
{
  std::string_view label;
  GotoStatement(std::string_view lbl, Lexeme token)
    : Statement(token), label(lbl) {}
  void validate() override {}

  void accept(StatementVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

// Compound statement (BEGIN ... END)
struct CompoundStatement : public Statement
{
  std::vector<std::unique_ptr<Statement>> statements;
  CompoundStatement(std::vector<std::unique_ptr<Statement>>&& stmts, Lexeme token) : Statement(token), statements(std::move(stmts)) {}
  void validate() override;

  void accept(StatementVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

struct WhileStatement : public Statement
{
  std::unique_ptr<Expression> condition;
  std::unique_ptr<Statement> body;
  WhileStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> content, Lexeme token)
    : Statement(token), condition(std::move(cond)), body(std::move(content)) {}
  void validate() override;

  void accept(StatementVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

struct RepeatStatement : public Statement
{
  std::vector<std::unique_ptr<Statement>> body;
  std::unique_ptr<Expression> untilExpr;
  RepeatStatement(std::vector<std::unique_ptr<Statement>> &&content, std::unique_ptr<Expression> cond, Lexeme token)
    : Statement(token), body(std::move(content)), untilExpr(std::move(cond)) {}
  void validate() override;

  void accept(StatementVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

// For
struct ForStatement : public Statement
{
  std::unique_ptr<VariableAccess> loopVar;
  Const start, end;
  bool increasing;   // to -> true, downto -> false
  std::unique_ptr<Statement> body;

  ForStatement(
    std::unique_ptr<VariableAccess> var,
    Const s,
    Const e,
    std::unique_ptr<Statement> content,
    bool to,
    Lexeme token
  ) : Statement(token), loopVar(std::move(var)), start(std::move(s)),
      end(std::move(e)),body(std::move(content)), increasing(to) {}
  void validate() override;

  void accept(StatementVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

struct IfStatement : public Statement
{
  std::unique_ptr<Expression> condition;
  std::unique_ptr<Statement> thenPart;
  std::unique_ptr<Statement> elsePart;   // nullptr if no else

  IfStatement(
    std::unique_ptr<Expression> cond,
    std::unique_ptr<Statement> thenStmt,
    std::unique_ptr<Statement> elseStmt,
    Lexeme token
  ) : Statement(token), condition(std::move(cond)),
      thenPart(std::move(thenStmt)), elsePart(std::move(elseStmt)) {}
  void validate() override;

  void accept(StatementVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

struct CaseStatement : public Statement
{
  struct CaseAlternative {
    std::vector<Const> labels;
    std::unique_ptr<Statement> statement;
    Lexeme token;
    CaseAlternative(std::vector<Const>&& lbls, std::unique_ptr<Statement> stmt, Lexeme token)
      : labels(std::move(lbls)), statement(std::move(stmt)) {this->token = token;}
  };

  std::unique_ptr<Expression> selector;
  std::vector<CaseAlternative> alternatives;

  CaseStatement(std::unique_ptr<Expression> sel, std::vector<CaseAlternative>&& cases, Lexeme token)
    : Statement(token), selector(std::move(sel)), alternatives(std::move(cases)) {}
  void validate() override;

  void accept(StatementVisitor& visitor, const Block& ctx) const override {
    visitor.visit(*this, ctx);
  }
};

// Program root
struct Program
{
  std::string_view name;
  std::unique_ptr<Block> block;               // the top‑level block (already contains symbol tables)
  std::unique_ptr<CompoundStatement> mainBody; // the main statement part

  Program(std::string_view id, std::unique_ptr<Block> defs, std::unique_ptr<CompoundStatement> body)
    : name(id), block(std::move(defs)), mainBody(std::move(body)) {}
};

}