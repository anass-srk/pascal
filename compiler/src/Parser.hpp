#pragma once
#include "Semantics.hpp"

namespace pascal_compiler
{

class SyntaxException
{
  const std::string m_msg; //Remember lifetimes
  const Lexeme m_token;

public:

  SyntaxException(std::string&& msg, const Lexeme& token) : m_msg(std::move(msg)), m_token(token){}

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
public:
  Block *m_current_block;

public:
  Parser(std::string&&);

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
    return m_lexer.getToken().m_type == type;
  }

  void parse();
  void program();
  void block();
  void declaration();
  void statement();

  // Declaration types
  void label_declaration();
  void const_definition();
  void type_definition();
  void variable_definition();
  void procedure_definition();
  void function_definition();

  //
  Const constant(const Lexeme &); // token containing id of the constant
  std::unique_ptr<Type> type_eval(const Lexeme&);
  std::unique_ptr<Enum> enum_type(const Lexeme&);
  std::vector<Lexeme> id_list();
};

}