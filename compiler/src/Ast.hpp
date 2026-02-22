#pragma once
#include "Semantics.hpp"

namespace pascal_compiler
{
struct SourceLocation 
{
  size_t line = 0;
  size_t column = 0;
  SourceLocation(size_t l = 0, size_t c = 0) : line(l), column(c) {}
  SourceLocation(const Lexeme& token) 
  {
    line = token.m_line;
    column = token.m_col;
  }
};

struct AstNode 
{
  SourceLocation loc;
  virtual ~AstNode() = default;
};

// Expressions
struct Expression : public AstNode 
{
  Expression(SourceLocation loc = SourceLocation{}) { this->loc = loc; }
  const Type* exprType = nullptr;
};

struct LiteralExpression : public Expression 
{
  std::unique_ptr<Const> value;
  LiteralExpression(std::unique_ptr<Const> c, SourceLocation loc) : Expression(loc), value(std::move(c)) {}
};

// Identifier reference – can be a variable, constant, enum value, or function name.
struct IdentifierExpression : public Expression 
{
  std::string_view name;
  std::variant<const Var*, const Const*, const EnumValue*, const Function*> symbol;

  template<typename T>
  IdentifierExpression(std::string_view id, T* sym, SourceLocation loc)
    : Expression(loc), name(id), symbol(sym) {}

  enum struct Kind { Var, Const, Enum, Func };
  Kind getKind() const {
    if (std::holds_alternative<const Var*>(symbol)) return Kind::Var;
    if (std::holds_alternative<const Const*>(symbol)) return Kind::Const;
    if (std::holds_alternative<const EnumValue*>(symbol)) return Kind::Enum;
    return Kind::Func;
  }
};

// Unary operations (+, -, not)
enum struct UnaryOp { Plus, Minus, Not };

struct UnaryExpression : public Expression 
{
  UnaryOp op;
  std::unique_ptr<Expression> operand;

  UnaryExpression(UnaryOp o, std::unique_ptr<Expression> e, SourceLocation loc)
    : Expression(loc), op(o), operand(std::move(e)) {}
  void validate(const Lexeme&);
};

// Binary operations
enum struct BinaryOp 
{
  Eq, Ne, Lt, Le, Gt, Ge, // relational
  Add, Sub, Or, // addition
  Mul, Div, And // multiplication
};

struct BinaryExpression : public Expression 
{
  BinaryOp op;
  std::unique_ptr<Expression> left, right;

  BinaryExpression(BinaryOp o, std::unique_ptr<Expression> l, std::unique_ptr<Expression> r, SourceLocation loc)
    : Expression(loc), op(o), left(std::move(l)), right(std::move(r)) {}
  void validate(const Lexeme&);
};

// Selectors for array and record accesses
struct Selector : public AstNode 
{
  virtual ~Selector() = default;
};

struct ArraySelector : public Selector 
{
  std::unique_ptr<Expression> index;
  ArraySelector(std::unique_ptr<Expression> idx, SourceLocation loc)
    : index(std::move(idx)) { this->loc = loc; }
};

struct FieldSelector : public Selector 
{
  std::string_view field;
  FieldSelector(std::string_view f, SourceLocation loc) : field(f) { this->loc = loc; }
};

// Variable access (l‑value)
struct VariableAccess : public Expression 
{
  const Var* baseVar = nullptr;               // the base variable (must be a Var)
  std::vector<std::unique_ptr<Selector>> selectors;
  VariableAccess(SourceLocation loc) : Expression(loc) {}
};

// Function call (returns a value)
struct FunctionCall : public Expression 
{
  const Function* function = nullptr;         // resolved function
  std::vector<std::unique_ptr<Expression>> arguments;
  FunctionCall(SourceLocation loc) : Expression(loc) {}
};



// Statements

struct Statement : public AstNode 
{
  Statement(SourceLocation loc = SourceLocation{}) { this->loc = loc; }
};

struct LabeledStatement : public Statement 
{
  std::string_view label;
  std::unique_ptr<Statement> stmt;
  LabeledStatement(std::string_view lbl, std::unique_ptr<Statement> s, SourceLocation loc)
    : Statement(loc), label(lbl), stmt(std::move(s)) {}
};

struct AssignmentStatement : public Statement 
{
  std::unique_ptr<VariableAccess> lhs;
  std::unique_ptr<Expression> rhs;
  AssignmentStatement(std::unique_ptr<VariableAccess> l, std::unique_ptr<Expression> r, SourceLocation loc)
    : Statement(loc), lhs(std::move(l)), rhs(std::move(r)) {}
};

struct ProcedureCallStatement : public Statement 
{
  std::string_view name;
  const Function* procedure = nullptr;   // null for write
  std::vector<std::unique_ptr<Expression>> arguments;
  ProcedureCallStatement(std::string_view id, SourceLocation loc)
    : Statement(loc), name(id) {}
};

// Read statement (built‑in)
struct ReadStatement : public Statement 
{
  std::vector<std::unique_ptr<VariableAccess>> arguments;
  ReadStatement(SourceLocation loc) : Statement(loc){}
};

// Write statement (built‑in)
struct WriteStatement : public Statement
{
  std::vector<std::unique_ptr<Expression>> arguments;
  WriteStatement(SourceLocation loc) : Statement(loc) {}
};

struct GotoStatement : public Statement 
{
  std::string_view label;
  GotoStatement(std::string_view lbl, SourceLocation loc)
    : Statement(loc), label(lbl) {}
};

// Compound statement (BEGIN ... END)
struct CompoundStatement : public Statement 
{
  std::vector<std::unique_ptr<Statement>> statements;
  CompoundStatement(SourceLocation loc) : Statement(loc) {}
};

struct WhileStatement : public Statement 
{
  std::unique_ptr<Expression> condition;
  std::unique_ptr<Statement> body;
  WhileStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> content, SourceLocation loc)
    : Statement(loc), condition(std::move(cond)), body(std::move(content)) {}
};

struct RepeatStatement : public Statement 
{
  std::vector<std::unique_ptr<Statement>> body;
  std::unique_ptr<Expression> untilExpr;
  RepeatStatement(SourceLocation loc) : Statement(loc){}
};

// For
struct ForStatement : public Statement 
{
  std::unique_ptr<VariableAccess> loopVar;
  std::unique_ptr<Expression> start, end;
  bool increasing;   // to -> true, downto -> false
  std::unique_ptr<Statement> body;

  ForStatement(
    std::unique_ptr<VariableAccess> var,
    std::unique_ptr<Expression> s,
    std::unique_ptr<Expression> e,
    std::unique_ptr<Statement> content,
    bool to,
    SourceLocation loc
  ) : Statement(loc), loopVar(std::move(var)), start(std::move(s)),
      end(std::move(e)),body(std::move(content)), increasing(to) {}
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
    SourceLocation loc
  ) : Statement(loc), condition(std::move(cond)),
      thenPart(std::move(thenStmt)), elsePart(std::move(elseStmt)) {}
};

struct CaseStatement : public Statement 
{
  struct CaseAlternative {
    std::vector<Const> labels;
    std::unique_ptr<Statement> stmt;
    SourceLocation loc;
  };

  std::unique_ptr<Expression> selector;
  std::vector<CaseAlternative> alternatives;

  CaseStatement(std::unique_ptr<Expression> sel, SourceLocation loc)
    : Statement(loc), selector(std::move(sel)) {}
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