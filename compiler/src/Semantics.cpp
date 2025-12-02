#include "Semantics.hpp"

namespace pascal_compiler
{

Const::Const(std::string_view name, size_t line, size_t col, std::string_view value, Enum *type) : m_name(name), m_line(line), m_col(col), m_type(type) 
{
  if(!m_type) 
    throw SemanticException
    (
      SEMANTIC_ERROR::SE_INVALID_TYPE,
      std::format
      (
        "Semantic error: the type of the constant '{}' at ({},{}) is null ?",
        m_name,m_line,m_col  
      ),
      m_line,m_col
    );
  
  const auto it = type->m_values.find(value);
  if(it == type->m_values.end())
  {
    throw SemanticException
      (
        SEMANTIC_ERROR::SE_INVALID_TYPE,
        std::format
        (
          "Semantic error: the type of the constant '{}' at ({},{}) is supposedly the enum {}, but {} isn't one of it's values !",
          m_name, m_line, m_col, type->to_string(), value 
        ),
        m_line, m_col
      );
  }

  m_ival = it->second;
  m_cat = CONST_CAT::CC_ENUM;
}

template Const::Const(std::string_view name, size_t line, size_t col, Int value, Type *type);
template Const::Const(std::string_view name, size_t line, size_t col, Real value, Type *type);
template Const::Const(std::string_view name, size_t line, size_t col, char value, Type *type);
template Const::Const(std::string_view name, size_t line, size_t col, std::string_view value, Type *type);

template <ConstType T>
Const::Const(std::string_view name, size_t line, size_t col, T value, Type *type) : m_name(name), m_line(line), m_col(col), m_type(type)
{
  if(!m_type) 
    throw SemanticException
    (
      SEMANTIC_ERROR::SE_INVALID_TYPE,
      std::format
      (
        "Semantic error: the type of the constant '{}' at ({},{}) is null ?",
        m_name,m_line,m_col  
      ),
      m_line,m_col
    );
  
  if constexpr (std::is_same_v<T,Int>)
  {
    if (m_type->m_name != CONST_CAT_NAMES[int(CONST_CAT::CC_INT)])
    {
      throw SemanticException
      (
        SEMANTIC_ERROR::SE_INVALID_TYPE,
        std::format
        (
          "Semantic error: the type of the constant '{}' with value '{}' at ({},{}) should be '{}', not Int !",
          m_name, value, m_line, m_col, m_type->m_name
        ),
        m_line, m_col
      );
    }
    m_ival = value;
    m_cat = CONST_CAT::CC_INT;
  }

  else if constexpr (std::is_same_v<T,Real>)
  {
    if (m_type->m_name != CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)])
    {
      throw SemanticException
      (
        SEMANTIC_ERROR::SE_INVALID_TYPE,
        std::format
        (
          "Semantic error: the type of the constant '{}' with value '{}' at ({},{}) should be '{}', not Real !",
          m_name, value, m_line, m_col, m_type->m_name
        ),
        m_line, m_col
      );
    }
    m_dval = value;
    m_cat = CONST_CAT::CC_REAL;
  }

  else if constexpr (std::is_same_v<T,char>)
  {
    if (m_type->m_name != CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)])
    {
      throw SemanticException
      (
        SEMANTIC_ERROR::SE_INVALID_TYPE,
        std::format
        (
          "Semantic error: the type of the constant '{}' with value '{}' at ({},{}) should be '{}', not Char !",
          m_name, value, m_line, m_col, m_type->m_name
        ),
        m_line, m_col
      );
    }
    m_cval = value;
    m_cat = CONST_CAT::CC_CHAR;
  }

  else if constexpr (std::is_same_v<T,std::string_view>)
  {
    if (m_type->m_name != CONST_CAT_NAMES[int(CONST_CAT::CC_STRING)])
    {
      throw SemanticException
      (
        SEMANTIC_ERROR::SE_INVALID_TYPE,
        std::format
        (
          "Semantic error: the type of the constant '{}' with value '{}' at ({},{}) should be '{}', not String !",
          m_name, value, m_line, m_col, m_type->m_name
        ),
        m_line, m_col
      );
    }
    m_sval = value;
    m_cat = CONST_CAT::CC_STRING;
  }
}

template Int Const::get();
template Real Const::get();
template char Const::get();
template std::string_view Const::get();

template <ConstType T>
T Const::get(){
  if constexpr (std::is_same_v<T,Int>)
  {
    if (m_cat != CONST_CAT::CC_INT && m_cat != CONST_CAT::CC_ENUM)
    {
      throw SemanticException
      (
        SEMANTIC_ERROR::SE_INVALID_TYPE,
        std::format
        (
          "Semantic error: the constant '{}' with value '{}' at ({},{}) is neither an Int nor an Enum !",
          m_name, m_ival, m_line, m_col
        ),
        m_line, m_col
      );
    }
    return m_ival;
  }

  else if constexpr (std::is_same_v<T,Real>)
  {
    if (m_cat != CONST_CAT::CC_REAL)
    {
      throw SemanticException
      (
        SEMANTIC_ERROR::SE_INVALID_TYPE,
        std::format
        (
          "Semantic error: the constant '{}' with value '{}' at ({},{}) is not a Real !",
          m_name, m_dval, m_line, m_col
        ),
        m_line, m_col
      );
    }
    return m_dval;
  }

  else if constexpr (std::is_same_v<T,char>)
  {
    if (m_cat != CONST_CAT::CC_CHAR)
    {
      throw SemanticException
      (
        SEMANTIC_ERROR::SE_INVALID_TYPE,
        std::format
        (
          "Semantic error: the constant '{}' with value '{}' at ({},{}) is not a Char !",
          m_name, m_cval, m_line, m_col
        ),
        m_line, m_col
      );
    }
    return m_cval;
  }

  else if constexpr (std::is_same_v<T,std::string_view>)
  {
    if (m_cat != CONST_CAT::CC_STRING)
    {
      throw SemanticException
      (
        SEMANTIC_ERROR::SE_INVALID_TYPE,
        std::format
        (
          "Semantic error: the constant '{}' with value '{}' at ({},{}) is not a String !",
          m_name, m_sval, m_line, m_col
        ),
        m_line, m_col
      );
    }
    return m_sval;
  }
}

Subrange::Subrange(
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