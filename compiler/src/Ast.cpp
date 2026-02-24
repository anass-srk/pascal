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

void LiteralExpression::validate(const Lexeme& token)
{
  this->exprType = value->m_type;
}

const Type * ArraySelector::apply(const Type *type)
{
  if(type->m_type != TYPE_CAT::TC_ARRAY)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_TYPE,
      std::format(
        "Semantic error: type ({}) is not indexable ! Expected an array !",
        type->to_string()
      ),
      loc.line,
      loc.column
    );
  }

  const Array *arr = static_cast<const Array *>(type);
  
  if(arr->m_itypes.size() != indices.size())
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_INDEX,
      std::format(
        "Semantic error: array type ({}) requires {} indices ! Found {} indices !",
        type->to_string(), arr->m_itypes.size(), indices.size()
      ),
      loc.line,
      loc.column
    );
  }

  for(int i = 0;i < indices.size();++i)
  {
    auto itype = indices[i]->exprType;
    if(itype->m_type == TYPE_CAT::TC_SUBRANGE) itype = static_cast<const Subrange*>(itype)->m_type;
    
    auto vtype = arr->m_itypes[i];
    if(vtype->m_type == TYPE_CAT::TC_SUBRANGE) vtype = static_cast<const Subrange*>(vtype)->m_type;
    
    if(vtype != itype)
    {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_INDEX,
        std::format(
          "Semantic error: array type ({}) requires the index-{} to be of type ({}) ! Found {} !",
          type->to_string(), i+1, arr->m_itypes[i]->to_string(), indices[i]->exprType->to_string()
        ),
        loc.line,
        loc.column
      );
    }
  }

  return arr->m_etype;
}

const Type* FieldSelector::apply(const Type* type)
{
  if(type->m_type != TYPE_CAT::TC_RECORD)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_TYPE,
      std::format(
        "Semantic error: Field name '{}' does not exists for non-record type ({}) ! Expected a record !",
        field, type->to_string()
      ),
      loc.line,
      loc.column
    );
  }

  const Record* rec = static_cast<const Record*>(type);
  auto it = rec->m_members.find(field);
  if(it == rec->m_members.end())
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_FIELD_NAME,
      std::format(
        "Semantic error: Field name '{}' does not exists for record type ({}) !",
        field, type->to_string()
      ),
      loc.line,
      loc.column
    );
  }

  return it->second.m_type;
}


void VariableAccess::validate(const Lexeme& token)
{
  const Type* current = baseVar->m_type;
  
  for(const auto& s : selectors)
  {
    current = s->apply(current);
  }

  this->exprType = current;
}