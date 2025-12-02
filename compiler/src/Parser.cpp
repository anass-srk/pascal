#include "Parser.hpp"
#include <format>

namespace pascal_compiler
{

Parser::Parser(std::string&& content) : m_lexer(std::move(content))
{

}

size_t Parser::match(TOKEN_TYPE type)
{
  const auto& token = m_lexer.getToken();
  if(token.m_type != type){
    throw SyntaxException(std::format("Syntax error: expected {}, found {} !", TOKEN_NAMES[int(type)], TOKEN_NAMES[int(token.m_type)]),token);
  }
  return token.m_index;
}

size_t Parser::match(std::initializer_list<TOKEN_TYPE> l){
  const auto current = m_lexer.getToken().m_type;
  for(auto type : l){
    if(type == current){
      return m_lexer.getToken().m_index;
    }
  }
  
  auto it = l.begin();
  std::string err = "Syntax error: expected (" + std::string(TOKEN_NAMES[int(*it)]);
  ++it;
  for(;it != l.end();++it){
    err += " or " + std::string(TOKEN_NAMES[int(*it)]);
  }
  err += "), found " + std::string(TOKEN_NAMES[int(current)]) + " !";
  throw SyntaxException(std::move(err), m_lexer.getToken());
}

void Parser::parse()
{
  program();
}

void Parser::init_block(Block& block){
  auto& types = block.m_types;
  for (auto type : {CONST_CAT::CC_INT, CONST_CAT::CC_REAL, CONST_CAT::CC_CHAR, CONST_CAT::CC_STRING})
  {
    auto name = CONST_CAT_NAMES[int(type)];
    types[name] = std::make_unique<Type>(name,0,0);
  }
}

void Parser::program()
{
  match_adv(TOKEN_TYPE::PROGRAM_TOKEN);
  match_adv(TOKEN_TYPE::ID_TOKEN);
  m_program_name = m_lexer.getToken().m_id;
  match_adv(TOKEN_TYPE::SEMI_TOKEN);

  m_block = std::make_unique<Block>();
  m_current_block = m_block.get();
  block();

  match_adv(TOKEN_TYPE::DOT_TOKEN);
}

void Parser::block()
{
  declaration();
  statement();
}

void Parser::declaration()
{

  adv();

  if(check(TOKEN_TYPE::LABEL_TOKEN))
  {
    label_declaration();
  }
  // else if(check(TOKEN_TYPE::CONST_TOKEN))
  // {
  //   const_definition();
  // }
  // else if(check(TOKEN_TYPE::TYPE_TOKEN))
  // {
  //   type_definition();
  // }
  // else if(check(TOKEN_TYPE::VAR_TOKEN))
  // {
  //   variable_definition();
  // }
  // else if(check(TOKEN_TYPE::PROCEDURE_TOKEN))
  // {
  //   procedure_definition();
  // }
  // else if(check(TOKEN_TYPE::FUNCTION_TOKEN))
  // {
  //   function_definition();
  // }

}

void Parser::label_declaration(){
  do{
    
    auto index = match_adv({TOKEN_TYPE::NUM_INT_TOKEN, TOKEN_TYPE::ID_TOKEN});
    auto id = m_lexer.get_token_view(index);
    
    if(auto it = m_current_block->m_labels.find(id); it != m_current_block->m_labels.end()){
      throw SemanticException(
        SEMANTIC_ERROR::SE_DUPLICATE_ID,
        std::format
        (
          "Semantic error: duplicate label id '{}' found at ({},{}) and ({},{}) !",
          id,
          m_lexer.getToken().m_line,
          m_lexer.getToken().m_col,
          it->second.m_line,
          it->second.m_col
        ),
        m_lexer.getToken().m_line,
        m_lexer.getToken().m_col
      );
    }

    m_current_block->m_labels[id] = {id,m_lexer.getToken().m_line,m_lexer.getToken().m_col};

    adv();
  }while (check(TOKEN_TYPE::COMMA_TOKEN));
  match(TOKEN_TYPE::SEMI_TOKEN);
}

void Parser::const_definition()
{

}

void Parser::statement()
{

}

}