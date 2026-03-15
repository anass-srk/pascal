#include "Ast.hpp"
#include "ValidationUtils.hpp"

namespace pascal_compiler
{

void UnaryExpression::validate()
{
  this->exprType = operand->exprType->get_underlying_type();
  if (!validation::is_valid_unary_operand(op, operand->exprType)) {
    const std::string expected = (op == UnaryOp::Not ? "Bool" : "Int, Char or Real");
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_OP,
      std::format(
        "Semantic error: Cannot apply unary operation ({}) on expression of type ({})! Expected {} !",
        token.to_string(), operand->exprType->to_string(), expected
      ),
      token.line(),
      token.column()
    );
  }
}

void BinaryExpression::validate()
{
  // For now, both expression are required to be bools
  // if an expression's type is a subrange, get the underlying type
  if (!validation::is_valid_binary_operand(op, left->exprType, right->exprType)) {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_OP,
      std::format(
        "Semantic error: Cannot apply binary operation ({}) on 2 expressions of type ({}) and ({})! Expected compatible basic types for relational operation!",
        token.to_string(), left->exprType->to_string(), right->exprType->to_string()
      ),
      token.line(),
      token.column()
    );
  }

  switch(op)
  {
    case BinaryOp::Le:
    case BinaryOp::Lt:
    case BinaryOp::Ge:
    case BinaryOp::Gt:
    case BinaryOp::Eq:
    case BinaryOp::Ne:
    break;
    default:
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_OP,
        std::format(
          "Semantic error: At ({}), expected comparison !",
          token.to_string()
        ),
        token.line(),
        token.column()
      );
  }
}

void NExpression::validate()
{
  // For now, all expression are required to be of the same type
  // if an expression's type is a subrange, get the underlying type

  const auto left = exprs[0].get();

  if(exprs.size() != ops.size()+1)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_OP,
      std::format(
        "Semantic error: At {}, found {} expressions and {} binary operations ! Expected {} binary operations !",
        token.to_string(), exprs.size(), ops.size(), exprs.size()-1
      ),
      token.line(),
      token.column()
    );
  }

  this->exprType = left->exprType->get_underlying_type();
  if (!validation::is_basic_type(exprType)) {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_OP,
      std::format(
        "Semantic error: Cannot apply binary operation ({}) on an expressions of type ({})! Expected Ints, Reals, Chars or Bools !",
        token.to_string(), left->exprType->to_string()
      ),
      token.line(),
      token.column()
    );
  }

  for(int i = 0;i < ops.size();++i)
  {
    const auto right = exprs[i+1].get();
    if (!validation::types_compatible(left->exprType, right->exprType)) {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_OP,
        std::format(
          "Semantic error: Cannot apply binary operation ({}) on 2 expressions of different types ({}) and ({})!",
          token.to_string(), left->exprType->to_string(), right->exprType->to_string()
        ),
        right->token.line(),
        right->token.column()
      );
    }
    const auto op = ops[i];  
    if (!validation::is_valid_binary_operand(op, left->exprType, right->exprType)) {
      std::string_view left_type = left->exprType->get_underlying_type()->m_name;
      if (left_type == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)]) {
        throw SemanticException(
          SEMANTIC_ERROR::SE_INVALID_OP,
          std::format(
            "Semantic error: Cannot apply binary operation ({}) on 2 boolean expressions !",
            right->token.to_string()
          ),
          right->token.line(),
          right->token.column()
        );
      } else {
        throw SemanticException(
          SEMANTIC_ERROR::SE_INVALID_OP,
          std::format(
            "Semantic error: Cannot apply binary operation ({}) on 2 expressions of underlying type '{}' !",
            right->token.to_string(), left_type
          ),
          right->token.line(),
          right->token.column()
        );
      }
    }
  }
}

void LiteralExpression::validate()
{
  this->exprType = value->type();
}

const Type * ArraySelector::apply(const Type *type)
{
  if(type->m_type != TYPE_CAT::TC_ARRAY)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_TYPE,
      std::format(
        "Semantic error: At ({}), type ({}) is not indexable ! Expected an array !",
        token.to_string(), type->to_string()
      ),
      token.line(),
      token.column()
    );
  }

  const Array *arr = static_cast<const Array *>(type);
  
  if(arr->m_itypes.size() != indices.size())
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_INDEX,
      std::format(
        "Semantic error: At ({}), array type ({}) requires {} indices ! Found {} indices !",
        token.to_string(), type->to_string(), arr->m_itypes.size(), indices.size()
      ),
      token.line(),
      token.column()
    );
  }

  for(int i = 0;i < indices.size();++i)
  {
    auto itype = indices[i]->exprType->get_underlying_type();
    
    auto vtype = arr->m_itypes[i]->get_underlying_type();
    
    if(vtype != itype)
    {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_INDEX,
        std::format(
          "Semantic error: At ({}), array type ({}) requires the index-{} to be of type ({}) ! Found {} !",
          token.to_string(), type->to_string(), i+1, arr->m_itypes[i]->to_string(), indices[i]->exprType->to_string()
        ),
        token.line(),
        token.column()
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
        "Semantic error: At ({}), Field name '{}' does not exists for non-record type ({}) ! Expected a record !",
        token.to_string(), field, type->to_string()
      ),
      token.line(),
      token.column()
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
      token.line(),
      token.column()
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
  if (!validation::is_basic_type(lhs->exprType) && !validation::is_enum_type(lhs->exprType)) {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_CALL,
      std::format(
        "Semantic error: In ({}), cannot assign an expressions to a variable of type ({})! Expected Ints, Reals, Chars, Bools or enums !",
        token.to_string(), lhs->exprType->to_string()
      ),
      token.line(),
      token.column()
    );
  }
  if (!validation::types_compatible(lhs->exprType, rhs->exprType)) {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_ASSIGN,
      std::format(
        "Semantic error: Variable access ({}) of type ({}) cannot be assigned value of different type ({}) !",
        lhs->baseVar->to_string(), lhs->exprType->to_string(), rhs->exprType->to_string()
      ),
      token.line(),
      token.column()
    );
  }
}

void LabeledStatement::validate()
{}

void WriteStatement::validate()
{
  for(const auto &exp : arguments)
  {
    if (!validation::is_writable_type(exp->exprType)) {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_CALL,
        std::format(
          "Semantic error: In ({}), cannot write an expressions of type ({})! Expected Ints, Reals, Chars, Strings, Bools or array of Chars !",
          token.to_string(), exp->exprType->to_string()
        ),
        token.line(),
        token.column()
      );
    }
  }
}

void ReadStatement::validate()
{
  for(const auto &exp : arguments)
  {
    if (!validation::is_readable_type(exp->exprType)) {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_CALL,
        std::format(
          "Semantic error: In ({}), cannot read a variable of type ({})! Expected Ints, Reals, Chars, Bools or array of Chars !",
          token.to_string(), exp->exprType->to_string()
        ),
        token.line(),
        token.column()
      );
    }
  }
}

void WhileStatement::validate()
{
  if (!validation::is_boolean_type(this->condition->exprType)) {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_COND,
      std::format(
        "Semantic error: In ({}), the while loop's condition is required to be a boolean ! Found {} !",
        this->token.to_string(), this->condition->exprType->to_string()
      ),
      token.line(),
      token.column()
    );
  }
}

void RepeatStatement::validate()
{
  if (!validation::is_boolean_type(this->untilExpr->exprType)) {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_COND,
      std::format(
        "Semantic error: In ({}), the repeat loop's condition is required to be a boolean ! Found {} !",
        this->token.to_string(), this->untilExpr->exprType->to_string()
      ),
      token.line(),
      token.column()
    );
  }
}

void ForStatement::validate()
{
  auto type_id = this->loopVar->exprType->get_underlying_type()->m_name;
  if(
    CONST_CAT_NAMES[int(this->start.category())] != type_id ||
    start.category() != end.category()
  )
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_TYPE,
      std::format(
        "Semantic error: In ({}), the for loop expects the bounds and the loop variable to be of the same type ! Found ({}) for the start, ({}) for the end and ({}) for the variable !",
        token.to_string(), CONST_CAT_NAMES[int(start.category())], CONST_CAT_NAMES[int(end.category())], loopVar->exprType->to_string()
      ),
      token.line(),
      token.column()
    );
  }
  Int a,b;
  switch (start.category())
  {
    case CONST_CAT::CC_CHAR:
      a = start.get<char>();
      b = end.get<char>();
    break;
    case CONST_CAT::CC_BOOL:
      a = start.get<bool>();
      b = end.get<bool>();
    break;
    case CONST_CAT::CC_ENUM:
    case CONST_CAT::CC_INT:
      a = start.get<Int>();
      b = end.get<Int>();
    break;
    default:
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_SUBRANGE,
        std::format(
            "Semantic error: In ({}), bounds can be made only using chars, enums or ints (constants) ! Found '{}' !",
            token.to_string(), CONST_CAT_NAMES[int(start.category())]
          ),
        token.line(),
        token.column()
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
      token.line(),
      token.column()
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
      token.line(),
      token.column()
    );
  }
}

void IfStatement::validate()
{
  if (!validation::is_boolean_type(this->condition->exprType)) {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_COND,
      std::format(
        "Semantic error: In ({}), the condition of if statement is required to be a boolean ! Found {} !",
        this->token.to_string(), this->condition->exprType->to_string()
      ),
      token.line(),
      token.column()
    );
  }
}

void CaseStatement::validate()
{
  auto type_id = this->selector->exprType->get_underlying_type()->m_name;
  if (!validation::is_case_selector_type(this->selector->exprType)) {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_TYPE,
      std::format(
        "Semantic error: In ({}), case statement's expression is of type ({}) ! Expected Int, Char, Bool or Enum type !",
        token.to_string(), this->selector->exprType->to_string()
      ),
      token.line(),
      token.column()
    );
  }

  auto check = [&]<typename T>(CONST_CAT cat)
  {
    std::unordered_set<T> values;
    for(const auto& alt : this->alternatives){
      for(int index = 0;index < alt.labels.size();++index){
        const auto current = alt.labels[index];
        if(!std::holds_alternative<T>(current.value()))
        {
          throw SemanticException(
            SEMANTIC_ERROR::SE_INVALID_TYPE,
            std::format(
              "Semantic error: In ({}), case statement's label ({}) is of type ({}) ! Expected {} !",
              alt.token.to_string(), current.to_string(), CONST_CAT_NAMES[int(current.category())], CONST_CAT_NAMES[int(cat)]
            ),
            alt.token.line(),
            alt.token.column()
          );
        }
        if(values.contains(current.get<T>()))
        {
          throw SemanticException(
            SEMANTIC_ERROR::SE_INVALID_TYPE,
            std::format(
              "Semantic error: In ({}), case statement's label ({}) is duplicated !",
              alt.token.to_string(), current.to_string()
            ),
            alt.token.line(),
            alt.token.column()
          );
        }
        values.insert(current.get<T>());
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
        token.line(),
        token.column()
      );
    }
  }
}

// How will we deal with references ? what about arrays ?
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
      token.line(),
      token.column()
    );
  }

  for(int i = 0;i < params.size();++i)
  {
    const Type* ptype = params[i].m_type->get_underlying_type();
    const Type* atype = args[i]->exprType->get_underlying_type();
    if(params[i].ref)
    {
      auto tmp = dynamic_cast<VariableAccess*>(args[i].get());
      if(!tmp)
      {
        throw SemanticException(
          SEMANTIC_ERROR::SE_INVALID_CALL,
          std::format(
            "Semantic error: At ({}), the {}-argument is not a variable but an expression !",
            token.to_string(), i+1
          ),
          token.line(),
          token.column()
        );
      }
    }
    if(ptype != atype)
    {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_CALL,
        std::format(
          "Semantic error: At ({}), the {}-argument is of type ({}) ! Expected type ({}) !",
          token.to_string(), i+1, atype->to_string(), ptype->to_string()
        ),
        token.line(),
        token.column()
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
      token.line(),
      token.column()
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
      token.line(),
      token.column()
    );
  }
  pascal_compiler::validate(function, args, token);
  this->exprType = function->m_type->m_ret_type;
}

}