#ifndef LEXER_HXX
#define LEXER_HXX

#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <concepts>
#include <format>
#include <exception>

namespace pascal_compiler
{

enum class TOKEN_TYPE : int
{
  AND_TOKEN,
  ARRAY_TOKEN,
  BEGIN_TOKEN,
  CASE_TOKEN,
  CONST_TOKEN,
  DIV_TOKEN,
  DO_TOKEN,
  DOWNTO_TOKEN,
  ELSE_TOKEN,
  END_TOKEN,
  FOR_TOKEN,
  FUNCTION_TOKEN,
  GOTO_TOKEN,
  IF_TOKEN,
  IN_TOKEN,
  LABEL_TOKEN,
  MOD_TOKEN,
  NIL_TOKEN,
  NOT_TOKEN,
  OF_TOKEN,
  OR_TOKEN,
  PROCEDURE_TOKEN,
  PROGRAM_TOKEN,
  RECORD_TOKEN,
  REPEAT_TOKEN,
  SET_TOKEN,
  THEN_TOKEN,
  TO_TOKEN,
  TYPE_TOKEN,
  UNTIL_TOKEN,
  VAR_TOKEN,
  WHILE_TOKEN,
  WITH_TOKEN,
  PLUS_TOKEN,
  MINUS_TOKEN,
  STAR_TOKEN,
  SLASH_TOKEN,
  ASSIGN_TOKEN,
  COMMA_TOKEN,
  SEMI_TOKEN,
  COLON_TOKEN,
  EQ_TOKEN,
  NEQ_TOKEN,
  LT_TOKEN,
  LE_TOKEN,
  GT_TOKEN,
  GE_TOKEN,
  LP_TOKEN,
  RP_TOKEN,
  LB_TOKEN,
  RB_TOKEN,
  READ_TOKEN,
  WRITE_TOKEN,
  POINTER_TOKEN,
  AT_TOKEN,
  DOT_TOKEN,
  RANGE_TOKEN,
  TRUE_TOKEN,
  FALSE_TOKEN,
  ID_TOKEN,
  STRING_LITERAL_TOKEN,
  NUM_INT_TOKEN,
  NUM_REAL_TOKEN,
  CHAR_LITERAL_TOKEN,
  EOF_TOKEN,
  TOKEN_TYPE_MAX = EOF_TOKEN
};

using Int = long long;
using Real = double;
using Uint = unsigned long long;

extern const char* TOKEN_NAMES[size_t(TOKEN_TYPE::TOKEN_TYPE_MAX) + 1];

enum class LEXER_ERROR
{
  LE_INVALID_CHAR,
  LE_INVALID_NUMBER,
  LE_INVALID_STRING,
  LE_INVALID_TOKEN,
  LE_INVALID_COMMENT,
  LEXER_ERROR_MAX = LE_INVALID_COMMENT
};

class LexerException : public std::exception
{

  const LEXER_ERROR m_error;
  const char *m_msg; //Remember to not initialize it with string.cstr()
  const size_t m_line;
  const size_t m_col;

  public:

  LexerException(LEXER_ERROR error,const char* msg, size_t line, size_t col) :
  m_error(error), m_msg(msg), m_line(line), m_col(col){}

  const char* what() const noexcept override { return m_msg; }

  LEXER_ERROR get_error() const
  {
    return m_error;
  }

  const char* get_msg() const
  {
    return m_msg;
  }

  const size_t get_line() const
  {
    return m_line;
  }

  const size_t get_column() const
  {
    return m_col;
  }

};

struct Lexeme
{
private:
  TOKEN_TYPE m_type;
  std::string_view m_id;
  size_t m_line;
  size_t m_col;
  size_t m_index;
  union
  {
    Int m_ival;
    Real m_dval;
  };

public:
  TOKEN_TYPE type() const { return m_type; }
  std::string_view id() const { return m_id; }
  size_t line() const { return m_line; }
  size_t column() const { return m_col; }
  size_t index() const { return m_index; }
  Int int_value() const { return m_ival; }
  Real real_value() const { return m_dval; }

  std::string to_string_literal() const;
  std::string to_string() const {return std::format("'{}' at ({},{})", m_id, m_line, m_col);};

  friend class Lexer;
};

class Lexer
{
  Lexeme m_token;
  std::string m_content;
  size_t m_index;
  int m_current_char;
  size_t m_col;
  size_t m_line;

  const static std::unordered_map<std::string, TOKEN_TYPE> keywords;

  void read_char();
  int peek();
  void read_word();
  void read_string();
  void read_number();
  void skip_comment();

public:
  Lexer(std::string&& content);
  const Lexeme &next_sym();

  const Lexeme &getToken() const
  {
    return m_token;
  }

  const std::string& getContent() const
  {
    return m_content;
  }

  const size_t get_line() const
  {
    return m_line;
  }

  const size_t get_column() const
  {
    return m_col;
  }

};

};

#endif