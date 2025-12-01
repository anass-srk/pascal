#ifndef LEXER_HXX
#define LEXER_HXX

#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <concepts>

namespace pascal_compiler
{

enum class TOKEN_TYPE : int
{
  AND_TOKEN,
  ARRAY_TOKEN,
  BEGIN_TOKEN,
  CASE_TOKEN,
  CHR_TOKEN,
  CONST_TOKEN,
  DIV_TOKEN,
  DO_TOKEN,
  DOWNTO_TOKEN,
  ELSE_TOKEN,
  END_TOKEN,
  FILE_TOKEN,
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
  PACKED_TOKEN,
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
  DOTDOT_TOKEN,
  LC_TOKEN,
  RC_TOKEN,
  TRUE_TOKEN,
  FALSE_TOKEN,
  ID_TOKEN,
  STRING_LITERAL_TOKEN,
  NUM_INT_TOKEN,
  NUM_REAL_TOKEN,
  EOF_TOKEN,
  ERROR_TOKEN,
  CHAR_LITERAL_TOKEN,
  TOKEN_TYPE_MAX = CHAR_LITERAL_TOKEN
};

static const std::string token_type_name[size_t(TOKEN_TYPE::TOKEN_TYPE_MAX) + 1] = {
  "AND_TOKEN",
  "ARRAY_TOKEN",
  "BEGIN_TOKEN",
  "CASE_TOKEN",
  "CHR_TOKEN",
  "CONST_TOKEN",
  "DIV_TOKEN",
  "DO_TOKEN",
  "DOWNTO_TOKEN",
  "ELSE_TOKEN",
  "END_TOKEN",
  "FILE_TOKEN",
  "FOR_TOKEN",
  "FUNCTION_TOKEN",
  "GOTO_TOKEN",
  "IF_TOKEN",
  "IN_TOKEN",
  "LABEL_TOKEN",
  "MOD_TOKEN",
  "NIL_TOKEN",
  "NOT_TOKEN",
  "OF_TOKEN",
  "OR_TOKEN",
  "PACKED_TOKEN",
  "PROCEDURE_TOKEN",
  "PROGRAM_TOKEN",
  "RECORD_TOKEN",
  "REPEAT_TOKEN",
  "SET_TOKEN",
  "THEN_TOKEN",
  "TO_TOKEN",
  "TYPE_TOKEN",
  "UNTIL_TOKEN",
  "VAR_TOKEN",
  "WHILE_TOKEN",
  "WITH_TOKEN",
  "PLUS_TOKEN",
  "MINUS_TOKEN",
  "STAR_TOKEN",
  "SLASH_TOKEN",
  "ASSIGN_TOKEN",
  "COMMA_TOKEN",
  "SEMI_TOKEN",
  "COLON_TOKEN",
  "EQ_TOKEN",
  "NEQ_TOKEN",
  "LT_TOKEN",
  "LE_TOKEN",
  "GT_TOKEN",
  "GE_TOKEN",
  "LP_TOKEN",
  "RP_TOKEN",
  "LB_TOKEN",
  "RB_TOKEN",
  "READ_TOKEN",
  "WRITE_TOKEN",
  "POINTER_TOKEN",
  "AT_TOKEN",
  "DOT_TOKEN",
  "DOTDOT_TOKEN",
  "LC_TOKEN",
  "RC_TOKEN",
  "TRUE_TOKEN",
  "FALSE_TOKEN",
  "ID_TOKEN",
  "STRING_LITERAL_TOKEN",
  "NUM_INT_TOKEN",
  "NUM_REAL_TOKEN",
  "EOF_TOKEN",
  "ERROR_TOKEN",
  "CHAR_LITERAL_TOKEN"
};

enum class LEXER_ERROR
{
  LE_INVALID_CHAR,
  LE_INVALID_NUMBER,
  LE_INVALID_STRING,
  LE_INVALID_COMMENT,
  LEXER_ERROR_MAX = LE_INVALID_COMMENT
};

class LexerException{

  const LEXER_ERROR m_error;
  const char* m_msg;
  const size_t m_line;
  const size_t m_col;

  public:

  LexerException(LEXER_ERROR error,const char* msg, size_t line, size_t col) : 
  m_error(error), m_msg(msg), m_line(line), m_col(col){} 

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
  TOKEN_TYPE m_type;
  union
  {
    long long m_ival;
    double m_dval;
  };
  std::string m_id;
  size_t m_col;
  size_t m_line;
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
  static constexpr size_t id_max_len = 255;

  void read_char();
  int peek();
  void read_word();
  void read_string();
  void read_number();
  void lexer_error(LEXER_ERROR le);
  void skip_comment();

public:
  Lexer(const std::string& filename);
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