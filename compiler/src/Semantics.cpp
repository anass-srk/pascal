#include "Semantics.hpp"

namespace pascal_compiler
{

void Block::init_main_block(Block& block){
  auto &types = block.m_types;
  for (auto type : {CONST_CAT::CC_INT, CONST_CAT::CC_REAL, CONST_CAT::CC_CHAR, CONST_CAT::CC_STRING, CONST_CAT::CC_BOOL})
  {
    auto name = CONST_CAT_NAMES[int(type)];
    types[name] = std::make_shared<Type>(name, 0, 0, TYPE_CAT::TC_BASIC);
  }
}

Const::Const(std::string_view name, size_t line, size_t col, const EnumValue& value) : m_name(name), m_line(line), m_col(col),
  m_type(static_cast<const Type*>(value.m_type)) 
{
  m_val = value.m_value;
  m_cat = CONST_CAT::CC_ENUM;
}

Const::Const(std::string_view name, size_t line, size_t col, std::string&& value, const Block& block) : m_name(name), m_line(line), m_col(col), 
  m_type(block.m_types.at(CONST_CAT_NAMES[int(CONST_CAT::CC_STRING)]).get())
{
  m_val = std::move(value);
  m_cat = CONST_CAT::CC_STRING;
}

template Const::Const(std::string_view name, size_t line, size_t col, Int value, const Block& block);
template Const::Const(std::string_view name, size_t line, size_t col, Real value, const Block& block);
template Const::Const(std::string_view name, size_t line, size_t col, char value, const Block& block);
template Const::Const(std::string_view name, size_t line, size_t col, bool value, const Block& block);

template <NumConstType T>
Const::Const(std::string_view name, size_t line, size_t col, T value, const Block& block) : m_name(name), m_line(line), m_col(col),
  m_type(block.m_types.at(CONST_CAT_NAMES[int(
    (std::is_same_v<T, Int> ? CONST_CAT::CC_INT : 
      (std::is_same_v<T, Real> ? CONST_CAT::CC_REAL : 
        (std::is_same_v<T, char>) ? CONST_CAT::CC_CHAR : CONST_CAT::CC_BOOL
      )
    )
  )]).get())
{

  m_val = value;

  if constexpr (std::is_same_v<T, Int>)
  {
    m_cat = CONST_CAT::CC_INT;
  }

  else if constexpr (std::is_same_v<T, Real>)
  {
    m_cat = CONST_CAT::CC_REAL;
  }

  else if constexpr (std::is_same_v<T, char>)
  {
    m_cat = CONST_CAT::CC_CHAR;
  }

  else if constexpr (std::is_same_v<T, bool>)
  {
    m_cat = CONST_CAT::CC_BOOL;
  }
}

Subrange::Subrange(
  const std::string_view& name, size_t line, size_t col,
  Const&& beg, Const&& end 
) : Type(name, line, col, TYPE_CAT::TC_SUBRANGE) {
  if(beg.m_type != end.m_type){
    throw SemanticException(
      SEMANTIC_ERROR::SE_SUBRANGE_TYPES_MISMATCH,
      std::format(
        "Semantic error: Subrange type must be made using 2 constants of the same type. Found {} and {} !",
        beg.m_type->to_string(), end.m_type->to_string()
      ),
      beg.m_line,
      beg.m_col
    );
  }

  switch(beg.m_cat){
    case CONST_CAT::CC_CHAR:
      m_beg = std::get<char>(beg.m_val);
      m_end = std::get<char>(end.m_val);
      break;
    case CONST_CAT::CC_ENUM:
    case CONST_CAT::CC_INT:
      m_beg = std::get<Int>(beg.m_val);
      m_end = std::get<Int>(end.m_val);
      break;
    break;
    default:
      throw SemanticException(
        SEMANTIC_ERROR::SE_SUBRANGE_TYPES_MISMATCH,
        std::format(
          "Semantic error: Subrange type \"{}\" can be made only using chars, enums or ints (constants) !",
          beg.m_type->to_string()
        ),
        m_line,
        m_col
      );
  }

  if (m_end <= m_beg){
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_SUBRANGE,
      std::format(
        "Semantic error: Subrange type \"{}\" 's beginning ({}) is greater or equals than its end ({}) !",
        beg.m_type->to_string(), m_beg, m_end
      ),
      m_line,
      m_col
    );
  }

  m_cat = beg.m_cat;

}

void Enum::insert(const Lexeme& token, Block& block){
  if(token.m_type != TOKEN_TYPE::ID_TOKEN){
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_TOKEN,
      std::format(
        "Semantic error: enum type '{}' values are identifiers. Found '{}' at ({},{}) !",
        to_string(), token.m_id, token.m_line, token.m_col
      ),
      token.m_line,
      token.m_col
    );
  }
  if(m_values.contains(token.m_id)){
    throw SemanticException(
      SEMANTIC_ERROR::SE_DUPLICATE_ID,
      std::format(
        "Semantic error: enum type '{}' already contains value '{}'. Found earlier at ({},{}) !!",
        to_string(), token.m_id, token.m_line, token.m_col
      ),
      token.m_line,
      token.m_col
    );
  }
  const Int value = Int(m_values.size());
  m_values[token.m_id] = value;
  block.m_enums_vals.emplace(std::make_pair(token.m_id, EnumValue(this, token, value)));
}

template <BlockMemberType T>
inline const auto& getInfo(const T& t){
  if constexpr (is_one_of<T, Label, Const, Var, EnumValue>){
    return t;
  }
  else if constexpr(std::is_same_v<T, std::shared_ptr<Type>>){
    return *t.get();
  }
  else if constexpr(std::is_same_v<T, Function>){
    return *t.m_type;
  }
}

template <BlockMemberType T>
void Block::check_used_id(const Lexeme &token, const std::unordered_map<std::string_view, T> &map)
{

  if(auto it = map.find(token.m_id); it != map.end())
  {

    throw SemanticException(
      SEMANTIC_ERROR::SE_DUPLICATE_ID,
      std::format
      (
        "Semantic error: duplicate id '{}' found at ({},{}) and ({},{}) !",
        token.m_id,
        token.m_line,
        token.m_col,
        getInfo(it->second).m_line,
        getInfo(it->second).m_col
      ),
      token.m_line,
      token.m_col
    );
  }

}
//Types cannot be shadowed as opposed to constants, vars...
void Block::check_used_id(const Lexeme& token)
{
  check_used_id(token, m_labels);
  check_used_id(token, m_consts);
  check_used_id(token, m_types);
  check_used_id(token, m_vars);
  check_used_id(token, m_funcs);
  check_used_id(token, m_enums_vals);
  Block *prev = m_parent;
  while(prev){
    check_used_id(token, prev->m_types);
    prev = prev->m_parent;
  }
}

template <BlockMemberType T>
std::conditional_t<std::is_same_v<T, std::shared_ptr<Type>>, const Type*, const T*>
Block::get(std::string_view id, const std::unordered_map<std::string_view, T> &map)
{
  const auto it = map.find(id);
  if constexpr (std::is_same_v<T, std::shared_ptr<Type>>){
    if(it != map.end()) return it->second.get();
  }
  else {
    if(it != map.end()) return &(it->second);
  }
  return nullptr;
}

template const Type* Block::get(std::string_view, const std::unordered_map<std::string_view, std::shared_ptr<Type>>&);
template const Label* Block::get(std::string_view, const std::unordered_map<std::string_view, Label>&);
template const Const* Block::get(std::string_view, const std::unordered_map<std::string_view, Const>&);
template const Var* Block::get(std::string_view, const std::unordered_map<std::string_view, Var>&);
template const Function* Block::get(std::string_view, const std::unordered_map<std::string_view, Function>&);
template const EnumValue* Block::get(std::string_view, const std::unordered_map<std::string_view, EnumValue> &);

void Record::check_duplicate_id(const Lexeme& rec, const Lexeme& name){
  if(m_members.contains(name.m_id))
    throw SemanticException(
      SEMANTIC_ERROR::SE_DUPLICATE_ID,
      std::format(
        "Semantic error: duplicate record member id '{}' found at ({},{}) and ({},{}) !",
        name.m_id,
        name.m_line,
        name.m_col,
        rec.m_line,
        rec.m_col),
      name.m_line,
      name.m_col
    );
  
  for(auto [_,record] : m_variants){
    record->check_duplicate_id(rec, name);
  }
}

};