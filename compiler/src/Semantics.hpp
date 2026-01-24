#pragma once
#include "Lexer.hpp"
#include <format>
#include <vector>
#include <unordered_map>
#include <memory>
#include <concepts>
#include <variant>
#include <unordered_set>

namespace pascal_compiler
{

enum class SEMANTIC_ERROR
{
  SE_SUBRANGE_TYPES_MISMATCH,
  SE_DUPLICATE_ID,
  SE_INVALID_TYPE,
  SE_INVALID_TOKEN,
  SE_MISSING_ID,
  SE_INVALID_OP,
  SE_INVALID_SUBRANGE,
  SE_INVALID_ENUM
};

class SemanticException{
  SEMANTIC_ERROR m_error;
  const std::string m_msg;
  const size_t m_line;
  const size_t m_col;

public:
  SemanticException(SEMANTIC_ERROR error, std::string&& msg, size_t line, size_t col) : m_error(error), m_msg(std::move(msg)), m_line(line), m_col(col) {}

  SEMANTIC_ERROR getError() const
  {
    return m_error;
  }

  const std::string& getMsg() const
  {
    return m_msg;
  }

  size_t getLine() const
  {
    return m_line;
  }

  size_t getColumn() const
  {
    return m_col;
  }
};

struct Label
{
  std::string_view m_id;
  size_t m_line, m_col;
  // TODO: store Location
};

enum class CONST_CAT : int
{
  CC_INT,
  CC_REAL,
  CC_ENUM,
  CC_STRING,
  CC_BOOL,
  CC_CHAR,
  CC_MAX = CC_CHAR
};

template <typename T, typename... U>
concept is_one_of = (std::is_same_v<T, U> || ... );

template <typename T>
concept NumConstType = is_one_of<T, Int, Real, char, bool>;

static inline const char* CONST_CAT_NAMES[int(CONST_CAT::CC_MAX) + 1] = {
  "Int",
  "Real",
  "Enum",
  "String",
  "Bool",
  "Char"
};

struct Type;
struct Enum;
struct Block;
struct EnumValue;
struct Record;
struct Function;

struct Const
{
  // A const doesn't always have an actual name (when unamed, points to constant)
  std::string_view m_name;
  size_t m_line, m_col;
  std::variant<Int, Real, std::string, char, bool> m_val;
  const Type *m_type; // Stored to tell which enum is used and to check types with variables
  CONST_CAT m_cat;

  template <NumConstType T>
  Const(std::string_view name, size_t line, size_t col, T value, const Block& main_block);
  Const(std::string_view name, size_t line, size_t col, std::string &&value, const Block &main_block);
  Const(std::string_view name, size_t line, size_t col, const EnumValue& value);

};

enum class TYPE_CAT : int{
  TC_BASIC,
  TC_ENUM,
  TC_SUBRANGE,
  TC_ARRAY,
  TC_RECORD,
  TC_FUNCTION,
  TC_MAX = TC_FUNCTION
};

static inline const char* TYPE_CAT_NAMES[int(TYPE_CAT::TC_MAX) + 1] = {
  "TC_BASIC",
  "TC_ENUM",
  "TC_SUBRANGE",
  "TC_ARRAY",
  "TC_RECORD",
  "TC_FUNCTION"
};

//TODO: Types don't always have names, in which case we can store them as a unique_ptr for each scope (or block).
//Remember that when type X = Y, X and Y are the same. Therefore, for named-types we use shared_ptr
struct Type
{
  const std::string_view m_name;
  const size_t m_line;
  const size_t m_col;
  const TYPE_CAT m_type;

  Type(std::string_view name, size_t line, size_t col, TYPE_CAT type) : m_name(name), m_line(line), m_col(col), m_type(type) {}
  std::string to_string() const{
    return std::format("\"{}\" at ({},{})", m_name, m_line, m_col); 
  }

  virtual ~Type() = default;
};

struct Enum : Type
{
  std::unordered_map<std::string_view,Int> m_values;
  Enum(const std::string_view &name, size_t line, size_t col) : Type(name, line, col, TYPE_CAT::TC_ENUM) {}

  // Automatically inserts EnumValue object to block
  // Use only when Enum is a pointer (uses this internally). Clearly the Enum should outlive EnumValue, which exists to facilitate search 
  void insert(const Lexeme&, Block&);
};

class EnumValue {
  friend struct Enum;
public:
  const Enum* m_type;
  const std::string_view m_id;
  const Int m_value;
  const size_t m_line;
  const size_t m_col;
private:
  EnumValue(Enum* type, const Lexeme& token, Int value) : m_type(type), m_id(token.m_id), m_value(value), m_line(token.m_line), m_col(token.m_col) {} 
};

struct Subrange : Type
{
  Const m_beg, m_end; // Must contain the same type
  Subrange(
    const std::string_view& name, size_t line, size_t col,
    Const&& beg, Const&& end
  );
};

struct Array : Type
{

  std::vector<const Type*> m_itypes;  // index types
  const Type* m_etype;                // element type

  Array(const std::string_view &name, size_t line, size_t col, std::vector<const Type*>&& types, const Type* etype) : 
  Type(name, line, col, TYPE_CAT::TC_ARRAY), m_itypes(std::move(types)), m_etype(etype) {}
};

struct Var
{
  std::string_view m_name;
  size_t m_line, m_col;
  Type *m_type;
};

struct Record : Type
{

  std::unordered_map<std::string_view,Var> m_members;
  std::unordered_map<std::string_view,Record*> m_variants;

  Record(const std::string_view &name, size_t line, size_t col) : Type(name, line, col, TYPE_CAT::TC_RECORD) {}
};

struct FunctionType : Type
{

  std::vector<Var> m_args;
  Type* m_ret_type; // Null for procedures

  FunctionType(const std::string_view &name, size_t line, size_t col, Type* ret_type) : Type(name, line, col, TYPE_CAT::TC_FUNCTION), m_ret_type(ret_type) {}
};

template <typename T>
concept BlockMemberType = is_one_of<T, Label, Const, std::shared_ptr<Type>, Var, Function, EnumValue>;

struct Block
{
  Block *m_parent;
  std::unordered_map<std::string_view, Label> m_labels;
  std::unordered_map<std::string_view, Const> m_consts;
  std::unordered_map<std::string_view, std::shared_ptr<Type>> m_types;
  std::unordered_map<std::string_view, Var> m_vars;
  std::unordered_map<std::string_view, Function> m_funcs;
  std::unordered_map<std::string_view, EnumValue> m_enums_vals;

  std::vector<std::unique_ptr<Type>> m_unamed_types;

  Block(Block *parent = nullptr) : m_parent(parent) {}

  static void init_main_block(Block&);

  // We plan to support variable shadowing including constants..., except for types 
  template <BlockMemberType T>
  static void check_used_id(const Lexeme &, const std::unordered_map<std::string_view, T> &);
  void check_used_id(const Lexeme&);

  template <BlockMemberType T>
  static std::conditional_t<std::is_same_v<T, std::shared_ptr<Type>>, const Type*, const T*> 
  get(std::string_view, const std::unordered_map<std::string_view, T> &);
};

struct Function
{
  FunctionType* m_type;
  std::unique_ptr<Block> m_block;
};

}