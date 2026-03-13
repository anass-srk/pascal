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
  if(token.type() != type){
    throw SyntaxException(std::format("Syntax error: At ({}), expected {}, found {} !", token.to_string(), TOKEN_NAMES[int(type)], TOKEN_NAMES[int(token.type())]),token);
  }
}

void Parser::match(std::initializer_list<TOKEN_TYPE> l){
  const auto current = m_lexer.getToken().type();
  for(auto type : l){
    if(type == current){
      return;
    }
  }
  
  auto it = l.begin();
  std::string err = std::format("Syntax error: At ({}), expected (", m_lexer.getToken().to_string()) + std::string(TOKEN_NAMES[int(*it)]);
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
  m_program_name = m_lexer.getToken().id();
  match_adv(TOKEN_TYPE::SEMI_TOKEN);

  m_block = std::make_unique<Block>();
  Block::init_main_block(*m_block.get());

  m_current_block = m_block.get();
  _top = m_current_block;
  block();

  match(TOKEN_TYPE::DOT_TOKEN);
}

void Parser::block()
{
  declaration();
  if(check(TOKEN_TYPE::BEGIN_TOKEN))
  {
    m_current_block->body = compound_stmt();
  }
}

void Parser::declaration()
{

  adv();

  while(!check(TOKEN_TYPE::BEGIN_TOKEN)){

  if(check(TOKEN_TYPE::LABEL_TOKEN))
  {
    label_declaration();
    continue;
  }
  if(check(TOKEN_TYPE::CONST_TOKEN))
  {
    const_definition();
    continue;
  }
  if(check(TOKEN_TYPE::TYPE_TOKEN))
  {
    type_definition();
    continue;
  }
  if(check(TOKEN_TYPE::VAR_TOKEN))
  {
    variable_declaration();
    continue;
  }
  if(check(TOKEN_TYPE::PROCEDURE_TOKEN))
  {
    function_definition(true);
    continue;
  }
  if(check(TOKEN_TYPE::FUNCTION_TOKEN))
  {
    function_definition(false);
    continue;
  }
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

    m_current_block->m_labels[token.id()] = {token.id(), token.line(), token.column()};

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
    m_current_block->m_consts.emplace(std::make_pair(token.id(), constant(token)));

    match_adv(TOKEN_TYPE::SEMI_TOKEN);
    adv();
  };
}

Const Parser::constant(const Lexeme& token)
{
  if(check(TOKEN_TYPE::STRING_LITERAL_TOKEN))
  {
    return Const(
      token.id(), // id of the contant
      token.line(),
      token.column(),
      m_lexer.getToken().to_string_literal(), // value of the constant
      *getTop()
    );
  }

  if(check(TOKEN_TYPE::CHAR_LITERAL_TOKEN))
  {
    return Const(
      token.id(),
      token.line(),
      token.column(),
      m_lexer.getToken().to_string_literal()[0],
      *getTop()
    );
  }

  if(check(TOKEN_TYPE::TRUE_TOKEN))
  {
    return Const(
      token.id(),
      token.line(),
      token.column(),
      true,
      *getTop()
    );
  }

  if(check(TOKEN_TYPE::FALSE_TOKEN))
  {
    return Const(
      token.id(),
      token.line(),
      token.column(),
      false,
      *getTop()
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
      token.id(),
      token.line(),
      token.column(),
      m_lexer.getToken().int_value() * fact,
      *getTop()
    );
  }

  if(check(TOKEN_TYPE::NUM_REAL_TOKEN))
  {
    return Const(
      token.id(),
      token.line(),
      token.column(),
      m_lexer.getToken().real_value() * fact,
      *getTop()
    );
  }

  if(check(TOKEN_TYPE::ID_TOKEN))
  {
    auto id = m_lexer.getToken().id();
    Block *current = m_current_block;
    
    do{
      
      if(auto enum_val = Block::get(id,current->m_enums_vals); enum_val != nullptr){
        if(sign){
          throw SemanticException(
            SEMANTIC_ERROR::SE_INVALID_OP,
            std::format(
              "Semantic error: applying a unary operation on the constant ({} at ({},{})) requires it to be an int or real ('{}' in your case) ! ",
              token.id(), token.line(), token.column(), enum_val->m_type->m_name
            ),
            token.line(),
            token.column()
          );
        }
        return Const(
          token.id(),
          token.line(),
          token.column(),
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
              token.id(), token.line(), token.column(), const_val->m_type->m_name
            ),
            token.line(),
            token.column()
          );
        }
        Const res = *const_val;
        res.m_name = token.id(); // Must change name as well as location
        res.m_line = token.line();
        res.m_col = token.column();

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
        token.id(), token.line(), token.column(), id
      ),
      token.line(),
      token.column()
    );
  }

  if(sign){ // Sign without actual value

    auto old = m_lexer.getToken();

    throw SyntaxException(
      std::format(
        "Syntax error: unary operations (+ and -) can be applied on int and/or real values ! ({},{})",
        old.line(), old.column()
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

    match(TOKEN_TYPE::SEMI_TOKEN);
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
  match(TOKEN_TYPE::LP_TOKEN);
  adv();
  auto vec = id_list();
  match(TOKEN_TYPE::RP_TOKEN);
  
  auto type = std::make_unique<Enum>(token.id(), token.line(), token.column());

  for(const auto& t : vec){
    m_current_block->check_used_id(t);
    type->insert(t, *m_current_block);
  }
  adv();
  return type;
}

const Type* Parser::get_type(const Lexeme &token, bool named){
  if(check(TOKEN_TYPE::ID_TOKEN)){
    const auto id = m_lexer.getToken().id();
    Block *current = m_current_block;
    
    do{
      if(auto t = current->m_types.find(id); t != current->m_types.end()){
        adv();
        if (named)
          return m_current_block->m_types.emplace(std::make_pair(token.id(), t->second)).first->second.get();
        else 
          return t->second.get();
        break;
      }
    
      current = current->m_parent;
    }while(current);
  }

  auto type = type_eval(token);
  
  if(named){
    return m_current_block->m_types.emplace(std::make_pair(token.id(), std::move(type))).first->second.get();
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
        const auto id = m_lexer.getToken().id();
        Block *current = m_current_block;
        
        do{
          if(auto t = Block::get(id, current->m_types); t && (t->m_type == TYPE_CAT::TC_ENUM || t->m_type == TYPE_CAT::TC_SUBRANGE)){
            types.emplace_back(t);
            first = true;
            adv();
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
        adv();

        types.emplace_back(
          m_current_block->m_unamed_types.emplace_back(
            std::make_unique<Subrange>(beg_token.id(), beg_token.line(), beg_token.column(), std::move(beg), std::move(end))
          ).get()
        );
      }

    }while(check(TOKEN_TYPE::COMMA_TOKEN));

    match(TOKEN_TYPE::RB_TOKEN);
    match_adv(TOKEN_TYPE::OF_TOKEN);
    adv();

    if(check(TOKEN_TYPE::ID_TOKEN)){
      const auto id = m_lexer.getToken().id();
      Block *current = m_current_block;

      do{
        if(auto t = Block::get(id, current->m_types); t){
          adv();
          return std::make_unique<Array>(
            token.id(), token.line(), token.column(), std::move(types), t
          );
        }
      
        current = current->m_parent;
      }while(current);
    }

    auto const beg_token = m_lexer.getToken();
    return std::make_unique<Array>(
      token.id(), token.line(), token.column(), std::move(types),
      m_current_block->m_unamed_types.emplace_back(type_eval(beg_token)).get()
    );
    
  }

  else if(check(TOKEN_TYPE::RECORD_TOKEN)){
    auto record = field_list(token);
    match(TOKEN_TYPE::END_TOKEN);
    adv();
    return record;
  }

  else if(check(TOKEN_TYPE::PROCEDURE_TOKEN)){
    auto proc_type = std::make_unique<FunctionType>(token.id(), token.line(), token.column(), nullptr);
    adv();
    if(check(TOKEN_TYPE::LP_TOKEN)){
      proc_type->m_args = args_list();
      adv();
    }
    return proc_type;
  }

  else if (check(TOKEN_TYPE::FUNCTION_TOKEN))
  {
    auto func_type = std::make_unique<FunctionType>(token.id(), token.line(), token.column(), nullptr);
    adv();
    if (check(TOKEN_TYPE::LP_TOKEN)){
      func_type->m_args = args_list();
      adv();
    }
    match(TOKEN_TYPE::COLON_TOKEN);
    match_adv(TOKEN_TYPE::ID_TOKEN);
    func_type->m_ret_type = find_type(true);
    adv();
    return func_type;
  }

  auto const beg_token = m_lexer.getToken();
  Const beg = constant(beg_token);
  match_adv(TOKEN_TYPE::RANGE_TOKEN);
  Const end = constant(m_lexer.next_sym());
  adv();

  return std::make_unique<Subrange>(token.id(), token.line(), token.column(), std::move(beg), std::move(end));
}

const Type* Parser::find_type(bool required)
{
  match(TOKEN_TYPE::ID_TOKEN);
  const Type *t = nullptr;
  auto tkn = m_lexer.getToken();
  auto type_id = tkn.id();
  Block *current = m_current_block;
  do
  {
    if (const auto _t = Block::get(type_id, current->m_types); _t)
    {
      t = _t;
      break;
    }
    current = current->m_parent;
  } while (current);

  if (t == nullptr && required)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_TYPE,
      std::format("Semantic error: Invalid typename ({}) !", tkn.to_string()),
      tkn.line(),
      tkn.column()
    );
  }
  return t;
}

std::vector<Arg> Parser::args_list()
{
  std::vector<Arg> args;
  std::unordered_map<std::string_view, Lexeme> used_ids;
  match(TOKEN_TYPE::LP_TOKEN);
  do{
    adv();
    bool ref = false;
    if(check(TOKEN_TYPE::VAR_TOKEN)){
      ref = true;
      adv();
    }
    const auto ids = id_list();
    match(TOKEN_TYPE::COLON_TOKEN);
    match_adv(TOKEN_TYPE::ID_TOKEN);
    
    const Type* t = find_type(true);

    for(const auto& id : ids){
      // Check if argument name is taken
      if(auto it = used_ids.find(id.id()); it != used_ids.end())
      {
        throw SemanticException(
          SEMANTIC_ERROR::SE_DUPLICATE_ID,
          std::format("Semantic error: '{}' is found in ({}) and ({}) !", it->first, it->second.to_string(), id.to_string()),
          id.line(),
          id.column()
        );
      }
      used_ids.insert(std::make_pair(id.id(), id));

      args.emplace_back(ref, id, t);
    }

    adv();
  }while(check(TOKEN_TYPE::SEMI_TOKEN));
  match(TOKEN_TYPE::RP_TOKEN);
  return args;
}

std::unique_ptr<Record> Parser::field_list(const Lexeme& token){
  std::unique_ptr<Record> record = std::make_unique<Record>(token.id(), token.line(), token.column());
  do{
    adv();
    if(check(TOKEN_TYPE::CASE_TOKEN) || check(TOKEN_TYPE::END_TOKEN)) break; // Move to variant part or quit
    auto var_names = id_list();
    match(TOKEN_TYPE::COLON_TOKEN);
    adv();
    auto type = get_type(token, false);

    for(const auto& name : var_names){
      record->check_duplicate_id(token, name);
      record->m_members[name.id()] = (Var){name.id(), name.line(), name.column(), type};
    }

  }while(check(TOKEN_TYPE::SEMI_TOKEN));

  if(!check(TOKEN_TYPE::CASE_TOKEN)) return record;

  adv();
  match(TOKEN_TYPE::ID_TOKEN);

  const auto id = m_lexer.getToken().id();

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
          const auto line = m_lexer.getToken().line();
          const auto col = m_lexer.getToken().column();
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
      const auto line = m_lexer.getToken().line();
      const auto col = m_lexer.getToken().column();
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
      const auto line = m_lexer.getToken().line();
      const auto col = m_lexer.getToken().column();
      throw SemanticException(
        SEMANTIC_ERROR::SE_DUPLICATE_ID,
        std::format(
          "Semantic error: tag variable name '{}' is already for the record '{}' at ({},{}) !\n",
          id, record->m_name, line, col),
        line, col);
    }
    
    const auto var_line = m_lexer.getToken().line();
    const auto var_col = m_lexer.getToken().column();
    
    match_adv(TOKEN_TYPE::COLON_TOKEN);
    match_adv(TOKEN_TYPE::ID_TOKEN);
    const auto type_id = m_lexer.getToken().id();
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
      const auto line = m_lexer.getToken().line();
      const auto col = m_lexer.getToken().column();
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
    match(TOKEN_TYPE::SEMI_TOKEN);
    adv();

    for (const auto &name : names){
      m_current_block->check_used_id(name);
      m_current_block->m_vars[name.id()] = (Var){name.id(), name.line(), name.column(), type};
    }
    
  }while(check(TOKEN_TYPE::ID_TOKEN));
}

void Parser::function_definition(bool is_proc)
{
  match_adv(TOKEN_TYPE::ID_TOKEN);

  auto token = m_lexer.getToken();
  const auto id = token.id();
  auto func_type = new FunctionType (token.id(), token.line(), token.column(), nullptr);

  m_current_block->check_used_id(token);

  adv();
  if(check(TOKEN_TYPE::LP_TOKEN)){
    func_type->m_args = args_list();
    adv();
  }

  if(is_proc)
  {
    match(TOKEN_TYPE::SEMI_TOKEN);
  }
  else
  {
    match(TOKEN_TYPE::COLON_TOKEN);
    match_adv(TOKEN_TYPE::ID_TOKEN);
    func_type->m_ret_type = find_type(true);
    match_adv(TOKEN_TYPE::SEMI_TOKEN);
  }

  m_current_block->m_unamed_types.emplace_back(func_type);

  Block *parent = m_current_block;
  m_current_block = new Block();
  m_current_block->m_parent = parent;

  // to return a value, we assign to a variable with the id of the function (not for procedures)
  if(!is_proc) m_current_block->m_vars.insert(std::make_pair(id, (Var){token.id(), token.line(), token.column(), func_type->m_ret_type}));
  for (const auto &arg : func_type->m_args)
  {
    m_current_block->m_vars.insert(std::make_pair(arg.m_name, Var(arg)));
  }

  block();
  match(TOKEN_TYPE::DOT_TOKEN);
  adv();

  auto *func_block = m_current_block;
  m_current_block = parent;

  m_current_block->m_funcs.insert(std::make_pair(id, Function(id, func_type, func_block)));
}

std::unique_ptr<Expression> Parser::gexpression()
{
  const auto token = m_lexer.getToken();
  auto first = expression();
  if(check({
    TOKEN_TYPE::EQ_TOKEN, TOKEN_TYPE::NEQ_TOKEN, TOKEN_TYPE::GE_TOKEN,
    TOKEN_TYPE::GT_TOKEN, TOKEN_TYPE::LT_TOKEN, TOKEN_TYPE::LE_TOKEN
  }))
  {
    BinaryOp op;
    switch(m_lexer.getToken().type())
    {
      case TOKEN_TYPE::EQ_TOKEN:
        op = BinaryOp::Eq;
      break;
      case TOKEN_TYPE::NEQ_TOKEN:
        op = BinaryOp::Ne;
      break;
      case TOKEN_TYPE::GE_TOKEN:
        op = BinaryOp::Ge;
      break;
      case TOKEN_TYPE::GT_TOKEN:
        op = BinaryOp::Gt;
      break;
      case TOKEN_TYPE::LT_TOKEN:
        op = BinaryOp::Lt;
      break;
      case TOKEN_TYPE::LE_TOKEN:
        op = BinaryOp::Le;
      break;
    }
    adv();

    first = std::make_unique<BinaryExpression>(op, std::move(first), expression(), token, getTop()->m_types.at(CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)]).get());
    first->validate();
    return first;
  }
  return first;
}

std::unique_ptr<Expression> Parser::expression()
{
  const auto token = m_lexer.getToken();

  bool pos = true;
  bool sign = false;
  if(check(TOKEN_TYPE::PLUS_TOKEN))
  {
    pos = true;
    sign = true;
    adv();
  }
  else if(check(TOKEN_TYPE::MINUS_TOKEN))
  {
    pos = false;
    sign = true;
    adv();
  }

  auto first = term();

  // Only one term (no addition operators)
  if (!check({TOKEN_TYPE::PLUS_TOKEN, TOKEN_TYPE::MINUS_TOKEN, TOKEN_TYPE::OR_TOKEN}))
  {
    if(!sign) return first;

    auto uexp = std::make_unique<UnaryExpression>((pos ? UnaryOp::Plus : UnaryOp::Minus), std::move(first), token);
    uexp->validate();
    return uexp;
  }

  const auto loc = first->token;
  auto nexp = std::make_unique<NExpression>(std::move(first), loc);

  while (check({TOKEN_TYPE::PLUS_TOKEN, TOKEN_TYPE::MINUS_TOKEN, TOKEN_TYPE::OR_TOKEN}))
  {
    BinaryOp op;
    switch(m_lexer.getToken().type())
    {
      case TOKEN_TYPE::PLUS_TOKEN:
        op = BinaryOp::Add;
      break;
      case TOKEN_TYPE::MINUS_TOKEN:
        op = BinaryOp::Sub;
      break;
      case TOKEN_TYPE::OR_TOKEN:
        op = BinaryOp::Or;
      break;
    }
    adv();
    nexp->add(op, term());
  }

  nexp->validate();

  if(sign)
  {
    auto uexp = std::make_unique<UnaryExpression>((pos ? UnaryOp::Plus : UnaryOp::Minus), std::move(nexp), token);
    uexp->validate();
    return uexp;
  }

  return nexp;
}

std::unique_ptr<Expression> Parser::term()
{
  const auto token = m_lexer.getToken();

  auto first = factor();

  if(!check({TOKEN_TYPE::STAR_TOKEN, TOKEN_TYPE::SLASH_TOKEN, TOKEN_TYPE::DIV_TOKEN, TOKEN_TYPE::AND_TOKEN})) return first;

  const auto loc = first->token;
  auto nexp = std::make_unique<NExpression>(std::move(first), loc);

  while (check({TOKEN_TYPE::STAR_TOKEN, TOKEN_TYPE::SLASH_TOKEN, TOKEN_TYPE::DIV_TOKEN, TOKEN_TYPE::AND_TOKEN}))
  {
    BinaryOp op;
    switch (m_lexer.getToken().type())
    {
    case TOKEN_TYPE::STAR_TOKEN:
      op = BinaryOp::Mul;
      break;
      case TOKEN_TYPE::SLASH_TOKEN:
      case TOKEN_TYPE::DIV_TOKEN:
        op = BinaryOp::Div;
      break;
      case TOKEN_TYPE::AND_TOKEN:
        op = BinaryOp::And;
      break;
      default:
      break;
    }
    adv();
    nexp->add(op, factor());
  }

  nexp->validate();

  return nexp;
}

std::unique_ptr<Expression> Parser::factor()
{
  const auto token = m_lexer.getToken();
  if(
    check({
      TOKEN_TYPE::NUM_INT_TOKEN, TOKEN_TYPE::NUM_REAL_TOKEN, TOKEN_TYPE::CHAR_LITERAL_TOKEN,
      TOKEN_TYPE::STRING_LITERAL_TOKEN, TOKEN_TYPE::TRUE_TOKEN, TOKEN_TYPE::FALSE_TOKEN
    })
  )
  {
    auto res = std::make_unique<LiteralExpression>(std::make_unique<Const>(constant(token)), token);
    res->validate();
    adv();
    return res;
  }

  if(check(TOKEN_TYPE::NOT_TOKEN))
  {
    adv();
    auto res = std::make_unique<UnaryExpression>(UnaryOp::Not, factor(), token);
    res->validate();
    return res;
  }

  if(check(TOKEN_TYPE::LP_TOKEN))
  {
    adv();
    auto res = expression();
    match(TOKEN_TYPE::RP_TOKEN);
    adv();
    return res;
  }

  if(!check(TOKEN_TYPE::ID_TOKEN))
  {
    match({
      TOKEN_TYPE::NUM_INT_TOKEN, TOKEN_TYPE::NUM_REAL_TOKEN, TOKEN_TYPE::CHAR_LITERAL_TOKEN,
      TOKEN_TYPE::STRING_LITERAL_TOKEN, TOKEN_TYPE::TRUE_TOKEN, TOKEN_TYPE::FALSE_TOKEN,
      TOKEN_TYPE::NOT_TOKEN, TOKEN_TYPE::LP_TOKEN, TOKEN_TYPE::ID_TOKEN, TOKEN_TYPE::READ_TOKEN
    });
  }

  const Var *var = nullptr;
  const Const *cons = nullptr;
  const Function *func = nullptr;
  const EnumValue* ev = nullptr;

  const auto id = token.id();
  Block *current = m_current_block;

  do
  {
    if(auto v  = Block::get(id, current->m_vars); v)
    {
      var = v;
      break;
    }
    if(auto f = Block::get(id, current->m_funcs); f && f->m_type->m_ret_type != nullptr) // A function, not a procedure
    {
      func = f;
      break;
    }
    if(auto e = Block::get(id, current->m_enums_vals); e)
    {
      ev = e;
      break;
    }
    if(auto c = Block::get(id, current->m_consts); c)
    {
      cons = c;
      break;
    }

    current = current->m_parent;
  } while (current);

  if(cons)
  {
    auto res = std::make_unique<LiteralExpression>(std::make_unique<Const>(Const(*cons)), token);
    res->validate();
    adv();
    return res;
  }
  if(ev)
  {
    auto res = std::make_unique<LiteralExpression>(std::make_unique<Const>(token.id(), token.line(), token.column(), *ev), token);
    res->validate();
    adv();
    return res;
  }

  // Vars or funcs or none (error)
  if(var)
  {
    return variable_access(var);
  }

  if(func)
  {
    return std::get<std::unique_ptr<FunctionCall>>(function_call(func, false));
  }

  throw SemanticException(
    SEMANTIC_ERROR::SE_MISSING_ID,
    std::format("Semantic error: '{}' does not correspond to a variable, a constant, a function or an enum !", id),
    token.line(),
    token.column()
  );
}

std::variant<std::unique_ptr<FunctionCall>, std::unique_ptr<ProcedureCall>> Parser::function_call(const Function *f, bool is_procedure)
{
  const auto tkn = m_lexer.getToken();
  const auto func_type = f->m_type;
  std::vector<std::unique_ptr<Expression>> args;
  match(TOKEN_TYPE::ID_TOKEN);
  adv();
  
  if(check(TOKEN_TYPE::LP_TOKEN))
  {
    do
    {
      adv();
      args.push_back(gexpression());
    }while(check(TOKEN_TYPE::COMMA_TOKEN));
    match(TOKEN_TYPE::RP_TOKEN);
    adv();
  }
  if(!is_procedure)
  {
    auto res = std::make_unique<FunctionCall>(f, std::move(args), tkn);
    res->validate();
    return res;
  }
  auto res = std::make_unique<ProcedureCall>(f, std::move(args), tkn);
  res->validate();
  return res;
}

std::unique_ptr<VariableAccess> Parser::variable_access(const Var *v)
{
  const auto tkn = m_lexer.getToken();

  std::vector<std::unique_ptr<Selector>> selectors;
  match(TOKEN_TYPE::ID_TOKEN);

  if(v == nullptr)
  {
    const auto id = tkn.id();
    Block *current = m_current_block;

    do
    {
      if (auto _v = Block::get(id, current->m_vars); _v)
      {
        v = _v;
        break;
      }

      current = current->m_parent;
    } while (current);
  }

  adv();

  while(check({TOKEN_TYPE::DOT_TOKEN, TOKEN_TYPE::LB_TOKEN}))
  {
    if(check(TOKEN_TYPE::DOT_TOKEN))
    {
      match_adv(TOKEN_TYPE::ID_TOKEN);
      const auto token = m_lexer.getToken();
      selectors.push_back(std::make_unique<FieldSelector>(token.id(), token));
      adv();
      continue;
    }
    std::vector<std::unique_ptr<Expression>> indices;
    do
    {
      adv();
      const auto token = m_lexer.getToken();
      indices.push_back(gexpression());
    }while(check(TOKEN_TYPE::COMMA_TOKEN));
    match(TOKEN_TYPE::RB_TOKEN);
    adv();
    Lexeme loc = indices[0]->token;
    selectors.push_back(std::make_unique<ArraySelector>(std::move(indices), loc));
  }

  auto res = std::make_unique<VariableAccess>(v, std::move(selectors), tkn);
  res->validate();
  return res;
}

std::unique_ptr<CompoundStatement> Parser::compound_stmt()
{
  const auto token = m_lexer.getToken();
  std::vector<std::unique_ptr<Statement>> statements;

  match(TOKEN_TYPE::BEGIN_TOKEN);
  do
  {
    adv();
    statements.push_back(statement());
  }while(check(TOKEN_TYPE::SEMI_TOKEN));

  match(TOKEN_TYPE::END_TOKEN);
  adv();

  auto res = std::make_unique<CompoundStatement>(std::move(statements), token);
  res->validate();
  return res;
}

std::unique_ptr<Statement> Parser::statement()
{

  const auto token = m_lexer.getToken();

  if(check({TOKEN_TYPE::ID_TOKEN, TOKEN_TYPE::NUM_INT_TOKEN})) // label or variable or procedure
  {
    const Var *var = nullptr;
    const Function *func = nullptr;
    const Label *label = nullptr;

    const auto id = token.id();
    Block *current = m_current_block;

    do
    {
      if (auto v = Block::get(id, current->m_vars); v)
      {
        var = v;
        break;
      }
      if (auto f = Block::get(id, current->m_funcs); f && f->m_type->m_ret_type == nullptr) // Not a function, a procedure
      {
        func = f;
        break;
      }
      if (auto l = Block::get(id, current->m_labels); l)
      {
        label = l;
        break;
      }

      current = current->m_parent;
    } while (current);

    if(label)
    {
      match_adv(TOKEN_TYPE::COLON_TOKEN);
      adv();
      auto res = std::make_unique<LabeledStatement>(label, statement(), token);
      res->validate();
      return res;
    }

    if(var)
    {
      auto v = variable_access(var);
      match(TOKEN_TYPE::ASSIGN_TOKEN);
      adv();
      auto res = std::make_unique<AssignmentStatement>(std::move(v), gexpression(), token);
      res->validate();
      return res;
    }

    // Handle procedures
    if(func)
    {
      return std::get<std::unique_ptr<ProcedureCall>>(function_call(func, true));
    }

    throw SemanticException(
      SEMANTIC_ERROR::SE_MISSING_ID,
      std::format(
        "Semantic error : invalid statement ! {} doesn't correspond to a label, a function or a variable !",
        token.to_string()
      ),
      token.line(),
      token.column()
    );
  }

  //TODO: Handle writes/reads
  if(check(TOKEN_TYPE::WRITE_TOKEN))
  {
    std::vector<std::unique_ptr<Expression>> args;
    match_adv(TOKEN_TYPE::LP_TOKEN);
    do{
      adv();
      args.push_back(gexpression());
    }while(check(TOKEN_TYPE::COMMA_TOKEN));
    match(TOKEN_TYPE::RP_TOKEN);
    adv();
    auto res = std::make_unique<WriteStatement>(token, std::move(args));
    res->validate();
    return res;
  }

  if(check(TOKEN_TYPE::READ_TOKEN))
  {
    std::vector<std::unique_ptr<VariableAccess>> args;
    match_adv(TOKEN_TYPE::LP_TOKEN);
    do
    {
      adv();
      args.push_back(variable_access(nullptr));
    } while (check(TOKEN_TYPE::COMMA_TOKEN));
    match(TOKEN_TYPE::RP_TOKEN);
    adv();

    auto res = std::make_unique<ReadStatement>(token, std::move(args));
    res->validate();
    return res;
  }

  if(check(TOKEN_TYPE::GOTO_TOKEN))
  {
    match_adv({TOKEN_TYPE::NUM_INT_TOKEN, TOKEN_TYPE::ID_TOKEN});
    const Label *label = nullptr;

    const auto tkn = m_lexer.getToken();
    const auto id = tkn.id();
    Block *current = m_current_block;

    do
    {
      if (auto l = Block::get(id, current->m_labels); l)
      {
        label = l;
        break;
      }

      current = current->m_parent;
    } while (current);

    if(!label)
    {
      throw SemanticException(
        SEMANTIC_ERROR::SE_MISSING_ID,
        std::format(
            "Semantic error : invalid statement ! {} doesn't correspond to a label !",
            tkn.to_string()),
        token.line(),
        token.column()
      );
    }
    adv();

    auto res = std::make_unique<GotoStatement>(label, token);
    res->validate();
    return res;
  }

  if(check(TOKEN_TYPE::BEGIN_TOKEN))
  {
    return compound_stmt();
  }

  if(check(TOKEN_TYPE::WHILE_TOKEN))
  {
    adv();
    auto cond = gexpression();
    match(TOKEN_TYPE::DO_TOKEN);
    adv();
    auto res = std::make_unique<WhileStatement>(std::move(cond), statement(), token);
    res->validate();
    return res;
  }

  if(check(TOKEN_TYPE::REPEAT_TOKEN))
  {
    std::vector<std::unique_ptr<Statement>> stmts;
    
    do
    {
      adv();
      stmts.push_back(statement());
    } while (check(TOKEN_TYPE::SEMI_TOKEN));
    
    match(TOKEN_TYPE::UNTIL_TOKEN);
    adv();
    auto res = std::make_unique<RepeatStatement>(std::move(stmts), gexpression(), token);
    res->validate();
    return res;
  }

  if(check(TOKEN_TYPE::FOR_TOKEN))
  {
    adv();
    auto v = variable_access(nullptr);
    match(TOKEN_TYPE::ASSIGN_TOKEN);
    adv();
    auto start = constant(m_lexer.getToken());
    match_adv({TOKEN_TYPE::TO_TOKEN, TOKEN_TYPE::DOWNTO_TOKEN});
    bool increasing = check(TOKEN_TYPE::TO_TOKEN);
    adv();
    auto end = constant(m_lexer.getToken());
    match_adv(TOKEN_TYPE::DO_TOKEN);
    adv();

    auto res = std::make_unique<ForStatement>(std::move(v), std::move(start), std::move(end), statement(), increasing, token);
    res->validate();
    return res;
  }

  if(check(TOKEN_TYPE::IF_TOKEN))
  {
    adv();
    auto cond = gexpression();
    match(TOKEN_TYPE::THEN_TOKEN);
    adv();
    auto then = statement();
    std::unique_ptr<Statement> otherwise;

    if(check(TOKEN_TYPE::ELSE_TOKEN))
    {
      adv();
      otherwise = statement();
    }

    auto res = std::make_unique<IfStatement>(std::move(cond), std::move(then), std::move(otherwise), token);
    res->validate();
    return res;
  }

  if(check(TOKEN_TYPE::CASE_TOKEN))
  {
    std::vector<CaseStatement::CaseAlternative> cases;
    adv();
    auto exp = gexpression();
    match(TOKEN_TYPE::OF_TOKEN);
    do
    {
      adv();
      const auto tkn = m_lexer.getToken();
      auto labels = case_label_list();
      match(TOKEN_TYPE::COLON_TOKEN);
      adv();
      cases.emplace_back(std::move(labels), statement(), tkn);
    }while(check(TOKEN_TYPE::SEMI_TOKEN));
    match(TOKEN_TYPE::END_TOKEN);
    adv();
    
    auto res = std::make_unique<CaseStatement>(std::move(exp), std::move(cases), token);
    res->validate();
    return res;
  }

  match({
    TOKEN_TYPE::ID_TOKEN, TOKEN_TYPE::NUM_INT_TOKEN, TOKEN_TYPE::BEGIN_TOKEN, TOKEN_TYPE::GOTO_TOKEN,
    TOKEN_TYPE::WRITE_TOKEN, TOKEN_TYPE::READ_TOKEN, TOKEN_TYPE::IF_TOKEN, TOKEN_TYPE::CASE_TOKEN,
    TOKEN_TYPE::WHILE_TOKEN, TOKEN_TYPE::REPEAT_TOKEN, TOKEN_TYPE::FOR_TOKEN
  });

}

}