#include "Lexer.hpp"

namespace pascal_compiler
{

const char* TOKEN_NAMES[size_t(TOKEN_TYPE::TOKEN_TYPE_MAX) + 1] = 
{
  "and",
  "array",
  "begin",
  "case",
  "const",
  "div",
  "do",
  "downto",
  "else",
  "end",
  "for",
  "function",
  "goto",
  "if",
  "in",
  "label",
  "mod",
  "nil",
  "not",
  "of",
  "or",
  "procedure",
  "program",
  "record",
  "repeat",
  "set",
  "then",
  "to",
  "type",
  "until",
  "var",
  "while",
  "with",
  "plus",
  "minus",
  "star",
  "slash",
  "assign",
  "comma",
  ";",
  ":",
  "=",
  "<>",
  "<",
  "<=",
  ">",
  ">=",
  "(",
  ")",
  "[",
  "]",
  "read",
  "write",
  "^",
  "@",
  ".",
  "..",
  "true",
  "false",
  "ID",
  "string",
  "integer",
  "real",
  "char",
  "EOF_TOKEN"
};

const std::unordered_map<std::string, TOKEN_TYPE> Lexer::keywords = {
  {"and", TOKEN_TYPE::AND_TOKEN},
  {"array", TOKEN_TYPE::ARRAY_TOKEN},
  {"begin", TOKEN_TYPE::BEGIN_TOKEN},
  {"case", TOKEN_TYPE::CASE_TOKEN},
  {"const", TOKEN_TYPE::CONST_TOKEN},
  {"div", TOKEN_TYPE::DIV_TOKEN},
  {"do", TOKEN_TYPE::DO_TOKEN},
  {"downto", TOKEN_TYPE::DOWNTO_TOKEN},
  {"else", TOKEN_TYPE::ELSE_TOKEN},
  {"end", TOKEN_TYPE::END_TOKEN},
  {"for", TOKEN_TYPE::FOR_TOKEN},
  {"function", TOKEN_TYPE::FUNCTION_TOKEN},
  {"goto", TOKEN_TYPE::GOTO_TOKEN},
  {"if", TOKEN_TYPE::IF_TOKEN},
  {"in", TOKEN_TYPE::IN_TOKEN},
  {"label", TOKEN_TYPE::LABEL_TOKEN},
  {"mod", TOKEN_TYPE::MOD_TOKEN},
  {"nil", TOKEN_TYPE::NIL_TOKEN},
  {"not", TOKEN_TYPE::NOT_TOKEN},
  {"of", TOKEN_TYPE::OF_TOKEN},
  {"or", TOKEN_TYPE::OR_TOKEN},
  {"procedure", TOKEN_TYPE::PROCEDURE_TOKEN},
  {"program", TOKEN_TYPE::PROGRAM_TOKEN},
  {"record", TOKEN_TYPE::RECORD_TOKEN},
  {"repeat", TOKEN_TYPE::REPEAT_TOKEN},
  {"read", TOKEN_TYPE::READ_TOKEN},
  {"write", TOKEN_TYPE::WRITE_TOKEN},
  {"set", TOKEN_TYPE::SET_TOKEN},
  {"then", TOKEN_TYPE::THEN_TOKEN},
  {"to", TOKEN_TYPE::TO_TOKEN},
  {"type", TOKEN_TYPE::TYPE_TOKEN},
  {"until", TOKEN_TYPE::UNTIL_TOKEN},
  {"var", TOKEN_TYPE::VAR_TOKEN},
  {"while", TOKEN_TYPE::WHILE_TOKEN},
  {"with", TOKEN_TYPE::WITH_TOKEN},
  {"true", TOKEN_TYPE::TRUE_TOKEN},
  {"false", TOKEN_TYPE::FALSE_TOKEN},
};

std::string Lexeme::to_string_literal() const
{
  std::string res;
  res.reserve(m_id.length());
  for (size_t i = 1; i < m_id.length(); ++i) // To skip first quote
  {
    if (m_id[i] == '\\')
    {
      ++i; // consume backslash. No need to check if out-of-bounds since m_id was checked before
      switch (m_id[i])
      {
      case 'n':
        res += '\n';
        break;
      case 't':
        res += '\t';
        break;
      case 'r':
        res += '\r';
        break;
      case '\\':
        res += '\\';
        break;
      case '"':
        res += '"';
        break;
      }
    }
    else
    {
      res += m_id[i];
    }
  }
  return res;
}

Lexer::Lexer(std::string &&content) : m_content(std::move(content)), m_index(0), m_col(1), m_line(1)
{

  m_token.m_line = 1;
  m_token.m_col = 1;

  read_char();
}

void Lexer::skip_comment()
{
  // Skip until closing brace
  while (m_current_char != EOF && m_current_char != '}')
  {
    read_char();
  }
  if (m_current_char == '}')
  {
    read_char();
    /*
    consume the '}' and point to the next char
    Read the next char since all of these functions assume that the first element was read,
    which is why they read the next char after their corresponding tokens are constructed
    */
  }
  else
  {
    throw LexerException(LEXER_ERROR::LE_INVALID_COMMENT,"Lexer error: missing closing '}' of the comment",m_line,m_col);
  }
}

void Lexer::read_char()
{
  if(m_index >= m_content.size())
  {
    m_current_char = EOF; // If m_current_char is not set, you will stuck in an infinite loop for the last token
    return;
  }
  m_current_char = m_content[m_index++];
  if (m_current_char == '\n')
  {
    ++m_line;
    m_col = 1;
  }
  else
  {
    ++m_col;
  }
}

int Lexer::peek()
{
  if(m_index >= m_content.size()) return EOF; //m_index is already pointing to the next value
  return m_content[m_index];
}

void Lexer::read_number()
{
  bool is_real = false;
  size_t len = 0;
  do
  {
    ++len;
    read_char();
  } while (isdigit(m_current_char));

  if (m_current_char == '.')
  {
    // Check if it's a range operator or decimal point
    // peek will keep the index unchanged
    int next_char = peek();

    if (isdigit(next_char))
    {
      is_real = true;
      // m_current_char is '.' here and the next one is a number
      ++len;
      read_char();
      do
      {
        ++len;
        read_char();
      } while (isdigit(m_current_char));
    }
  }

  if (m_current_char == 'e' || m_current_char == 'E')
  {
    is_real = true;
    ++len;
    read_char();
    
    if (m_current_char == '+' || m_current_char == '-')
    {
      ++len;
      read_char();
    }
    
    if (!isdigit(m_current_char))
    {
      throw LexerException(LEXER_ERROR::LE_INVALID_NUMBER,"Lexer error: 'e' or 'E' should be followed by a number (after '+'/'-' if used)",m_line,m_col);
    }
    
    do
    {
      ++len;
      read_char();
    }while (isdigit(m_current_char));
  }

  m_token.m_id = std::string_view(m_content.data()+m_token.m_index,len);

  if (is_real)
  {
    m_token.m_dval = atof(std::string(m_token.m_id).c_str());
    m_token.m_type = TOKEN_TYPE::NUM_REAL_TOKEN;
  }
  else
  {
    m_token.m_ival = atoll(std::string(m_token.m_id).c_str());
    m_token.m_type = TOKEN_TYPE::NUM_INT_TOKEN;
  }
}

void Lexer::read_word()
{

  size_t len = 0;

  do
  {
    ++len;
    read_char();
  } while (isalnum(m_current_char) || m_current_char == '_');

  m_token.m_id = std::string_view(m_content.data()+m_token.m_index,len);
  
  // Convert to lowercase for keyword lookup
  std::string lower_id = std::string(m_token.m_id);
  for (char &c : lower_id)
  {
    c = tolower(c);
  }

  if (auto it = keywords.find(lower_id); it != keywords.end())
  {
    m_token.m_type = it->second;
  }
  else
  {
    m_token.m_type = TOKEN_TYPE::ID_TOKEN;
  }
}

void Lexer::read_string()
{
  const char quote = m_current_char;

  read_char(); //point to the next char after the quote

  size_t len = 1;

  while (m_current_char != EOF && m_current_char != quote)
  {
    if (m_current_char == '\\')
    {
      read_char(); // consume backslash
      switch (m_current_char)
      {
      case 'n':
      case 't':
      case 'r':
      case '\\':
      case '"':
        len += 2;
        break;
      default:
        throw LexerException(LEXER_ERROR::LE_INVALID_STRING,"Lexer error: only \\n, \\t, \\r, \\\\ and \" can be used in a string",m_line,m_col);
      }
    }
    else
    {
      ++len;
    }
    read_char();
  }

  m_token.m_id = std::string_view(m_content.data() + m_token.m_index, len);

  if (m_current_char == quote)
  {
    read_char(); // consume closing quote
  }
  // For chars :
  // the length is 2 (includes single quote)
  // Could be 3 if the first char is '\' (escaped chars) 
  else if (quote == '\'' && (len != 2 || (len == 3 && m_token.m_id[1] != '\\') || len > 3))
  {
    throw LexerException(LEXER_ERROR::LE_INVALID_CHAR,"Lexer error: chars have a length of 1",m_line,m_col);
  }
  else
  {
    throw LexerException(LEXER_ERROR::LE_INVALID_STRING,"Lexer error: missing closing quote",m_line,m_col);
  }

  m_token.m_type = (quote == '"' ? TOKEN_TYPE::STRING_LITERAL_TOKEN : TOKEN_TYPE::CHAR_LITERAL_TOKEN);

}

const Lexeme &Lexer::next_sym()
{

  while (isspace(m_current_char))
  {
    read_char();
  }

  if (m_current_char == EOF)
  {
    m_token.m_type = TOKEN_TYPE::EOF_TOKEN;
    return m_token;
  }
  // Handle comments
  if (m_current_char == '{')
  {
    skip_comment();
    return next_sym(); // Skip to next token after comment
  }

  m_token.m_id = "";
  m_token.m_line = m_line;
  m_token.m_col = m_col;
  m_token.m_index = m_index-1;

  if (isalpha(m_current_char) || m_current_char == '_')
  {
    read_word();
    return m_token;
  }
  if (isdigit(m_current_char))
  {
    read_number();
    return m_token;
  }
  if (m_current_char == '"' || m_current_char == '\'') // Should handle chars too
  {
    read_string();
    return m_token;
  }
  // Handle special chars
  switch (m_current_char)
  {
  case '+':
    m_token.m_id = "+";
    m_token.m_type = TOKEN_TYPE::PLUS_TOKEN;
    break;
  case '-':
    m_token.m_id = "-";
    m_token.m_type = TOKEN_TYPE::MINUS_TOKEN;
    break;
  case '*':
    m_token.m_id = "*";
    m_token.m_type = TOKEN_TYPE::STAR_TOKEN;
    break;
  case '/':
    m_token.m_id = "/";
    m_token.m_type = TOKEN_TYPE::SLASH_TOKEN;
    break;
  case ':':
    read_char();
    if (m_current_char == '=')
    {

      m_token.m_id = ":=";
      m_token.m_type = TOKEN_TYPE::ASSIGN_TOKEN;
      // read_char() will be called after the switch statement
    }
    else
    {
      m_token.m_id = ":";
      m_token.m_type = TOKEN_TYPE::COLON_TOKEN;
      return m_token;
    }
    break;
  case ',':
    m_token.m_id = ",";
    m_token.m_type = TOKEN_TYPE::COMMA_TOKEN;
    break;
  case ';':
    m_token.m_id = ";";
    m_token.m_type = TOKEN_TYPE::SEMI_TOKEN;
    break;
  case '=':
    m_token.m_id = "=";
    m_token.m_type = TOKEN_TYPE::EQ_TOKEN;
    break;
  case '<':
    read_char();
    if (m_current_char == '>')
    {
      m_token.m_id = "<>";
      m_token.m_type = TOKEN_TYPE::NEQ_TOKEN;
    }
    else if (m_current_char == '=')
    {
      m_token.m_id = "<=";
      m_token.m_type = TOKEN_TYPE::LE_TOKEN;
    }
    else
    {
      m_token.m_id = "<";
      m_token.m_type = TOKEN_TYPE::LT_TOKEN;
      return m_token;
    }
    break;
  case '>':
    read_char();
    if (m_current_char == '=')
    {
      m_token.m_id = ">=";
      m_token.m_type = TOKEN_TYPE::GE_TOKEN;
    }
    else
    {
      m_token.m_id = ">";
      m_token.m_type = TOKEN_TYPE::GT_TOKEN;
      return m_token;
    }
    break;
  case '(':
    m_token.m_id = "(";
    m_token.m_type = TOKEN_TYPE::LP_TOKEN;
    break;
  case ')':
    m_token.m_id = ")";
    m_token.m_type = TOKEN_TYPE::RP_TOKEN;
    break;
  case '[':
    m_token.m_id = "[";
    m_token.m_type = TOKEN_TYPE::LB_TOKEN;
    break;
  case ']':
    m_token.m_id = "]";
    m_token.m_type = TOKEN_TYPE::RB_TOKEN;
    break;
  case '^':
    m_token.m_id = "^";
    m_token.m_type = TOKEN_TYPE::POINTER_TOKEN;
    break;
  case '@':
    m_token.m_id = "@";
    m_token.m_type = TOKEN_TYPE::AT_TOKEN;
    break;
  case '.':
    read_char();
    if (m_current_char == '.')
    {
      m_token.m_id = "..";
      m_token.m_type = TOKEN_TYPE::RANGE_TOKEN;
    }
    else
    {
      m_token.m_id = ".";
      m_token.m_type = TOKEN_TYPE::DOT_TOKEN;
      return m_token;
    }
    break;
  default:
    throw LexerException(LEXER_ERROR::LE_INVALID_TOKEN,"Lexer error: invalid token",m_line,m_col);
  }
  read_char();
  return m_token;
}

}