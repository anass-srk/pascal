#include "Ast.hpp"

namespace pascal_compiler
{

void UnaryExpression::validate()
{
  auto type_name = operand->exprType->m_name;
  this->exprType = operand->exprType;
  if(operand->exprType->m_type == TYPE_CAT::TC_SUBRANGE)
  {
    this->exprType = static_cast<const Subrange *>(operand->exprType)->m_utype;
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
      token.m_line,
      token.m_col
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
      token.m_line,
      token.m_col
    );
  }
}

void BinaryExpression::validate()
{
  // For now, both expression are required to be bools
  // if an expression's type is a subrange, get the underlying type
  auto left_type = left->exprType->m_name;
  this->exprType = left->exprType;
  if (left->exprType->m_type == TYPE_CAT::TC_SUBRANGE)
  {
    this->exprType = static_cast<const Subrange *>(left->exprType)->m_utype;
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
        "Semantic error: Cannot apply binary operation ({}) on 2 expressions of different types ({}) and ({})!",
        token.to_string(), left->exprType->to_string(), right->exprType->to_string()
      ),
      token.m_line,
      token.m_col
    );
  }
  if(left_type != CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)])
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_OP,
      std::format(
        "Semantic error: Cannot apply binary operation ({}) on 2 expressions of type ({}) and ({})! Expected Bools !",
        token.to_string(), left->exprType->to_string(), right->exprType->to_string()
      ),
      token.m_line,
      token.m_col
    );
  }

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
        token.m_line,
        token.m_col
      );
  }
}

void NExpression::validate()
{
  // For now, all expression are required to be of the same type
  // if an expression's type is a subrange, get the underlying type

  const auto left = exprs[0].get();

  if(exprs.size()+1 != ops.size())
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_OP,
      std::format(
        "Semantic error: At {}, found {} expressions and {} binary operations ! Expected {} binary operations !",
        token.to_string(), exprs.size(), ops.size(), exprs.size()-1
      ),
      token.m_line,
      token.m_col
    );
  }

  auto left_type = left->exprType->m_name;
  this->exprType = left->exprType;
  if (left->exprType->m_type == TYPE_CAT::TC_SUBRANGE)
  {
    this->exprType = static_cast<const Subrange *>(left->exprType)->m_utype;
    left_type = CONST_CAT_NAMES[int(static_cast<const Subrange *>(left->exprType)->m_cat)];
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
        "Semantic error: Cannot apply binary operation ({}) on an expressions of type ({})! Expected Ints, Reals, Chars or Bools !",
        token.to_string(), left->exprType->to_string()
      ),
      token.m_line,
      token.m_col
    );
  }

  for(int i = 0;i < ops.size();++i)
  {
    const auto right = exprs[i+1].get();
    auto right_type = right->exprType->m_name;
    if (right->exprType->m_type == TYPE_CAT::TC_SUBRANGE)
    {
      right_type = CONST_CAT_NAMES[int(static_cast<const Subrange *>(right->exprType)->m_cat)];
    }
    if(left_type != right_type)
    {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_OP,
        std::format(
          "Semantic error: Cannot apply binary operation ({}) on 2 expressions of different types ({}) and ({})!",
          token.to_string(), left->exprType->to_string(), right->exprType->to_string()
        ),
        right->token.m_line,
        right->token.m_col
      );
    }
    const auto op = ops[i];  
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
              right->token.to_string()
            ),
            right->token.m_line,
            right->token.m_col
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
              right->token.to_string(), left_type
            ),
            right->token.m_line,
            right->token.m_col
          );
      }
    }
  }
}

void LiteralExpression::validate()
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
      token.m_line,
      token.m_col
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
      token.m_line,
      token.m_col
    );
  }

  for(int i = 0;i < indices.size();++i)
  {
    auto itype = indices[i]->exprType;
    if(itype->m_type == TYPE_CAT::TC_SUBRANGE) itype = static_cast<const Subrange*>(itype)->m_utype;
    
    auto vtype = arr->m_itypes[i];
    if(vtype->m_type == TYPE_CAT::TC_SUBRANGE) vtype = static_cast<const Subrange*>(vtype)->m_utype;
    
    if(vtype != itype)
    {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_INDEX,
        std::format(
          "Semantic error: array type ({}) requires the index-{} to be of type ({}) ! Found {} !",
          type->to_string(), i+1, arr->m_itypes[i]->to_string(), indices[i]->exprType->to_string()
        ),
        token.m_line,
        token.m_col
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
      token.m_line,
      token.m_col
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
      token.m_line,
      token.m_col
    );
  }

  return it->second.m_type;
}


void VariableAccess::validate()
{
  const Type* current = baseVar->m_type->get_underlying_type();
  
  for(const auto& s : selectors)
  {
    current = s->apply(current);
  }

  this->exprType = current;
}


void CompoundStatement::validate()
{
  for(const auto& stmt : statements)
  {
    stmt->validate();
  }
}

void AssignmentStatement::validate()
{
  if(lhs->exprType->get_underlying_type() != rhs->exprType->get_underlying_type())
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_ASSIGN,
      std::format(
        "Semantic error: Variable access ({}) of type ({}) cannot be assigned value of different type ({}) !",
        lhs->baseVar->to_string(), lhs->exprType->to_string(), rhs->exprType->to_string()
      ),
      token.m_line,
      token.m_col
    );
  }
}

void LabeledStatement::validate()
{}

void WriteStatement::validate()
{
  for(const auto &exp : arguments)
  {
    const auto type = exp->exprType->get_underlying_type()->m_name;
    
    if(
      type != CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] &&
      type != CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)] &&
      type != CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)] && 
      type != CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)]
    )
    {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_CALL,
        std::format(
          "Semantic error: In ({}), cannot write an expressions of type ({})! Expected Ints, Reals, Chars or Bools !",
          token.to_string(), exp->exprType->to_string()
        ),
        token.m_line,
        token.m_col
      );
    }

  }
}

void ReadStatement::validate()
{
  for(const auto &exp : arguments)
  {
    const auto type = exp->exprType->get_underlying_type()->m_name;
    
    if(
      type != CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] &&
      type != CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)] &&
      type != CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)] && 
      type != CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)]
    )
    {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_CALL,
        std::format(
          "Semantic error: In ({}), cannot read a variable of type ({})! Expected Ints, Reals, Chars or Bools !",
          token.to_string(), exp->exprType->to_string()
        ),
        token.m_line,
        token.m_col
      );
    }

  }
}

void WhileStatement::validate()
{
  if(this->condition->exprType->get_underlying_type()->m_name != CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)])
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_COND,
      std::format(
        "Semantic error: In ({}), the while loop's condition is required to be a boolean ! Found {} !",
        this->token.to_string(), this->condition->exprType->to_string()
      ),
      token.m_line,
      token.m_col
    );
  }
}

void RepeatStatement::validate()
{
  if(this->untilExpr->exprType->get_underlying_type()->m_name != CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)])
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_COND,
      std::format(
        "Semantic error: In ({}), the repeat loop's condition is required to be a boolean ! Found {} !",
        this->token.to_string(), this->untilExpr->exprType->to_string()
      ),
      token.m_line,
      token.m_col
    );
  }
}

void ForStatement::validate()
{
  auto type_id = this->loopVar->exprType->get_underlying_type()->m_name;
  if(
    CONST_CAT_NAMES[int(this->start.m_cat)] != type_id ||
    start.m_cat != end.m_cat
  )
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_TYPE,
      std::format(
        "Semantic error: In ({}), the for loop expects the bounds and the loop variable to be of the same type ! Found ({}) for the start, ({}) for the end and ({}) for the variable !",
        token.to_string(), CONST_CAT_NAMES[int(start.m_cat)], CONST_CAT_NAMES[int(end.m_cat)], loopVar->exprType->to_string()
      ),
      token.m_line,
      token.m_col
    );
  }
  Int a,b;
  switch (start.m_cat)
  {
    case CONST_CAT::CC_CHAR:
      a = std::get<char>(start.m_val);
      b = std::get<char>(end.m_val);
    break;
    case CONST_CAT::CC_BOOL:
      a = std::get<bool>(start.m_val);
      b = std::get<bool>(end.m_val);
    break;
    case CONST_CAT::CC_ENUM:
    case CONST_CAT::CC_INT:
      a = std::get<Int>(start.m_val);
      b = std::get<Int>(end.m_val);
    break;
    default:
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_SUBRANGE,
        std::format(
            "Semantic error: In ({}), bounds can be made only using chars, enums or ints (constants) ! Found '{}' !",
            token.to_string(), CONST_CAT_NAMES[int(start.m_cat)]  
          ),
        token.m_line,
        token.m_col
      );
  }
  if(increasing && a > b)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_SUBRANGE,
      std::format(
        "Semantic error: In ({}), the lower bound should be less than the upper bound ! Found {}->{} !",
        token.to_string(), a, b
      ),
      token.m_line,
      token.m_col
    );
  }
  else if(!increasing && a < b)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_SUBRANGE,
      std::format(
        "Semantic error: In ({}), the lower bound should be greater than the upper bound ! Found {}->{} !",
        token.to_string(), a, b
      ),
      token.m_line,
      token.m_col
    );
  }
}

void IfStatement::validate()
{
  if(this->condition->exprType->get_underlying_type()->m_name != CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)])
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_COND,
      std::format(
        "Semantic error: In ({}), the condition of if statement is required to be a boolean ! Found {} !",
        this->token.to_string(), this->condition->exprType->to_string()
      ),
      token.m_line,
      token.m_col
    );
  }
}

void CaseStatement::validate()
{
  auto type_id = this->selector->exprType->get_underlying_type()->m_name;

  auto check = [&]<typename T>(CONST_CAT cat)
  {
    std::unordered_set<T> values;
    for(const auto& alt : this->alternatives){
      for(int index = 0;index < alt.labels.size();++index){
        const auto current = alt.labels[index];
        if(!std::holds_alternative<T>(current.m_val))
        {
          throw SemanticException(
            SEMANTIC_ERROR::SE_INVALID_TYPE,
            std::format(
              "Semantic error: In ({}), case statement's label ({}) is of type ({}) ! Expected {} !",
              alt.token.to_string(), current.to_string(), CONST_CAT_NAMES[int(current.m_cat)], CONST_CAT_NAMES[int(cat)]
            ),
            alt.token.m_line,
            alt.token.m_col
          );
        }
        if(values.contains(std::get<T>(current.m_val)))
        {
          throw SemanticException(
            SEMANTIC_ERROR::SE_INVALID_TYPE,
            std::format(
              "Semantic error: In ({}), case statement's label ({}) is duplicated !",
              alt.token.to_string(), current.to_string()
            ),
            alt.token.m_line,
            alt.token.m_col
          );
        }
        values.insert(std::get<T>(current.m_val));
      }
    }
  };

  for(const auto& alt : alternatives)
  {
    if(type_id == CONST_CAT_NAMES[int(CONST_CAT::CC_INT)])
    {
      check.template operator()<Int>(CONST_CAT::CC_INT);
    }
    else if (type_id == CONST_CAT_NAMES[int(CONST_CAT::CC_ENUM)])
    {
      check.template operator()<Int>(CONST_CAT::CC_ENUM);
    }
    else if (type_id == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)])
    {
      check.template operator()<bool>(CONST_CAT::CC_BOOL);
    }
    else if (type_id == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)])
    {
      check.template operator()<char>(CONST_CAT::CC_CHAR);
    }
    else
    {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_TYPE,
        std::format(
          "Semantic error: In ({}), case statement's expression is of type ({}) ! Expected Int, Char, Bool or Enum type !",
          token.to_string(), this->selector->exprType->to_string()
        ),
        token.m_line,
        token.m_col
      );
    }
  }
}

void validate(const Function *func, const std::vector<std::unique_ptr<Expression>> &args, Lexeme token)
{
  const auto& params = func->m_type->m_args;  
  if(params.size() != args.size())
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_CALL,
      std::format(
        "Semantic error: At ({}), found {} arguments ! Expected {} arguments !",
        token.to_string(), args.size(), params.size()
      ),
      token.m_line,
      token.m_col
    );
  }

  for(int i = 0;i < params.size();++i)
  {
    const Type* ptype = params[i].m_type->get_underlying_type();
    const Type* atype = args[i]->exprType->get_underlying_type();
    if(ptype != atype)
    {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_CALL,
        std::format(
          "Semantic error: At ({}), the {}-argument is of type ({}) ! Expected type ({}) !",
          token.to_string(), i+1, atype->to_string(), ptype->to_string()
        ),
        token.m_line,
        token.m_col
      );
    }
  }
}

void ProcedureCall::validate()
{
  if(procedure->m_type->m_ret_type != nullptr)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_CALL,
      std::format(
        "Semantic error: At ({}), invalid procedure with return-type ({}) !",
        token.to_string(), procedure->m_type->m_ret_type->to_string()
      ),
      token.m_line,
      token.m_col
    );
  }
  pascal_compiler::validate(procedure, args, token);
}

void FunctionCall::validate()
{
  if(function->m_type->m_ret_type == nullptr)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_CALL,
      std::format(
        "Semantic error: At ({}), invalid function with no return-type !",
        token.to_string()
      ),
      token.m_line,
      token.m_col
    );
  }
  pascal_compiler::validate(function, args, token);
  this->exprType = function->m_type->m_ret_type;
}

}