#pragma once
#include "Lexer.hpp"
#include <format>
#include <vector>
#include <unordered_map>
#include <memory>
#include <concepts>
#include <variant>
#include <unordered_set>
#include <exception>

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
  SE_INVALID_ENUM,
  SE_AMBIGUOUS_TAG_VAR,
  SE_INVALID_FIELD_NAME,
  SE_INVALID_INDEX,
  SE_INVALID_ASSIGN,
  SE_INVALID_CALL,
  SE_INVALID_COND
};

class SemanticException : public std::exception {
  SEMANTIC_ERROR m_error;
  const std::string m_msg;
  const size_t m_line;
  const size_t m_col;

public:
  SemanticException(SEMANTIC_ERROR error, std::string&& msg, size_t line, size_t col) : m_error(error), m_msg(std::move(msg)), m_line(line), m_col(col) {}

  const char* what() const noexcept override { return m_msg.c_str(); }

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

struct ID{
  const std::string_view m_id;
  const size_t m_line, m_col;
  
  ID(std::string_view id, size_t line, size_t col)
    : m_id(id), m_line(line), m_col(col) {}
  
  ID(const Lexeme& token) 
    : m_id(token.id()), m_line(token.line()), m_col(token.column()) {}


  std::string_view id() const { return m_id; }
  size_t line() const { return m_line; }
  size_t column() const { return m_col; }
};

struct Label : ID
{
  Label(const Lexeme& token) : ID(token) {}
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

class Const : public ID
{
  std::variant<Int, Real, std::string, char, bool> m_val;
  const Type *m_type;
  const CONST_CAT m_cat;

public:
  template <NumConstType T>
  Const(const Lexeme& token, T value, const Block& main_block);
  Const(const Lexeme& token, std::string &&value, const Block &main_block);
  Const(const Lexeme& token, const EnumValue& value);
  Const(const Lexeme& token, const Const& other);

  const Type* type() const { return m_type; }
  CONST_CAT category() const { return m_cat; }

  template <typename T>
  T get() const;
  Int asInt() const;

  const std::variant<Int, Real, std::string, char, bool>& value() const;

  static Const withSign(const Const& original, int sign, const Lexeme& new_token);

  std::string to_string() const
  {
    return std::format("\"{}\" at ({},{})", m_id, m_line, m_col);
  }
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
struct Type : ID
{
  const TYPE_CAT m_type;

  Type(std::string_view name, size_t line, size_t col, TYPE_CAT type) : ID(name, line, col), m_type(type) {}
  Type(const Lexeme& token, TYPE_CAT type_cat) : ID(token), m_type(type_cat) {}
  std::string to_string() const{
    return std::format("\"{}\" at ({},{})", m_id, m_line, m_col); 
  }

  virtual ~Type() = default;

  virtual const Type* get_underlying_type() const { return this;};
};

class Enum : public Type
{

private:
  std::unordered_map<std::string_view,Int> m_values;

public:
  Enum(const Lexeme& token) : Type(token, TYPE_CAT::TC_ENUM) {}
  const std::unordered_map<std::string_view,Int>& values() const { return m_values;}; 
  // Automatically inserts EnumValue object to block
  // Use only when Enum is a pointer (uses this internally). Clearly the Enum should outlive EnumValue, which exists to facilitate search 
  void insert(const Lexeme&, Block&);
};

class EnumValue : public ID{

  friend struct Enum;

private:
  const Enum* m_type;
  const Int m_value;

  EnumValue(Enum* type, const Lexeme& token, Int value) : m_type(type), m_value(value), ID(token) {} 

public:
  Int int_value() const { return m_value; }
  const Enum* type() const { return m_type; }
};

class Subrange : public Type
{
  Int m_beg, m_end; // Must contain the same type
  CONST_CAT m_cat;
  const Type* m_utype; // Underlying type

public:
  Subrange(const Lexeme& token, Const&& beg, Const&& end);

  Int start() const { return m_beg; }
  Int end() const { return m_end; }
  const CONST_CAT category() const { return m_cat; }

  const Type *get_underlying_type() const override { return m_utype; };
};

class Array : public Type
{

  std::vector<const Type*> m_itypes;  // index types
  const Type* m_etype;                // element type

public:

  Array(const Lexeme& token, std::vector<const Type*>&& types, const Type* etype) 
    : Type(token, TYPE_CAT::TC_ARRAY), m_itypes(std::move(types)), m_etype(etype) {}

  const std::vector<const Type*>& index_types() const { return m_itypes; }
  const Type* element_type() const { return m_etype; }

};

class Var : public ID
{
  const Type *m_type;

public:

  Var(const Lexeme& token, const Type* type) 
    : ID(token), m_type(type) {}
  Var(std::string_view id, size_t line, size_t col, const Type *type)
      : ID(id, line, col), m_type(type) {}

  std::string to_string() const
  {
    return std::format("\"{}\" at ({},{})", m_id, m_line, m_col);
  }

  const Type* type() const { return m_type; }
};

class Record : public Type
{

  std::unordered_map<std::string_view,Var> m_members;

  void check_duplicate_id(const Lexeme &rec, const ID &var);

public:

  Record(const Lexeme& token)
    : Type(token, TYPE_CAT::TC_RECORD) {}

  const std::unordered_map<std::string_view,Var>& attributes() const { return m_members; }
  void add_attribute(const Lexeme&, const Var&);
  const Var* add_attribute(const Var&);

};

class Arg : public Var
{
  bool m_ref;

public:

  Arg(bool by_ref, const Lexeme& token, const Type* type) 
    : Var(token, type), m_ref(by_ref) {}
    
  const bool is_ref() const { return m_ref; }
};

struct FunctionType : Type
{

  std::vector<Arg> m_args;
  const Type* m_ret_type; // Null for procedures

  FunctionType(const std::string_view &name, size_t line, size_t col, Type* ret_type) : Type(name, line, col, TYPE_CAT::TC_FUNCTION), m_ret_type(ret_type) {}
};

template <typename T>
concept BlockMemberType = is_one_of<T, Label, Const, std::shared_ptr<Type>, Var, Function, EnumValue>;

struct CompoundStatement;

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

  std::unique_ptr<CompoundStatement> body;

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
  std::string_view m_id;
  const FunctionType* m_type;
  std::unique_ptr<Block> block;
  Function(std::string_view , const FunctionType*, Block*);
};

}