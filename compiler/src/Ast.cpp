#include "Ast.hpp"
#include "ValidationUtils.hpp"

namespace pascal_compiler
{

void UnaryExpression::validate()
{
  this->m_expr_type = m_operand->type()->get_underlying_type();
  if (!validation::is_valid_unary_operand(m_op, m_operand->type())) {
    const std::string expected = (m_op == UnaryOp::Not ? "Bool" : "Int, Char or Real");
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_OP,
      std::format(
        "Semantic error: Cannot apply unary operation ({}) on expression of type ({})! Expected {} !",
        m_token.to_string(), m_operand->type()->to_string(), expected
      ),
      m_token.line(),
      m_token.column()
    );
  }
}

void BinaryExpression::validate()
{
  // For now, both expression are required to be bools
  // if an expression's type is a subrange, get the underlying type
  if (!validation::is_valid_binary_operand(m_op, m_left->type(), m_right->type())) {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_OP,
      std::format(
        "Semantic error: Cannot apply binary operation ({}) on 2 expressions of type ({}) and ({})! Expected compatible basic types (including subranges and enums) for relational operation!",
        m_token.to_string(), m_left->type()->to_string(), m_right->type()->to_string()
      ),
      m_token.line(),
      m_token.column()
    );
  }

  switch(m_op)
  {
    case RelOp::Le:
    case RelOp::Lt:
    case RelOp::Ge:
    case RelOp::Gt:
    case RelOp::Eq:
    case RelOp::Ne:
    break;
    default:
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_OP,
        std::format(
          "Semantic error: At ({}), expected comparison !",
          m_token.to_string()
        ),
        m_token.line(),
        m_token.column()
      );
  }
}

void NExpression::validate()
{
  // For now, all expression are required to be of the same type
  // if an expression's type is a subrange, get the underlying type

  const auto left = m_exprs[0].get();

  if(m_exprs.size() != m_ops.size()+1)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_OP,
      std::format(
        "Semantic error: At {}, found {} expressions and {} binary operations ! Expected {} binary operations !",
        m_token.to_string(), m_exprs.size(), m_ops.size(), m_exprs.size()-1
      ),
      m_token.line(),
      m_token.column()
    );
  }

  this->m_expr_type = left->type()->get_underlying_type();
  if (!validation::is_basic_type(type())) {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_OP,
      std::format(
        "Semantic error: Cannot apply binary operation ({}) on an expressions of type ({})! Expected Ints, Reals, Chars, Bools or Strings !",
        m_token.to_string(), left->type()->to_string()
      ),
      m_token.line(),
      m_token.column()
    );
  }

  for(int i = 0;i < m_ops.size();++i)
  {
    const auto right = m_exprs[i+1].get();
    if (!validation::types_compatible(left->type(), right->type())) {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_OP,
        std::format(
          "Semantic error: Cannot apply binary operation ({}) on 2 expressions of different types ({}) and ({})!",
          m_token.to_string(), left->type()->to_string(), right->type()->to_string()
        ),
        right->token().line(),
        right->token().column()
      );
    }
    const auto op = m_ops[i];  
    if (!validation::is_valid_binary_operand(op, left->type(), right->type())) {
      std::string_view left_type = left->type()->get_underlying_type()->id();
      if (left_type == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)]) {
        throw SemanticException(
          SEMANTIC_ERROR::SE_INVALID_OP,
          std::format(
            "Semantic error: Cannot apply binary operation ({}) on 2 boolean expressions !",
            right->token().to_string()
          ),
          right->token().line(),
          right->token().column()
        );
      } else {
        throw SemanticException(
          SEMANTIC_ERROR::SE_INVALID_OP,
          std::format(
            "Semantic error: Cannot apply binary operation ({}) on 2 expressions of underlying type '{}' !",
            right->token().to_string(), left_type
          ),
          right->token().line(),
          right->token().column()
        );
      }
    }
  }
}

void LiteralExpression::validate()
{
  this->m_expr_type = m_constant->type();
}

const Type * ArraySelector::apply(const Type *type)
{
  if(type->category() != TYPE_CAT::TC_ARRAY)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_TYPE,
      std::format(
        "Semantic error: At ({}), type ({}) is not indexable ! Expected an array !",
        m_token.to_string(), type->to_string()
      ),
      m_token.line(),
      m_token.column()
    );
  }

  const Array *arr = static_cast<const Array *>(type);
  
  if(arr->index_types().size() != m_indices.size())
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_INDEX,
      std::format(
        "Semantic error: At ({}), array type ({}) requires {} indices ! Found {} indices !",
        m_token.to_string(), type->to_string(), arr->index_types().size(), m_indices.size()
      ),
      m_token.line(),
      m_token.column()
    );
  }

  for(int i = 0;i < m_indices.size();++i)
  {
    auto itype = m_indices[i]->type()->get_underlying_type();
    
    auto vtype = arr->index_types()[i]->get_underlying_type();
    
    if(vtype != itype)
    {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_INDEX,
        std::format(
          "Semantic error: At ({}), array type ({}) requires the index-{} to be of type ({}) ! Found {} !",
          m_token.to_string(), type->to_string(), i+1, arr->index_types()[i]->to_string(), m_indices[i]->type()->to_string()
        ),
        m_token.line(),
        m_token.column()
      );
    }
  }

  return arr->element_type();
}

const Type* FieldSelector::apply(const Type* type)
{
  if(type->category() != TYPE_CAT::TC_RECORD)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_TYPE,
      std::format(
        "Semantic error: At ({}), Field name '{}' does not exists for non-record type ({}) ! Expected a record !",
        m_token.to_string(), m_field, type->to_string()
      ),
      m_token.line(),
      m_token.column()
    );
  }

  const Record* rec = static_cast<const Record*>(type);
  auto it = rec->attributes().find(m_field);
  if(it == rec->attributes().end())
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_FIELD_NAME,
      std::format(
        "Semantic error: Field name '{}' does not exists for record type ({}) !",
        m_field, type->to_string()
      ),
      m_token.line(),
      m_token.column()
    );
  }

  return it->second.type();
}


void VariableAccess::validate()
{
  const Type* current = m_base_var->type()->get_underlying_type();
  
  for(const auto& s : m_selectors)
  {
    current = s->apply(current);
  }

  this->m_expr_type = current;
}


void CompoundStatement::validate()
{
  for(const auto& stmt : m_statements)
  {
    stmt->validate();
  }
}

void AssignmentStatement::validate()
{
  if (!validation::is_basic_type(m_lhs->type()) && !validation::is_enum_type(m_lhs->type())) {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_CALL,
      std::format(
        "Semantic error: In ({}), cannot assign an expressions to a variable of type ({})! Expected Ints, Reals, Chars, Bools, Enums or Strings !",
        m_token.to_string(), m_lhs->type()->to_string()
      ),
      m_token.line(),
      m_token.column()
    );
  }
  if (!validation::types_compatible(m_lhs->type(), m_rhs->type())) {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_ASSIGN,
      std::format(
        "Semantic error: Variable access ({}) of type ({}) cannot be assigned value of different type ({}) !",
        m_lhs->base_var()->to_string(), m_lhs->type()->to_string(), m_rhs->type()->to_string()
      ),
      m_token.line(),
      m_token.column()
    );
  }
}

void LabeledStatement::validate()
{}

void WriteStatement::validate()
{
  for(const auto &exp : m_arguments)
  {
    if (!validation::is_io_compatible(exp->type())) {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_CALL,
        std::format(
          "Semantic error: In ({}), cannot write an expressions of type ({})! Expected Ints, Reals, Chars, Strings, Bools or array of Chars !",
          m_token.to_string(), exp->type()->to_string()
        ),
        m_token.line(),
        m_token.column()
      );
    }
  }
}

void ReadStatement::validate()
{
  for(const auto &exp : m_arguments)
  {
    if (!validation::is_io_compatible(exp->type())) {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_CALL,
        std::format(
          "Semantic error: In ({}), cannot read a variable of type ({})! Expected Ints, Reals, Chars, Strings, Bools or array of Chars !",
          m_token.to_string(), exp->type()->to_string()
        ),
        m_token.line(),
        m_token.column()
      );
    }
  }
}

void WhileStatement::validate()
{
  if (!validation::is_boolean_type(this->m_condition->type())) {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_COND,
      std::format(
        "Semantic error: In ({}), the while loop's condition is required to be a boolean ! Found {} !",
        this->m_token.to_string(), this->m_condition->type()->to_string()
      ),
      m_token.line(),
      m_token.column()
    );
  }
}

void RepeatStatement::validate()
{
  if (!validation::is_boolean_type(this->m_until_expr->type())) {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_COND,
      std::format(
        "Semantic error: In ({}), the repeat loop's condition is required to be a boolean ! Found {} !",
        this->m_token.to_string(), this->m_until_expr->type()->to_string()
      ),
      m_token.line(),
      m_token.column()
    );
  }
}

void ForStatement::validate()
{
  auto type_id = this->m_loop_var->type()->get_underlying_type()->id();
  if(
    CONST_CAT_NAMES[int(this->m_start.category())] != type_id ||
    m_start.category() != m_end.category()
  )
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_TYPE,
      std::format(
        "Semantic error: In ({}), the for loop expects the bounds and the loop variable to be of the same type ! Found ({}) for the start, ({}) for the end and ({}) for the variable !",
        m_token.to_string(), CONST_CAT_NAMES[int(m_start.category())], CONST_CAT_NAMES[int(m_end.category())], m_loop_var->type()->to_string()
      ),
      m_token.line(),
      m_token.column()
    );
  }
  Int a,b;
  switch (m_start.category())
  {
    case CONST_CAT::CC_CHAR:
      a = m_start.get<char>();
      b = m_end.get<char>();
    break;
    case CONST_CAT::CC_BOOL:
      a = m_start.get<bool>();
      b = m_end.get<bool>();
    break;
    case CONST_CAT::CC_ENUM:
    case CONST_CAT::CC_INT:
      a = m_start.get<Int>();
      b = m_end.get<Int>();
    break;
    default:
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_SUBRANGE,
        std::format(
            "Semantic error: In ({}), bounds can be made only using chars, enums or ints (constants) ! Found '{}' !",
            m_token.to_string(), CONST_CAT_NAMES[int(m_start.category())]
          ),
        m_token.line(),
        m_token.column()
      );
  }
  if(m_increasing && a > b)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_SUBRANGE,
      std::format(
        "Semantic error: In ({}), the lower bound should be less than the upper bound ! Found {}->{} !",
        m_token.to_string(), a, b
      ),
      m_token.line(),
      m_token.column()
    );
  }
  else if(!m_increasing && a < b)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_SUBRANGE,
      std::format(
        "Semantic error: In ({}), the lower bound should be greater than the upper bound ! Found {}->{} !",
        m_token.to_string(), a, b
      ),
      m_token.line(),
      m_token.column()
    );
  }
}

void IfStatement::validate()
{
  if (!validation::is_boolean_type(this->m_condition->type())) {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_COND,
      std::format(
        "Semantic error: In ({}), the condition of if statement is required to be a boolean ! Found {} !",
        this->m_token.to_string(), this->m_condition->type()->to_string()
      ),
      m_token.line(),
      m_token.column()
    );
  }
}

void CaseStatement::validate()
{
  auto type_id = this->m_selector->type()->get_underlying_type()->id();
  if (!validation::is_case_selector_type(this->m_selector->type())) {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_TYPE,
      std::format(
        "Semantic error: In ({}), case statement's expression is of type ({}) ! Expected Int, Char, Bool or Enum type !",
        m_token.to_string(), this->m_selector->type()->to_string()
      ),
      m_token.line(),
      m_token.column()
    );
  }

  auto check = [&]<typename T>(CONST_CAT cat)
  {
    std::unordered_set<T> values;
    for(const auto& alt : this->m_alternatives){
      for(int index = 0;index < alt.labels().size();++index){
        const auto current = alt.labels()[index];
        if(!std::holds_alternative<T>(current.value()))
        {
          throw SemanticException(
            SEMANTIC_ERROR::SE_INVALID_TYPE,
            std::format(
              "Semantic error: In ({}), case statement's label ({}) is of type ({}) ! Expected {} !",
              alt.token().to_string(), current.to_string(), CONST_CAT_NAMES[int(current.category())], CONST_CAT_NAMES[int(cat)]
            ),
            alt.token().line(),
            alt.token().column()
          );
        }
        if(values.contains(current.get<T>()))
        {
          throw SemanticException(
            SEMANTIC_ERROR::SE_INVALID_TYPE,
            std::format(
              "Semantic error: In ({}), case statement's label ({}) is duplicated !",
              alt.token().to_string(), current.to_string()
            ),
            alt.token().line(),
            alt.token().column()
          );
        }
        values.insert(current.get<T>());
      }
    }
  };

  for(const auto& alt : m_alternatives)
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
          m_token.to_string(), this->m_selector->type()->to_string()
        ),
        m_token.line(),
        m_token.column()
      );
    }
  }
}

// How will we deal with references ? what about arrays ?
void validate(const Function *func, const std::vector<std::unique_ptr<Expression>> &args, Lexeme m_token)
{
  const auto& params = func->type()->args();  
  if(params.size() != args.size())
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_CALL,
      std::format(
        "Semantic error: At ({}), found {} arguments ! Expected {} arguments !",
        m_token.to_string(), args.size(), params.size()
      ),
      m_token.line(),
      m_token.column()
    );
  }

  for(int i = 0;i < params.size();++i)
  {
    const Type* ptype = params[i].type()->get_underlying_type();
    const Type* atype = args[i]->type()->get_underlying_type();
    if(params[i].is_ref())
    {
      auto tmp = dynamic_cast<VariableAccess*>(args[i].get());
      if(!tmp)
      {
        throw SemanticException(
          SEMANTIC_ERROR::SE_INVALID_CALL,
          std::format(
            "Semantic error: At ({}), the {}-argument is not a variable but an expression !",
            m_token.to_string(), i+1
          ),
          m_token.line(),
          m_token.column()
        );
      }
    }
    if(ptype != atype)
    {
      throw SemanticException(
        SEMANTIC_ERROR::SE_INVALID_CALL,
        std::format(
          "Semantic error: At ({}), the {}-argument is of type ({}) ! Expected type ({}) !",
          m_token.to_string(), i+1, atype->to_string(), ptype->to_string()
        ),
        m_token.line(),
        m_token.column()
      );
    }
  }
}

void ProcedureCall::validate()
{
  if(m_procedure->type()->return_type() != nullptr)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_CALL,
      std::format(
        "Semantic error: At ({}), invalid procedure with return-type ({}) !",
        m_token.to_string(), m_procedure->type()->return_type()->to_string()
      ),
      m_token.line(),
      m_token.column()
    );
  }
  pascal_compiler::validate(m_procedure, m_args, m_token);
}

void FunctionCall::validate()
{
  if(m_function->type()->return_type() == nullptr)
  {
    throw SemanticException(
      SEMANTIC_ERROR::SE_INVALID_CALL,
      std::format(
        "Semantic error: At ({}), invalid function with no return-type !",
        m_token.to_string()
      ),
      m_token.line(),
      m_token.column()
    );
  }
  pascal_compiler::validate(m_function, m_args, m_token);
  this->m_expr_type = m_function->type()->return_type();
}

}