#include "Ast.hpp"

namespace pascal_compiler
{

void UnaryExpression::validate(const Lexeme& token)
{
  auto type_name = operand->exprType->m_name;
  this->exprType = operand->exprType;
  if(operand->exprType->m_type == TYPE_CAT::TC_SUBRANGE)
  {
    this->exprType = static_cast<const Subrange *>(operand->exprType);
    type_name = CONST_CAT_NAMES[int(static_cast<const Subrange *>(operand->exprType)->m_cat)];
  }
  if (
    (op == UnaryOp::Plus || op == UnaryOp::Minus) &&
    type_name != CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] &&
    type_name != CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)] &&
    type_name != CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)]
  )
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_OP,
      std::format(
        "Semantic error: Cannot apply unary operation ({}) on expression of type ({})! Expected Int, Char or Real !",
        token.to_string(), operand->exprType->to_string()
      ),
      loc.line,
      loc.column
    );
  }

  if (op == UnaryOp::Not && type_name != CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)])
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_OP,
      std::format(
        "Semantic error: Cannot apply unary operation ({}) on expression of type ({})! Expected Bool !",
        token.to_string(), operand->exprType->to_string()
      ),
      loc.line,
      loc.column
    );
  }
}

void BinaryExpression::validate(const Lexeme& token)
{
  // For now, both expression are required to be of the same type
  // if an expression's type is a subrange, get the underlying type
  auto left_type = left->exprType->m_name;
  this->exprType = left->exprType;
  if (left->exprType->m_type == TYPE_CAT::TC_SUBRANGE)
  {
    this->exprType = static_cast<const Subrange *>(left->exprType);
    left_type = CONST_CAT_NAMES[int(static_cast<const Subrange *>(left->exprType)->m_cat)];
  }
  auto right_type = left->exprType->m_name;
  if (right->exprType->m_type == TYPE_CAT::TC_SUBRANGE)
  {
    right_type = CONST_CAT_NAMES[int(static_cast<const Subrange *>(right->exprType)->m_cat)];
  }

  if(left_type != right_type)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_OP,
      std::format(
        "Semantic error: Cannot apply binary operation ({}) on 2 expressions of different types ({}} and ({})!",
        token.to_string(), left->exprType->to_string(), right->exprType->to_string()
      ),
      loc.line,
      loc.column
    );
  }
  if(
    left_type != CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] &&
    left_type != CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)] &&
    left_type != CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)] && 
    left_type != CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)]
  )
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_OP,
      std::format(
        "Semantic error: Cannot apply binary operation ({}) on 2 expressions of type ({}} and ({})! Expected Ints, Reals, Chars or Bools !",
        token.to_string(), left->exprType->to_string(), right->exprType->to_string()
      ),
      loc.line,
      loc.column
    );
  }

  if(left_type == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)])
  {
    switch(op)
    {
      case BinaryOp::Eq:
      case BinaryOp::Ne:
      case BinaryOp::And:
      case BinaryOp::Or:
      break;
      default:
        throw SemanticException(
          SEMANTIC_ERROR::SE_INVALID_OP,
          std::format(
            "Semantic error: Cannot apply binary operation ({}) on 2 boolean expressions !",
            token.to_string()
          ),
          loc.line,
          loc.column
        );
    }
  }
  else
  {
    switch(op)
    {
      case BinaryOp::Add:
      case BinaryOp::Sub:
      case BinaryOp::Mul:
      case BinaryOp::Div:
      case BinaryOp::Eq:
      case BinaryOp::Ne:
      case BinaryOp::Ge:
      case BinaryOp::Gt:
      case BinaryOp::Le:
      case BinaryOp::Lt:
      break;
      default:
        throw SemanticException(
          SEMANTIC_ERROR::SE_INVALID_OP,
          std::format(
            "Semantic error: Cannot apply binary operation ({}) on 2 expressions of underlying type '{}' !",
            token.to_string(), left_type
          ),
          loc.line,
          loc.column
        );
    }
  }
}


}