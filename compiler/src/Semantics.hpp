#include "Lexer.hpp"
#include <format>
#include <vector>
#include <unordered_map>
#include <memory>
#include <variant>
#include <concepts>

namespace pascal_compiler
{

enum class SEMANTIC_ERROR
{
  SE_SUBRANGE_TYPES_MISMATCH,
  SE_DUPLICATE_ID
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

enum class CONST_CAT
{
  CC_INT,
  CC_REAL,
  CC_ENUM,
  CC_STRING,
  CC_CHAR
};

struct Type;

template <typename T>
concept ConstType = std::is_same_v<T,Int> || std::is_same_v<T,Real> || std::is_same_v<T,std::string_view> || std::is_same_v<T,char>;


struct Const
{
  
  std::string_view m_name;
  size_t m_line, m_col;
  std::variant<Int,Real,std::string_view,char> m_val;
  Type *m_type;
  CONST_CAT m_cat;

  template <ConstType T>
  Const(std::string_view name, size_t line, size_t col, T value, Type *type) : m_name(name), m_line(line), m_col(col), m_type(type)
  {
    
  }

};
//TODO: Types don't always have names, in which case we can create one instantaneously using a counter (since IDs cannot start with numbers). For now we pick ""
//Remember that when type X = Y, X and Y are the same (same pointer)
struct Type
{
  const std::string_view m_name;
  const size_t m_line;
  const size_t m_col;

  Type(const std::string_view& name, size_t line, size_t col) : m_name(name), m_line(line), m_col(col) {}
  std::string to_string(){
    return std::format("\"{}\" at ({},{})", m_name, m_line, m_col); 
  }

  virtual ~Type() = default;
};

struct Enum : Type
{
  std::vector<std::string> m_values;
  Enum(const std::string_view &name, size_t line, size_t col) : Type(name, line, col) {}
};

struct Subrange : Type
{
  Const m_beg, m_end; // Must contain the same type
  Subrange(
    const std::string_view& name, size_t line, size_t col,
    Const&& beg, Const&& end 
    ) : Type(name, line, col), m_beg(std::move(beg)), m_end(std::move(end)) {
    if(m_beg.m_cat != m_end.m_cat || typeid(m_beg.m_type) != typeid(m_end.m_type)){
      throw SemanticException(
        SEMANTIC_ERROR::SE_SUBRANGE_TYPES_MISMATCH,
        std::format(
          "Semantic error: Subrange type must be made using 2 constants of the same type. Found {} and {} !",
          m_beg.m_type->to_string(), m_end.m_type->to_string()
        ),
        m_beg.m_line,
        m_beg.m_col
      );
    }
    switch(m_beg.m_cat){
      case CONST_CAT::CC_CHAR:
      case CONST_CAT::CC_ENUM:
      case CONST_CAT::CC_INT:
      break;
      default:
        throw SemanticException(
          SEMANTIC_ERROR::SE_SUBRANGE_TYPES_MISMATCH,
          std::format(
            "Semantic error: Subrange type \"{}\" can be made only using chars, enums or ints (constants) !",
            m_beg.m_type->to_string()
          ),
          m_line,
          m_col
        );
    }
  }
};

struct Array : Type
{

  size_t m_size;
  Type* m_etype;

  Array(const std::string_view &name, size_t line, size_t col) : Type(name, line, col) {}
};

struct Record;

struct Var
{
  std::string_view m_name;
  Type *m_type;
};

struct Record : Type
{

  std::unordered_map<std::string_view,Var> m_members;
  std::unordered_map<std::string_view,Record*> m_variants;

  Record(const std::string_view &name, size_t line, size_t col) : Type(name, line, col) {}
};

struct Procedure;
struct Function;

struct Block
{
  Block *m_parent;
  std::unordered_map<std::string_view, Label> m_labels;
  std::unordered_map<std::string_view, Const> m_consts;
  std::unordered_map<std::string, std::unique_ptr<Type>> m_types;
  std::unordered_map<std::string_view, Var> m_vars;
  std::unordered_map<std::string_view, Procedure> m_procs;
  std::unordered_map<std::string_view, Function> m_funcs;

  Block(Block *parent = nullptr) : m_parent(parent) {}
};

struct Procedure
{
  std::string_view m_name;
  std::vector<Var> m_args;
  std::unique_ptr<Block> m_block;
};

struct Function
{
  Procedure m_proc;
  Type *m_type;
};

}