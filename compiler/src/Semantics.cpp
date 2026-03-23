#include "Semantics.hpp"
#include "Ast.hpp"
#include "Lexer.hpp"

namespace pascal_compiler
{

void Block::init_main_block(Block& block){
  auto &types = block.m_types;
  for (auto type : {CONST_CAT::CC_INT, CONST_CAT::CC_REAL, CONST_CAT::CC_CHAR, CONST_CAT::CC_CONST_STRING, CONST_CAT::CC_BOOL})
  {
    auto name = CONST_CAT_NAMES[int(type)];
    types.insert(std::make_pair(name,std::make_shared<Type>(name, 0, 0, TYPE_CAT::TC_BASIC)));
  }
}

Const::Const(const Lexeme& token, const EnumValue& value) : ID(token),
    m_type(static_cast<const Type*>(value.type())),
    m_val(value.int_value()),
    m_cat(CONST_CAT::CC_ENUM)
{
}

Const::Const(const Lexeme &token, std::string &&value, const Block &block) : ID(token),
    m_type(block.m_types.at(CONST_CAT_NAMES[int(CONST_CAT::CC_CONST_STRING)]).get()),
    m_val(std::move(value)),
    m_cat(CONST_CAT::CC_CONST_STRING)
{
}

template Const::Const(const Lexeme &token, Int value, const Block& block);
template Const::Const(const Lexeme &token, Real value, const Block& block);
template Const::Const(const Lexeme &token, char value, const Block& block);
template Const::Const(const Lexeme &token, bool value, const Block& block);

template <NumConstType T>
Const::Const(const Lexeme &token, T value, const Block& block) : ID(token),
    m_type(block.m_types.at(CONST_CAT_NAMES[int(
      (std::is_same_v<T, Int> ? CONST_CAT::CC_INT :
        (std::is_same_v<T, Real> ? CONST_CAT::CC_REAL :
          (std::is_same_v<T, char>) ? CONST_CAT::CC_CHAR : CONST_CAT::CC_BOOL
        )
      )
    )]).get()),
    m_val(value),
    m_cat(std::is_same_v<T, Int> ? CONST_CAT::CC_INT :
        (std::is_same_v<T, Real> ? CONST_CAT::CC_REAL :
          (std::is_same_v<T, char>) ? CONST_CAT::CC_CHAR : CONST_CAT::CC_BOOL
        )
      )
{
}

template <typename T>
T Const::get() const {
  return std::get<T>(m_val);
}

Int Const::asInt() const {
  return std::visit([](auto&& val) -> Int {
    using T = std::decay_t<decltype(val)>;
    if constexpr (std::is_same_v<T, Int>) return val;
    else if constexpr (std::is_same_v<T, char>) return static_cast<Int>(val);
    else if constexpr (std::is_same_v<T, bool>) return static_cast<Int>(val);
    else throw std::bad_variant_access();
  }, m_val);
}

const std::variant<Int, Real, std::string, char, bool>& Const::value() const { return m_val; }

Const::Const(const Lexeme& token, const Const& other) 
  : ID(token), m_type(other.type()), m_cat(other.category()), m_val(other.value()) {}

Const Const::withSign(const Const& original, int sign, const Lexeme& new_token) {
  Const res(new_token, original);

  if (original.category() == CONST_CAT::CC_INT) {
    res.m_val = original.get<Int>() * sign;
  } else if (original.category() == CONST_CAT::CC_REAL) {
    res.m_val = original.get<Real>() * sign;
  }

  return res;
}

Subrange::Subrange(const Lexeme &token, Const &&beg, Const &&end)
    : Type(token, TYPE_CAT::TC_SUBRANGE) {
  if(beg.type() != end.type()){
    throw SemanticException(
      SEMANTIC_ERROR::SE_SUBRANGE_TYPES_MISMATCH,
      std::format(
        "Semantic error: Subrange type must be made using 2 constants of the same type. Found {} and {} !",
        beg.type()->to_string(), end.type()->to_string()
      ),
      beg.line(),
      beg.column()
    );
  }

  m_utype = beg.type();

  switch(beg.category()){
    case CONST_CAT::CC_BOOL:
      m_beg = beg.get<bool>();
      m_end = end.get<bool>();
      break;
    case CONST_CAT::CC_CHAR:
      m_beg = beg.get<char>();
      m_end = end.get<char>();
      break;
    case CONST_CAT::CC_ENUM:
    case CONST_CAT::CC_INT:
      m_beg = beg.get<Int>();
      m_end = end.get<Int>();
      break;
    default:
      throw SemanticException(
        SEMANTIC_ERROR::SE_SUBRANGE_TYPES_MISMATCH,
        std::format(
          "Semantic error: Subrange type \"{}\" can be made only using chars, enums or ints (constants) !",
          beg.type()->to_string()
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
        beg.type()->to_string(), m_beg, m_end
      ),
      m_line,
      m_col
    );
  }

  m_cat = beg.category();
}

void Enum::insert(const Lexeme& token, Block& block){
  if(token.type() != TOKEN_TYPE::ID_TOKEN){
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_TOKEN,
      std::format(
        "Semantic error: enum type '{}' values are identifiers. Found '{}' at ({},{}) !",
        to_string(), token.id(), token.line(), token.column()
      ),
      token.line(),
      token.column()
    );
  }
  if(m_values.contains(token.id())){
    throw SemanticException(
      SEMANTIC_ERROR::SE_DUPLICATE_ID,
      std::format(
        "Semantic error: enum type '{}' already contains value '{}'. Found earlier at ({},{}) !!",
        to_string(), token.id(), token.line(), token.column()
      ),
      token.line(),
      token.column()
    );
  }
  const Int value = Int(m_values.size());
  m_values[token.id()] = value;
  block.m_enums_vals.emplace(std::make_pair(token.id(), EnumValue(this, token, value)));
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
size_t getInfoLine(const T& t){
  if constexpr (is_one_of<T, Label, Const, Var, EnumValue>){
    return t.line();
  }
  else if constexpr(std::is_same_v<T, std::shared_ptr<Type>>){
    return t.get()->line();
  }
  else if constexpr(std::is_same_v<T, Function>){
    return t.type()->line();
  }
}

template <BlockMemberType T>
size_t getInfoCol(const T& t){
  if constexpr (is_one_of<T, Label, Const, Var, EnumValue>){
    return t.column();
  }
  else if constexpr(std::is_same_v<T, std::shared_ptr<Type>>){
    return t.get()->column();
  }
  else if constexpr(std::is_same_v<T, Function>){
    return t.type()->column();
  }
}

template <BlockMemberType T>
void Block::check_used_id(const Lexeme &token, const std::unordered_map<std::string_view, T> &map)
{

  if(auto it = map.find(token.id()); it != map.end())
  {

      throw SemanticException(
        SEMANTIC_ERROR::SE_DUPLICATE_ID,
        std::format
        (
          "Semantic error: duplicate id '{}' found at ({},{}) and ({},{}) !",
          token.id(),
          token.line(),
          token.column(),
          getInfoLine(it->second),
          getInfoCol(it->second)
        ),
        token.line(),
        token.column()
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

void Record::check_duplicate_id(const Lexeme& rec, const ID& var){
  if(m_members.contains(var.id()))
    throw SemanticException(
      SEMANTIC_ERROR::SE_DUPLICATE_ID,
      std::format(
        "Semantic error: duplicate record member id '{}' found at ({},{}) and ({},{}) !",
        var.id(),
        var.line(),
        var.column(),
        rec.line(),
        rec.column()),
      var.line(),
      var.column()
    );
}

void Record::add_attribute(const Lexeme& token, const Var& attribute)
{
  check_duplicate_id(token, attribute);
  m_members.emplace(attribute.id(), attribute);
}

const Var* Record::add_attribute(const Var& attribute)
{
  return &m_members.emplace(attribute.id(), attribute).first->second;
}


Function::Function(const Lexeme& token, const FunctionType *func_type, Block *b)
  : ID(token), m_type(func_type), block(b) {}
};