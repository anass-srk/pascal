#pragma once
#include "Semantics.hpp"
#include "Ast.hpp"
#include <exception>

namespace pascal_compiler
{

class SyntaxException : public std::exception
{
  const std::string m_msg;
  const Lexeme m_token;

public:

  SyntaxException(std::string&& msg, const Lexeme& token) : m_msg(std::move(msg)), m_token(token){}

  const char* what() const noexcept override { return m_msg.c_str(); }

  const std::string& getMsg() const
  {
    return m_msg;
  }

  const Lexeme& getToken() const
  {
    return m_token;
  }

};

class Parser
{
  Lexer m_lexer;
  std::string m_program_name;
  std::unique_ptr<Block> m_block;
  Block *_top;
public:
  Block *m_current_block;

public:
  Parser(std::string&&);

  const Block * getTop() {return _top;};

  void match(TOKEN_TYPE);
  inline void match_adv(TOKEN_TYPE type)
  {
    m_lexer.next_sym();
    match(type);
  }

  void match(std::initializer_list<TOKEN_TYPE>);
  inline void match_adv(std::initializer_list<TOKEN_TYPE> l)
  {
    m_lexer.next_sym();
    return match(l);
  }

  inline void adv(){
    m_lexer.next_sym();
  }

  inline bool check(TOKEN_TYPE type){
    return m_lexer.getToken().type() == type;
  }

  inline bool check(std::initializer_list<TOKEN_TYPE> l)
  {
    for(auto t : l)
    {
      if(t == m_lexer.getToken().type()) return true;
    }
    return false;
  }

  void parse();
  void program();
  void block();
  void declaration();

  // Declaration types
  void label_declaration();
  void const_definition();
  void type_definition();
  void variable_declaration();
  void function_definition(bool is_proc);

  //
  Const constant(const Lexeme&); // token containing id of the constant
  const Type* get_type(const Lexeme&, bool); // Includes reading typenames + type_eval
  std::unique_ptr<Type> type_eval(const Lexeme&);
  std::unique_ptr<Enum> enum_type(const Lexeme&);
  std::vector<Lexeme> id_list();
  std::unique_ptr<Record> field_list(const Lexeme&);
  std::vector<Const> case_label_list();
  std::vector<Arg> args_list();

  const Type* find_type(bool required);

  std::unique_ptr<Expression> gexpression();
  std::unique_ptr<Expression> expression();
  std::unique_ptr<Expression> term();
  std::unique_ptr<Expression> factor();

  std::unique_ptr<VariableAccess> variable_access(const Var*);
  std::variant<std::unique_ptr<FunctionCall>, std::unique_ptr<ProcedureCall>> function_call(const Function*, bool is_procedure);

  std::unique_ptr<CompoundStatement> compound_stmt();
  std::unique_ptr<Statement> statement();
  
};

}