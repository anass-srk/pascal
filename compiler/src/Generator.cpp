#include "Generator.hpp"
#include "Ast.hpp"
#include "Semantics.hpp"
#include "Visitor.hpp"
#include <iostream>
#include <utility>

namespace pascal_compiler {

size_t Generator::get_type_size(const Type *t) {
  if (t->category() == TYPE_CAT::TC_BASIC) {
    if (t->id() == CONST_CAT_NAMES[int(CONST_CAT::CC_INT)])
      return 8;
    if (t->id() == CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)])
      return 8;
    if (t->id() == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)])
      return 1;
    if (t->id() == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)])
      return 1;
    return -1;
  }

  if (t->category() == TYPE_CAT::TC_ENUM) {
    process_type(t);
    return 8; // It's basically an Int
  }
  if (t->category() == TYPE_CAT::TC_FUNCTION)
    return 0;

  auto it = m_type_sizes.find(t);
  if (it != m_type_sizes.end())
    return it->second;
  process_type(t);
  return m_type_sizes.at(t);
}

void Generator::process_type(const Type *type) {

  switch (type->category()) {
  case TYPE_CAT::TC_BASIC:
  case TYPE_CAT::TC_FUNCTION:
    return;
  }

  if (m_type_sizes.contains(type) || m_type_lens.contains(type))
    return;

  if (type->category() == TYPE_CAT::TC_ENUM) {
    m_type_lens.emplace(type, static_cast<const Enum *>(type)->values().size());
    return;
  }

  if (type->category() == TYPE_CAT::TC_SUBRANGE) {
    m_type_sizes.emplace(type, get_type_size(type->get_underlying_type()));
    const auto sub = static_cast<const Subrange *>(type);
    m_type_lens.emplace(type, (sub->end() - sub->start() + 1));
    return;
  }
  if (type->category() == TYPE_CAT::TC_ARRAY) {
    const Array *arr = static_cast<const Array *>(type);

    size_t size = get_type_size(arr->element_type());

    for (auto index_type : arr->index_types()) {
      process_type(index_type);
      size *= m_type_lens.at(index_type);
    }
    m_type_sizes.emplace(type, size);
    return;
  }

  // Record type

  const Record *rec = static_cast<const Record *>(type);

  auto& offsets = m_record_offsets.try_emplace(rec).first->second;

  size_t size = 0;
  for (const auto &[_, var] : rec->attributes()) {
    offsets.emplace(var.id(), size);
    size += get_type_size(var.type());
  }
  m_type_sizes.emplace(type, size);
}

using namespace pascal_vm;

void Generator::process_context(const Context &ctx) {

  size_t addr = vm.add_jmp(OPCODE::JMP, 0);

  for (const auto &[_, c] : ctx.m_consts) {
    // only strings are stored
    if (c.category() != CONST_CAT::CC_CONST_STRING) continue;
    const auto str = c.get<std::string>();

    m_global_const_info.emplace(
      c.id(), Info{.location = vm.get_current_location(), .size = str.length()+1}
    );
    vm.add_string(str);
  }

  vm.conf_jmp(addr, vm.get_current_location());

  size_t loc = 0;

  for (const auto &[_, v] : ctx.m_vars) {
    const size_t size = get_type_size(v.type());
    m_global_var_info.emplace(&v, Info{.location = loc, .size = size});
    loc += size;
  }
  vm.add_resize_stack(loc);

  // deal with functions later

  ctx.body->accept(*this, ctx);
  
  vm.add_halt();
}

void Generator::visit(const CompoundStatement &stmts, const Context &ctx) {
  for (const auto &stmt : stmts.statements()) {
    stmt->accept(*this, ctx);
  }
}

void Generator::visit(const AssignmentStatement &stmt, const Context &ctx) {
  bool by_value = m_by_value;
  m_by_value = false;
  stmt.var()->accept(*this, ctx);
  m_by_value = by_value;
  // Now we have the address of the variable in the top of the stack
  stmt.expr()->accept(*this, ctx);
  // Now the value is in the top of the stack
  // store the value depending on the type (same type for both at the moment, basic types only)
  const auto cat = get_catagory(stmt.expr()->type());
  
  switch (cat) {
    case CONST_CAT::CC_INT:
    case CONST_CAT::CC_REAL:
    case CONST_CAT::CC_ENUM:
      vm.add_move(OPCODE::STORE_Q);
      break;
    case CONST_CAT::CC_BOOL:
    case CONST_CAT::CC_CHAR:
      vm.add_move(OPCODE::STORE_B);
      break;
  }
}

void Generator::visit(const LiteralExpression &expr, const Context &ctx) {
  const auto *c = expr.constant();
  switch (c->category()) {
  case CONST_CAT::CC_BOOL:
    vm.add_push(OPCODE::PUSH_B, c->get<bool>());
    break;
  case CONST_CAT::CC_CHAR:
    vm.add_push(OPCODE::PUSH_B, c->get<char>());
    break;
  case CONST_CAT::CC_INT:
  case CONST_CAT::CC_ENUM:
    vm.add_push(OPCODE::PUSH_Q, c->get<Int>());
    break;
  case CONST_CAT::CC_REAL:
    vm.add_push(OPCODE::PUSH_Q, c->get<Real>());
    break;
  case CONST_CAT::CC_CONST_STRING: {
    const auto it = m_const_info.find(c->id());
    if (it != m_const_info.end()) {
      vm.add_push_frame_pointer();
      vm.add_push<Int>(OPCODE::PUSH_Q, it->second.location);
      vm.add_arithmetic_op(OPCODE::ADD_I);
    } else
      vm.add_push<Int>(OPCODE::PUSH_Q, m_global_const_info.at(c->id()).location);
  } break;
  }
}

void Generator::visit(const UnaryExpression &expr, const Context &ctx) {
  expr.operand()->accept(*this, ctx);
  // Now the data is pushed
  if(expr.op() == UnaryOp::Plus) return; // No operation
  if(expr.op() == UnaryOp::Not) {
    vm.add_not_op();
    return;
  }
  // Minus
  const auto cat = get_catagory(expr.operand()->type());
  switch (cat){ 
  case CONST_CAT::CC_INT:
    vm.add_push(OPCODE::PUSH_Q, Int(-1));
    vm.add_arithmetic_op(OPCODE::MUL_I);
    break;
  case CONST_CAT::CC_REAL:
    vm.add_push(OPCODE::PUSH_Q, Real(-1));
    vm.add_arithmetic_op(OPCODE::MUL_R);
    break;
  case CONST_CAT::CC_CHAR:  
    vm.add_push(OPCODE::PUSH_B, char(-1));
    vm.add_arithmetic_op(OPCODE::MUL_C);
    break;
  }
}

void Generator::visit(const BinaryExpression &expr, const Context &ctx) {
  // Int, Real, Char or Bool or Enum
  expr.left()->accept(*this, ctx);
  expr.right()->accept(*this, ctx);
  // Both on the stack with the same type
  const auto cat = get_catagory(expr.left()->type());
  switch (cat) {
    case CONST_CAT::CC_ENUM:
    case CONST_CAT::CC_INT:
      vm.add_cmp(OPCODE::CMP_I);
      break;
    case CONST_CAT::CC_REAL:
      vm.add_cmp(OPCODE::CMP_R);
      break;
    case CONST_CAT::CC_CHAR:
    case CONST_CAT::CC_BOOL:
      vm.add_cmp(OPCODE::CMP_C);
      break;
  }
  
  switch(expr.op()) {
  // only cmp ops, the other cases are not possible for binary expressions
  case RelOp::Eq:
    vm.add_flag(OPCODE::EQ);
    break;
  case RelOp::Ne:
    vm.add_flag(OPCODE::NE);
    break;
  case RelOp::Lt:
    vm.add_flag(OPCODE::LT);
    break;
  case RelOp::Le:
    vm.add_flag(OPCODE::LE);
    break;
  case RelOp::Gt:
    vm.add_flag(OPCODE::GT);
    break;
  case RelOp::Ge:
    vm.add_flag(OPCODE::GE);
    break;
  }

}

void Generator::visit(const NExpression &expr, const Context &ctx) {
  const auto& exprs = expr.exprs();
  exprs[0]->accept(*this, ctx);
  // All of them have the same type
  const auto cat = get_catagory(exprs[0]->type());
  OPCODE ADD, SUB, MUL, DIV;
  switch (cat) {
  // enums are not possible
  case CONST_CAT::CC_INT:
    ADD = OPCODE::ADD_I;
    SUB = OPCODE::SUB_I;
    MUL = OPCODE::MUL_I;
    DIV = OPCODE::DIV_I;
    break;
  case CONST_CAT::CC_REAL:
    ADD = OPCODE::ADD_R;
    SUB = OPCODE::SUB_R;
    MUL = OPCODE::MUL_R;
    DIV = OPCODE::DIV_R;
    break;
  case CONST_CAT::CC_CHAR:
    ADD = OPCODE::ADD_C;
    SUB = OPCODE::SUB_C;
    MUL = OPCODE::MUL_C;
    DIV = OPCODE::DIV_C;
    break;
  case CONST_CAT::CC_BOOL:
    break;
  }

  const auto& ops = expr.ops();
  for(int i = 1;i < exprs.size();++i) {
    exprs[i]->accept(*this, ctx);
    
    switch(ops[i-1]) {
    case ALOp::Add:
      vm.add_arithmetic_op(ADD);
      break;
    case ALOp::Sub:
      vm.add_arithmetic_op(SUB);
      break;
    case ALOp::Or:
      vm.add_logic_op(OPCODE::OR);
      break;
    case ALOp::Mul:
      vm.add_arithmetic_op(MUL);
      break;
    case ALOp::Div:
      vm.add_arithmetic_op(DIV);
      break;
    case ALOp::And:
      vm.add_logic_op(OPCODE::AND);
      break;
    }
    
  }
}

void Generator::visit(const VariableAccess& var, const Context& ctx) {

  bool by_value = m_by_value;
  m_by_value = true;

  const auto it = m_global_var_info.find(var.base_var());
  
  if(it != m_global_var_info.end()) {
    vm.add_push(OPCODE::PUSH_Q, it->second.location);
  } else {
    vm.add_push(OPCODE::PUSH_Q, m_var_info.at(var.base_var()).location);
    vm.add_push_frame_pointer();
    vm.add_arithmetic_op(OPCODE::ADD_I);
  }

  for(const auto& selector : var.selectors()) {
    selector->accept(*this,ctx); // pushes offset for record/array
    vm.add_arithmetic_op(OPCODE::ADD_I);
  }

  m_by_value = by_value;

  // Basic types only
  if(m_by_value) {
    const auto cat = get_catagory(var.type());
    switch(cat) {
      case CONST_CAT::CC_INT:
      case CONST_CAT::CC_REAL:
      case CONST_CAT::CC_ENUM:
        vm.add_load(OPCODE::LOAD_Q);
        break;
      case CONST_CAT::CC_BOOL:
      case CONST_CAT::CC_CHAR:
        vm.add_load(OPCODE::LOAD_B);
        break;
      }
  }

}

void Generator::visit(const FieldSelector& selector, const Context& ctx) {
  const Record* type = static_cast<const Record*>(selector.applied_on());
  vm.add_push(OPCODE::PUSH_Q, m_record_offsets.at(type).at(selector.field()));
}

void Generator::visit(const ArraySelector& selector, const Context& ctx) {
  const Array* type = static_cast<const Array*>(selector.applied_on());
  size_t current_size = get_type_size(type->element_type());
  for(const auto& exp : selector.indices()) {
    exp->accept(*this, ctx); // pushes value
    vm.add_push(OPCODE::PUSH_Q, current_size);
    vm.add_arithmetic_op(OPCODE::MUL_I);
    current_size *= m_type_lens.at(exp->type());
  }
}

void Generator::visit(const WriteStatement& stmt, const Context& ctx) {
  for(const auto& expr : stmt.args()) {
    expr->accept(*this, ctx); // pushes value or addr for const string
    if(expr->type()->get_underlying_type()->category() == TYPE_CAT::TC_ARRAY) {
      // An array of chars (validated)
      vm.add_write(OPCODE::WRITE_S);
      continue;
    }
    switch (get_catagory(expr->type())) {
      case CONST_CAT::CC_INT:
      case CONST_CAT::CC_ENUM:
        vm.add_write(OPCODE::WRITE_I);
        break;
      case CONST_CAT::CC_REAL:
        vm.add_write(OPCODE::WRITE_R);
        break;
      case CONST_CAT::CC_CONST_STRING:
        vm.add_write(OPCODE::WRITE_CONST_S);
        break;
      case CONST_CAT::CC_BOOL:
        vm.add_write(OPCODE::WRITE_B);
        break;
      case CONST_CAT::CC_CHAR:
        vm.add_write(OPCODE::WRITE_C);
        break;
    }
  }
}

void Generator::visit(const ReadStatement& stmt, const Context& ctx) {
  bool by_value = m_by_value;
  m_by_value = false;

  for(const auto& var : stmt.args()) {
    var->accept(*this, ctx); // pushed address
    if(var->type()->get_underlying_type()->category() == TYPE_CAT::TC_ARRAY) {
      // array of chars validated
      const Array* arr = static_cast<const Array*>(var->type()->get_underlying_type());
      size_t len = 1;
      for(const auto& i : arr->index_types()) {
        len *= m_type_sizes.at(i);
      }
      vm.add_push(OPCODE::PUSH_Q, Int(len));
      vm.add_read_string();
      continue;
    }
    switch (get_catagory(var->type())) {
      case CONST_CAT::CC_INT:
      case CONST_CAT::CC_ENUM:
        vm.add_write(OPCODE::READ_I);
        break;
      case CONST_CAT::CC_REAL:
        vm.add_write(OPCODE::READ_R);
        break;
      case CONST_CAT::CC_BOOL:
        vm.add_write(OPCODE::READ_B);
        break;
      case CONST_CAT::CC_CHAR:
        vm.add_write(OPCODE::READ_C);
        break;
    }
  }

  m_by_value = by_value;
}


} // namespace pascal_compiler