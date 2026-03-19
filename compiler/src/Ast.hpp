#pragma once
#include "Semantics.hpp"
#include "Visitor.hpp"

#define DEFINE_EXPR_ACCEPT()                                                   \
  void accept(ExpressionVisitor &visitor, const Block &ctx) const override {   \
    visitor.visit(*this, ctx);                                                 \
  }

#define DEFINE_STMT_ACCEPT()                                                   \
  void accept(StatementVisitor &visitor, const Block &ctx) const override {    \
    visitor.visit(*this, ctx);                                                 \
  }

#define DEFINE_SELECTOR_ACCEPT()                                               \
  void accept(SelectorVisitor &visitor, const Block &ctx) const override {     \
    visitor.visit(*this, ctx);                                                 \
  }

namespace pascal_compiler {

struct AstNode {
protected:
  Lexeme m_token;

public:
  virtual ~AstNode() = default;
  const Lexeme &token() const { return m_token; }
};

// Expressions
struct Expression : public AstNode {
protected:
  const Type *m_expr_type = nullptr;

public:
  Expression(Lexeme token = Lexeme{}) { m_token = token; }
  const Type *type() const { return m_expr_type; }

  virtual void validate() = 0;
  virtual void accept(ExpressionVisitor &, const Block &) const = 0;
};

struct LiteralExpression : public Expression {
private:
  std::unique_ptr<Const> m_constant;

public:
  LiteralExpression(std::unique_ptr<Const> c, Lexeme token)
      : Expression(token), m_constant(std::move(c)) {}
  void validate() override;

  const Const *constant() const { return m_constant.get(); }

  DEFINE_EXPR_ACCEPT()
};

// Unary operations (+, -, not)
enum struct UnaryOp { Plus, Minus, Not };

struct UnaryExpression : public Expression {
private:
  UnaryOp m_op;
  std::unique_ptr<Expression> m_operand;

public:
  UnaryExpression(UnaryOp o, std::unique_ptr<Expression> e, Lexeme token)
      : Expression(token), m_op(o), m_operand(std::move(e)) {}
  void validate() override;

  UnaryOp op() const { return m_op; }
  const Expression *operand() const { return m_operand.get(); }

  DEFINE_EXPR_ACCEPT()
};

// Binary operations
enum struct ALOp {
  Add,
  Sub,
  Or,
  Mul,
  Div,
  And
};

enum struct RelOp {
  Eq,
  Ne,
  Lt,
  Le,
  Gt,
  Ge
};

// Only for relational operations
struct BinaryExpression : public Expression {
private:
  RelOp m_op;
  std::unique_ptr<Expression> m_left;
  std::unique_ptr<Expression> m_right;

public:
  BinaryExpression(RelOp o, std::unique_ptr<Expression> l,
                   std::unique_ptr<Expression> r, Lexeme token,
                   const Type *bool_type)
      : Expression(token), m_op(o), m_left(std::move(l)),
        m_right(std::move(r)) {
    m_expr_type = bool_type;
  }
  void validate() override;

  RelOp op() const { return m_op; }
  const Expression *left() const { return m_left.get(); }
  const Expression *right() const { return m_right.get(); }

  DEFINE_EXPR_ACCEPT()
};

struct NExpression : public Expression {
private:
  std::vector<ALOp> m_ops;
  std::vector<std::unique_ptr<Expression>> m_exprs;

public:
  NExpression(std::unique_ptr<Expression> first, Lexeme token)
      : Expression(token) {
    m_exprs.push_back(std::move(first));
  }

  void add(ALOp op, std::unique_ptr<Expression> exp) {
    m_ops.push_back(op);
    m_exprs.push_back(std::move(exp));
  }

  void validate() override;

  const std::vector<ALOp> &ops() const { return m_ops; }
  const std::vector<std::unique_ptr<Expression>> &exprs() const {
    return m_exprs;
  }

  DEFINE_EXPR_ACCEPT()
};

// Selectors for array and record accesses
struct Selector : public AstNode {
  virtual ~Selector() = default;
  virtual const Type *apply(const Type *) = 0;

  virtual void accept(SelectorVisitor &, const Block &) const = 0;
};

struct ArraySelector : public Selector {
private:
  std::vector<std::unique_ptr<Expression>> m_indices;

public:
  ArraySelector(std::vector<std::unique_ptr<Expression>> &&idx, Lexeme token)
      : m_indices(std::move(idx)) {
    m_token = token;
  }
  const Type *apply(const Type *) override;

  const std::vector<std::unique_ptr<Expression>> &indices() const {
    return m_indices;
  }

  DEFINE_SELECTOR_ACCEPT()
};

struct FieldSelector : public Selector {
private:
  std::string_view m_field;

public:
  FieldSelector(std::string_view f, Lexeme token) : m_field(f) {
    m_token = token;
  }
  const Type *apply(const Type *) override;

  std::string_view field() const { return m_field; }

  DEFINE_SELECTOR_ACCEPT()
};

// Variable access (l‑value)
struct VariableAccess : public Expression {
private:
  const Var *m_base_var = nullptr;
  std::vector<std::unique_ptr<Selector>> m_selectors;

public:
  VariableAccess(const Var *v, std::vector<std::unique_ptr<Selector>> &&sels,
                 Lexeme token)
      : Expression(token), m_selectors(std::move(sels)), m_base_var(v) {}
  void validate() override;

  const Var *base_var() const { return m_base_var; }
  const std::vector<std::unique_ptr<Selector>> &selectors() const {
    return m_selectors;
  }

  DEFINE_EXPR_ACCEPT()
};

// Function call (returns a value)
struct FunctionCall : public Expression {
private:
  const Function *m_function = nullptr;
  std::vector<std::unique_ptr<Expression>> m_args;

public:
  FunctionCall(const Function *func,
               std::vector<std::unique_ptr<Expression>> &&arguments,
               Lexeme token)
      : Expression(token), m_args(std::move(arguments)), m_function(func) {}
  void validate() override;

  const Function *function() const { return m_function; }
  const std::vector<std::unique_ptr<Expression>> &args() const {
    return m_args;
  }

  DEFINE_EXPR_ACCEPT()
};

// Statements

struct Statement : public AstNode {
  Statement(Lexeme token = Lexeme{}) { m_token = token; }
  virtual void validate() = 0;
  virtual void accept(StatementVisitor &, const Block &) const = 0;
};

struct LabeledStatement : public Statement {
private:
  const Label *m_label;
  std::unique_ptr<Statement> m_stmt;

public:
  LabeledStatement(const Label *lbl, std::unique_ptr<Statement> s, Lexeme token)
      : Statement(token), m_label(lbl), m_stmt(std::move(s)) {}
  void validate() override;

  const Label *label() const { return m_label; }
  const Statement *statement() const { return m_stmt.get(); }

  DEFINE_STMT_ACCEPT()
};

struct AssignmentStatement : public Statement {
private:
  std::unique_ptr<VariableAccess> m_lhs;
  std::unique_ptr<Expression> m_rhs;

public:
  AssignmentStatement(std::unique_ptr<VariableAccess> l,
                      std::unique_ptr<Expression> r, Lexeme token)
      : Statement(token), m_lhs(std::move(l)), m_rhs(std::move(r)) {}
  void validate() override;

  const VariableAccess *var() const { return m_lhs.get(); }
  const Expression *expr() const { return m_rhs.get(); }

  DEFINE_STMT_ACCEPT()
};

struct ProcedureCall : public Statement {
private:
  const Function *m_procedure = nullptr;
  std::vector<std::unique_ptr<Expression>> m_args;

public:
  ProcedureCall(const Function *func,
                std::vector<std::unique_ptr<Expression>> &&arguments,
                Lexeme token)
      : Statement(token), m_args(std::move(arguments)), m_procedure(func) {}
  void validate() override;

  const Function *procedure() const { return m_procedure; }
  const std::vector<std::unique_ptr<Expression>> &args() const {
    return m_args;
  }

  DEFINE_STMT_ACCEPT()
};

// Read statement (built‑in)
struct ReadStatement : public Statement {
private:
  std::vector<std::unique_ptr<VariableAccess>> m_arguments;

public:
  ReadStatement(Lexeme token,
                std::vector<std::unique_ptr<VariableAccess>> &&args)
      : Statement(token), m_arguments(std::move(args)) {}
  void validate() override;

  const std::vector<std::unique_ptr<VariableAccess>> &args() const {
    return m_arguments;
  }

  DEFINE_STMT_ACCEPT()
};

// Write statement (built‑in)
struct WriteStatement : public Statement {
private:
  std::vector<std::unique_ptr<Expression>> m_arguments;

public:
  WriteStatement(Lexeme token, std::vector<std::unique_ptr<Expression>> &&args)
      : Statement(token), m_arguments(std::move(args)) {}
  void validate() override;

  const std::vector<std::unique_ptr<Expression>> &args() const {
    return m_arguments;
  }

  DEFINE_STMT_ACCEPT()
};

struct GotoStatement : public Statement {
private:
  const Label *m_label;

public:
  GotoStatement(const Label *lbl, Lexeme token)
      : Statement(token), m_label(lbl) {}
  void validate() override {}

  const Label *label() const { return m_label; }

  DEFINE_STMT_ACCEPT()
};

// Compound statement (BEGIN ... END)
struct CompoundStatement : public Statement {
private:
  std::vector<std::unique_ptr<Statement>> m_statements;

public:
  CompoundStatement(std::vector<std::unique_ptr<Statement>> &&stmts,
                    Lexeme token)
      : Statement(token), m_statements(std::move(stmts)) {}
  void validate() override;

  const std::vector<std::unique_ptr<Statement>> &statements() const {
    return m_statements;
  }

  DEFINE_STMT_ACCEPT()
};

struct WhileStatement : public Statement {
private:
  std::unique_ptr<Expression> m_condition;
  std::unique_ptr<Statement> m_body;

public:
  WhileStatement(std::unique_ptr<Expression> cond,
                 std::unique_ptr<Statement> content, Lexeme token)
      : Statement(token), m_condition(std::move(cond)),
        m_body(std::move(content)) {}
  void validate() override;

  const Expression *condition() const { return m_condition.get(); }
  const Statement *body() const { return m_body.get(); }

  DEFINE_STMT_ACCEPT()
};

struct RepeatStatement : public Statement {
private:
  std::vector<std::unique_ptr<Statement>> m_body;
  std::unique_ptr<Expression> m_until_expr;

public:
  RepeatStatement(std::vector<std::unique_ptr<Statement>> &&content,
                  std::unique_ptr<Expression> cond, Lexeme token)
      : Statement(token), m_body(std::move(content)),
        m_until_expr(std::move(cond)) {}
  void validate() override;

  const std::vector<std::unique_ptr<Statement>> &body() const { return m_body; }
  const Expression *condition() const { return m_until_expr.get(); }

  DEFINE_STMT_ACCEPT()
};

// For
struct ForStatement : public Statement {
private:
  std::unique_ptr<VariableAccess> m_loop_var;
  Const m_start, m_end;
  bool m_increasing; // to -> true, downto -> false
  std::unique_ptr<Statement> m_body;

public:
  ForStatement(std::unique_ptr<VariableAccess> var, Const s, Const e,
               std::unique_ptr<Statement> content, bool to, Lexeme token)
      : Statement(token), m_loop_var(std::move(var)), m_start(std::move(s)),
        m_end(std::move(e)), m_body(std::move(content)), m_increasing(to) {}
  void validate() override;

  const VariableAccess *var() const { return m_loop_var.get(); }
  const Const &start() const { return m_start; }
  const Const &end() const { return m_end; }
  bool increasing() const { return m_increasing; }
  const Statement *body() const { return m_body.get(); }

  DEFINE_STMT_ACCEPT()
};

struct IfStatement : public Statement {
private:
  std::unique_ptr<Expression> m_condition;
  std::unique_ptr<Statement> m_then_part;
  std::unique_ptr<Statement> m_else_part; // nullptr if no else

public:
  IfStatement(std::unique_ptr<Expression> cond,
              std::unique_ptr<Statement> thenStmt,
              std::unique_ptr<Statement> elseStmt, Lexeme token)
      : Statement(token), m_condition(std::move(cond)),
        m_then_part(std::move(thenStmt)), m_else_part(std::move(elseStmt)) {}
  void validate() override;

  const Expression *condition() const { return m_condition.get(); }
  const Statement *then_stmt() const { return m_then_part.get(); }
  const Statement *else_stmt() const { return m_else_part.get(); }

  DEFINE_STMT_ACCEPT()
};

struct CaseStatement : public Statement {
  struct CaseAlternative {
  private:
    std::vector<Const> m_labels;
    std::unique_ptr<Statement> m_statement;
    Lexeme m_token;

  public:
    CaseAlternative(std::vector<Const> &&lbls, std::unique_ptr<Statement> stmt,
                    Lexeme token)
        : m_labels(std::move(lbls)), m_statement(std::move(stmt)) {
      m_token = token;
    }

    const std::vector<Const> &labels() const { return m_labels; }
    const Statement *statement() const { return m_statement.get(); }
    const Lexeme &token() const { return m_token; }
  };

private:
  std::unique_ptr<Expression> m_selector;
  std::vector<CaseAlternative> m_alternatives;

public:
  CaseStatement(std::unique_ptr<Expression> sel,
                std::vector<CaseAlternative> &&cases, Lexeme token)
      : Statement(token), m_selector(std::move(sel)),
        m_alternatives(std::move(cases)) {}
  void validate() override;

  const Expression *selector() const { return m_selector.get(); }
  const std::vector<CaseAlternative> &alternatives() const {
    return m_alternatives;
  }

  DEFINE_STMT_ACCEPT()
};

// Program root
struct Program {
private:
  std::string_view m_name;
  std::unique_ptr<Block> m_block;
  std::unique_ptr<CompoundStatement> m_main_body;

public:
  Program(std::string_view id, std::unique_ptr<Block> defs,
          std::unique_ptr<CompoundStatement> body)
      : m_name(id), m_block(std::move(defs)), m_main_body(std::move(body)) {}

  std::string_view name() const { return m_name; }
  const Block *block() const { return m_block.get(); }
  const CompoundStatement *main_body() const { return m_main_body.get(); }
};

} 