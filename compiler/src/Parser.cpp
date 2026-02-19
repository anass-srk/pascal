#include "Parser.hpp"
#include <format>

namespace pascal_compiler
{

Parser::Parser(std::string&& content) : m_lexer(std::move(content))
{

}

void Parser::match(TOKEN_TYPE type)
{
  const auto& token = m_lexer.getToken();
  if(token.m_type != type){
    throw SyntaxException(std::format("Syntax error: expected {}, found {} !", TOKEN_NAMES[int(type)], TOKEN_NAMES[int(token.m_type)]),token);
  }
}

void Parser::match(std::initializer_list<TOKEN_TYPE> l){
  const auto current = m_lexer.getToken().m_type;
  for(auto type : l){
    if(type == current){
      return;
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

void Parser::program()
{
  match_adv(TOKEN_TYPE::PROGRAM_TOKEN);
  match_adv(TOKEN_TYPE::ID_TOKEN);
  m_program_name = m_lexer.getToken().m_id;
  match_adv(TOKEN_TYPE::SEMI_TOKEN);

  m_block = std::make_unique<Block>();
  Block::init_main_block(*m_block.get());

  m_current_block = m_block.get();
  block();

  match(TOKEN_TYPE::DOT_TOKEN);
}

void Parser::block()
{
  declaration();
  statement();
}

void Parser::declaration()
{

  adv();

  while(!check(TOKEN_TYPE::BEGIN_TOKEN)){

  if(check(TOKEN_TYPE::LABEL_TOKEN))
  {
    label_declaration();
  }
  if(check(TOKEN_TYPE::CONST_TOKEN))
  {
    const_definition();
  }
  if(check(TOKEN_TYPE::TYPE_TOKEN))
  {
    type_definition();
  }
  if(check(TOKEN_TYPE::VAR_TOKEN))
  {
    variable_declaration();
  }
  // else if(check(TOKEN_TYPE::PROCEDURE_TOKEN))
  // {
  //   procedure_definition();
  // }
  // else if(check(TOKEN_TYPE::FUNCTION_TOKEN))
  // {
  //   function_definition();
  // }
  else{
    break;
  }
  }
}

void Parser::label_declaration(){
  do
  {
    match_adv({TOKEN_TYPE::NUM_INT_TOKEN, TOKEN_TYPE::ID_TOKEN});
    Lexeme token = m_lexer.getToken();
    m_current_block->check_used_id(token);

    m_current_block->m_labels[token.m_id] = {token.m_id, token.m_line, token.m_col};

    adv();
  }while (check(TOKEN_TYPE::COMMA_TOKEN));
  match(TOKEN_TYPE::SEMI_TOKEN);
  adv();
}

void Parser::const_definition()
{
  match_adv(TOKEN_TYPE::ID_TOKEN);
  while(check(TOKEN_TYPE::ID_TOKEN))
  {
    Lexeme token = m_lexer.getToken();
    m_current_block->check_used_id(token);

    match_adv(TOKEN_TYPE::EQ_TOKEN);
    adv();
    m_current_block->m_consts.emplace(std::make_pair(token.m_id, constant(token)));

    match_adv(TOKEN_TYPE::SEMI_TOKEN);
    adv();
  };
}

Const Parser::constant(const Lexeme& token)
{
  if(check(TOKEN_TYPE::STRING_LITERAL_TOKEN))
  {
    return Const(
      token.m_id, // id of the contant
      token.m_line,
      token.m_col,
      m_lexer.getToken().to_string_literal(), // value of the constant
      *m_current_block
    );
  }

  if(check(TOKEN_TYPE::CHAR_LITERAL_TOKEN))
  {
    return Const(
      token.m_id,
      token.m_line,
      token.m_col,
      m_lexer.getToken().to_string_literal()[0],
      *m_current_block
    );
  }

  if(check(TOKEN_TYPE::TRUE_TOKEN))
  {
    return Const(
      token.m_id,
      token.m_line,
      token.m_col,
      true,
      *m_current_block
    );
  }

  if(check(TOKEN_TYPE::FALSE_TOKEN))
  {
    return Const(
      token.m_id,
      token.m_line,
      token.m_col,
      false,
      *m_current_block
    );
  }
  
  bool sign = false;
  Int fact = 1;
  
  if(check(TOKEN_TYPE::PLUS_TOKEN))
  {
    sign = true;
    adv();
  }
  else if(check(TOKEN_TYPE::MINUS_TOKEN))
  {
    sign = true;
    fact = -1;
    adv();
  }

  if(check(TOKEN_TYPE::NUM_INT_TOKEN))
  {
    return Const(
      token.m_id,
      token.m_line,
      token.m_col,
      m_lexer.getToken().m_ival * fact,
      *m_current_block
    );
  }

  if(check(TOKEN_TYPE::NUM_REAL_TOKEN))
  {
    return Const(
      token.m_id,
      token.m_line,
      token.m_col,
      m_lexer.getToken().m_dval * fact,
      *m_current_block
    );
  }

  if(check(TOKEN_TYPE::ID_TOKEN))
  {
    auto id = m_lexer.getToken().m_id;
    Block *current = m_current_block;
    
    do{
      
      if(auto enum_val = Block::get(id,current->m_enums_vals); enum_val != nullptr){
        if(sign){
          throw SemanticException(
            SEMANTIC_ERROR::SE_INVALID_OP,
            std::format(
              "Semantic error: applying a unary operation on the constant ({} at ({},{})) requires it to be an int or real ('{}' in your case) ! ",
              token.m_id, token.m_line, token.m_col, enum_val->m_type->m_name
            ),
            token.m_line,
            token.m_col
          );
        }
        return Const(
          token.m_id,
          token.m_line,
          token.m_col,
          *enum_val          
        );
      }

      if(auto const_val = Block::get(id,current->m_consts); const_val != nullptr){
        // Only ints and reals can be negative
        if(sign && const_val->m_cat != CONST_CAT::CC_INT && const_val->m_cat != CONST_CAT::CC_REAL){
          throw SemanticException(
            SEMANTIC_ERROR::SE_INVALID_OP,
            std::format(
              "Semantic error: applying a unary operation on the constant ({} at ({},{})) requires it to be an int or real ('{}' in your case) ! ",
              token.m_id, token.m_line, token.m_col, const_val->m_type->m_name
            ),
            token.m_line,
            token.m_col
          );
        }
        Const res = *const_val;
        res.m_name = token.m_id; // Must change name as well as location
        res.m_line = token.m_line;
        res.m_col = token.m_col;

        if(const_val->m_cat == CONST_CAT::CC_INT){
          res.m_val = std::get<Int>(res.m_val) * fact;
        }else if(const_val->m_cat == CONST_CAT::CC_REAL){
          res.m_val = std::get<Real>(res.m_val) * fact;
        }

        return res;
      }

      current = current->m_parent;
    }while(current);

    throw SemanticException(
      SEMANTIC_ERROR::SE_MISSING_ID,
      std::format(
        "Semantic error: to initialize a constant ({} at ({},{})) with an id, it needs to reference another constant or enum value ('{}' in your case) ! ",
        token.m_id, token.m_line, token.m_col, id
      ),
      token.m_line,
      token.m_col
    );
  }

  if(sign){ // Sign without actual value

    auto old = m_lexer.getToken();

    throw SyntaxException(
      std::format(
        "Syntax error: unary operations (+ and -) can be applied on int and/or real values ! ({},{})",
        old.m_line, old.m_col
      ),
      old
    );
  }

  match({
    TOKEN_TYPE::PLUS_TOKEN,
    TOKEN_TYPE::MINUS_TOKEN, 
    TOKEN_TYPE::ID_TOKEN, 
    TOKEN_TYPE::STRING_LITERAL_TOKEN, 
    TOKEN_TYPE::CHAR_LITERAL_TOKEN,
    TOKEN_TYPE::NUM_INT_TOKEN,
    TOKEN_TYPE::NUM_REAL_TOKEN,
    TOKEN_TYPE::TRUE_TOKEN,
    TOKEN_TYPE::FALSE_TOKEN
  });
}

void Parser::type_definition()
{
  match_adv(TOKEN_TYPE::ID_TOKEN);
  while (check(TOKEN_TYPE::ID_TOKEN))
  {
    Lexeme token = m_lexer.getToken();
    m_current_block->check_used_id(token);

    match_adv(TOKEN_TYPE::EQ_TOKEN);
    adv();

    get_type(token, true);

    match_adv(TOKEN_TYPE::SEMI_TOKEN);
    adv();
  }
}

// Atleast one element
std::vector<Lexeme> Parser::id_list(){
  std::vector<Lexeme> res;

  match(TOKEN_TYPE::ID_TOKEN);
  res.emplace_back(m_lexer.getToken());
  adv();
  
  while(check(TOKEN_TYPE::COMMA_TOKEN)){
    match_adv(TOKEN_TYPE::ID_TOKEN);
    res.emplace_back(m_lexer.getToken());
    adv();
  }
  
  return res;
}

std::unique_ptr<Enum> Parser::enum_type(const Lexeme& token){
  adv();
  auto vec = id_list();
  match(TOKEN_TYPE::RP_TOKEN);
  
  auto type = std::make_unique<Enum>(token.m_id, token.m_line, token.m_col);

  for(const auto& t : vec){
    m_current_block->check_used_id(t);
    type->insert(t, *m_current_block);
  }
  
  return type;
}

const Type* Parser::get_type(const Lexeme &token, bool named){
  if(check(TOKEN_TYPE::ID_TOKEN)){
    const auto id = m_lexer.getToken().m_id;
    Block *current = m_current_block;
    
    do{
      if(auto t = current->m_types.find(id); t != current->m_types.end()){
        if (named)
          return m_current_block->m_types.emplace(std::make_pair(token.m_id, t->second)).first->second.get();
        else 
          return t->second.get();
        break;
      }
    
      current = current->m_parent;
    }while(current);
  }

  auto type = type_eval(token);
  
  if(named){
    return m_current_block->m_types.emplace(std::make_pair(token.m_id, std::move(type))).first->second.get();
  }else{ 
    return m_current_block->m_unamed_types.emplace_back(std::move(type)).get();
  }
}

std::unique_ptr<Type> Parser::type_eval(const Lexeme& token){

  if(check(TOKEN_TYPE::LP_TOKEN)){ // Enum
    return enum_type(token);
  }

  else if(check(TOKEN_TYPE::ARRAY_TOKEN)){
    match_adv(TOKEN_TYPE::LB_TOKEN);

    std::vector<const Type*> types;
    do{
      adv();

      bool first = false, second = false;

      if(check(TOKEN_TYPE::ID_TOKEN)){ // If is TYPENAME of simple type
        const auto id = m_lexer.getToken().m_id;
        Block *current = m_current_block;
        
        do{
          if(auto t = Block::get(id, current->m_types); t && (t->m_type == TYPE_CAT::TC_ENUM || t->m_type == TYPE_CAT::TC_SUBRANGE)){
            types.emplace_back(t);
            first = true;
            break;
          }
        
          current = current->m_parent;
        }while(current);
      }

      else if(check(TOKEN_TYPE::LP_TOKEN)){ // Enum (unamed type)
        auto beg_token = m_lexer.getToken();
        types.emplace_back(
          m_current_block->m_unamed_types.emplace_back(enum_type(beg_token)).get()
        );
        second = true;
      }

      if (!first && !second){ //Subrange (unamed as well)
        auto beg_token = m_lexer.getToken();
        Const beg = constant(beg_token);
        match_adv(TOKEN_TYPE::RANGE_TOKEN);
        Const end = constant(m_lexer.next_sym());

        types.emplace_back(
          m_current_block->m_unamed_types.emplace_back(
            std::make_unique<Subrange>(beg_token.m_id, beg_token.m_line, beg_token.m_col, std::move(beg), std::move(end))
          ).get()
        );
      }

      adv();
    }while(check(TOKEN_TYPE::COMMA_TOKEN));

    match(TOKEN_TYPE::RB_TOKEN);
    match_adv(TOKEN_TYPE::OF_TOKEN);
    adv();

    if(check(TOKEN_TYPE::ID_TOKEN)){
      const auto id = m_lexer.getToken().m_id;
      Block *current = m_current_block;

      do{
        if(auto t = Block::get(id, current->m_types); t){
          return std::make_unique<Array>(
            token.m_id, token.m_line, token.m_col, std::move(types), t
          );
        }
      
        current = current->m_parent;
      }while(current);
    }

    auto const beg_token = m_lexer.getToken();

    return std::make_unique<Array>(
      token.m_id, token.m_line, token.m_col, std::move(types),
      m_current_block->m_unamed_types.emplace_back(type_eval(beg_token)).get()
    );
    
  }

  else if(check(TOKEN_TYPE::RECORD_TOKEN)){
    auto record = field_list(token);
    match(TOKEN_TYPE::END_TOKEN);
    return record;
  }

  auto const beg_token = m_lexer.getToken();
  Const beg = constant(beg_token);
  match_adv(TOKEN_TYPE::RANGE_TOKEN);
  Const end = constant(m_lexer.next_sym());

  return std::make_unique<Subrange>(token.m_id, token.m_line, token.m_col, std::move(beg), std::move(end));
}

std::unique_ptr<Record> Parser::field_list(const Lexeme& token){
  std::unique_ptr<Record> record = std::make_unique<Record>(token.m_id, token.m_line, token.m_col);
  do{
    adv();
    if(check(TOKEN_TYPE::CASE_TOKEN) || check(TOKEN_TYPE::END_TOKEN)) break; // Move to variant part or quit
    auto var_names = id_list();
    match(TOKEN_TYPE::COLON_TOKEN);
    adv();
    auto type = get_type(token, false);
    adv();

    for(const auto& name : var_names){
      record->check_duplicate_id(token, name);
      record->m_members[name.m_id] = (Var){name.m_id, name.m_line, name.m_col, type};
    }

  }while(check(TOKEN_TYPE::SEMI_TOKEN));

  if(!check(TOKEN_TYPE::CASE_TOKEN)) return record;

  adv();
  match(TOKEN_TYPE::ID_TOKEN);

  const auto id = m_lexer.getToken().m_id;

  const Type* type = nullptr;
  const Var* var = nullptr;

  {
    Block *current = m_current_block;
    do{
      if(auto t = Block::get(id, current->m_types); t && (t->m_type == TYPE_CAT::TC_ENUM || t->m_type == TYPE_CAT::TC_SUBRANGE) ){
        type = t;
      }
      current = current->m_parent;
    }while(current);
  }

  if(type){ // tag variable not declared explicitly
    for(const auto &[name, v] : record->m_members){
      if(v.m_type == type){
        if(var){ // Already found a tag
          const auto line = m_lexer.getToken().m_line;
          const auto col = m_lexer.getToken().m_col;
          throw SemanticException(
            SEMANTIC_ERROR::SE_AMBIGUOUS_TAG_VAR,
            std::format(
              "Semantic error: Ambiguous tag variable for the record '{}' ({} or {} ?) at ({},{}) !\n",
              record->m_name, name, var->m_name, line, col
            ),
            line, col
          );
        }
        var = &v;
      }
    }
    if(!var){
      const auto line = m_lexer.getToken().m_line;
      const auto col = m_lexer.getToken().m_col;
      throw SemanticException(
        SEMANTIC_ERROR::SE_MISSING_ID,
        std::format(
          "Semantic error: Missing tag variable for the record '{}' at ({},{}) !\n",
          record->m_name, line, col),
        line, col
      );
    }
  }else{
    if(record->m_members.contains(id)){
      const auto line = m_lexer.getToken().m_line;
      const auto col = m_lexer.getToken().m_col;
      throw SemanticException(
        SEMANTIC_ERROR::SE_DUPLICATE_ID,
        std::format(
          "Semantic error: tag variable name '{}' is already for the record '{}' at ({},{}) !\n",
          id, record->m_name, line, col),
        line, col);
    }
    
    const auto var_line = m_lexer.getToken().m_line;
    const auto var_col = m_lexer.getToken().m_col;
    
    match_adv(TOKEN_TYPE::COLON_TOKEN);
    match_adv(TOKEN_TYPE::ID_TOKEN);
    const auto type_id = m_lexer.getToken().m_id;
    {
      Block *current = m_current_block;
      do{
        if (auto t = Block::get(type_id, current->m_types); t && (t->m_type == TYPE_CAT::TC_ENUM || t->m_type == TYPE_CAT::TC_SUBRANGE)){
          type = t;
        }
        current = current->m_parent;
      } while (current);
    }
    if(!type){
      const auto line = m_lexer.getToken().m_line;
      const auto col = m_lexer.getToken().m_col;
      throw SemanticException(
        SEMANTIC_ERROR::SE_MISSING_ID,
        std::format(
          "Semantic error: tag variable type '{}' for the record '{}' at ({},{}) is not defined !\n",
          type_id, record->m_name, line, col),
        line, col
      );
    }
    var = &(record->m_members[id] = (Var){id, var_line, var_col, type}); 
  }

  match_adv(TOKEN_TYPE::OF_TOKEN);

  do{
    adv();
    auto constants = case_label_list();

    if(type->m_type == TYPE_CAT::TC_ENUM){
      for (const auto &c : constants){
        if (c.m_type != type){
          throw SemanticException(
            SEMANTIC_ERROR::SE_MISSING_ID,
            std::format(
              "Semantic error: case label '{}' for the record '{}' at ({},{}) is not an enum of the tag type '{}' at ({},{}) !\n",
              c.m_name, record->m_name, c.m_line, c.m_col, type->m_name, type->m_line, type->m_col),
            c.m_line, c.m_col
          );
        }
      }
    }else{ // Type is a subrange
      auto sub = static_cast<const Subrange*>(type);
      for(const auto& c : constants){
        if(c.m_cat != sub->m_cat){ // Char, Enum or Int
          throw SemanticException(
            SEMANTIC_ERROR::SE_MISSING_ID,
            std::format(
              "Semantic error: case label '{}' for the record '{}' at ({},{}) is not a subrange of the tag type '{}' at ({},{}) !\n",
              c.m_name, record->m_name, c.m_line, c.m_col, type->m_name, type->m_line, type->m_col),
            c.m_line, c.m_col
          );
        }
        const Int val = (std::holds_alternative<char>(c.m_val) ? static_cast<Int>(std::get<char>(c.m_val)) : std::get<Int>(c.m_val));
        if(val < sub->m_beg || val > sub->m_end){
          throw SemanticException(
            SEMANTIC_ERROR::SE_MISSING_ID,
            std::format(
              "Semantic error: case label '{}' for the record '{}' at ({},{}) is not in the subrange ({} <= {}) of the tag type '{}' at ({},{}) !\n",
              c.m_name, record->m_name, c.m_line, c.m_col, sub->m_beg, sub->m_end, type->m_name, type->m_line, type->m_col),
            c.m_line, c.m_col
          );
        }
      }
    }

    match(TOKEN_TYPE::COLON_TOKEN);
    match_adv(TOKEN_TYPE::LP_TOKEN);
    auto variant = field_list(m_lexer.getToken());
    match(TOKEN_TYPE::RP_TOKEN);
    adv();

    for (const auto &[id,var] : variant->m_members){
      record->check_duplicate_id(token, var);
      record->m_members[id] = var;
    }

  }while(check(TOKEN_TYPE::SEMI_TOKEN));

  return record;
}

std::vector<Const> Parser::case_label_list(){
  std::vector<Const> v;

  v.push_back(constant(m_lexer.getToken()));
  adv();
  while(check(TOKEN_TYPE::COMMA_TOKEN)){
    adv();
    v.push_back(constant(m_lexer.getToken()));
    adv();
  }

  return v;
}

void Parser::variable_declaration()
{
  adv();
  do{
    const auto names = id_list();
    match(TOKEN_TYPE::COLON_TOKEN);
    adv();
    const auto type = get_type(m_lexer.getToken(), false);
    match_adv(TOKEN_TYPE::SEMI_TOKEN);
    adv();

    for (const auto &name : names){
      m_current_block->check_used_id(name);
      m_current_block->m_vars[name.m_id] = (Var){name.m_id, name.m_line, name.m_col, type};
    }
    
  }while(check(TOKEN_TYPE::ID_TOKEN));
}

void Parser::statement()
{

}

}