#include "Generator.hpp"
#include "Ast.hpp"
#include "Semantics.hpp"
#include "Visitor.hpp"
#include <iostream>
#include <utility>
#include <variant>

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

  m_const_info.emplace_back();
  for (const auto &[_, c] : ctx.m_consts) {
    // only strings are stored
    if (c.category() != CONST_CAT::CC_CONST_STRING) continue;
    const auto str = c.get<std::string>();

    m_const_info.back().emplace(
      c.id(), Info{.location = Int(vm.get_current_location()), .size = str.length()+1}
    );
    vm.add_string(str);
  }

  for (auto c : ctx.m_unamed_const_strings) {
    // only strings are stored
    const auto str = c->get<std::string>();
    m_const_info.back().emplace(
      c, Info{.location = Int(vm.get_current_location()), .size = str.length() + 1}
    );
    vm.add_string(str);
  }

  vm.conf_jmp(addr, vm.get_current_location());

  size_t loc = 0;
  m_var_info.emplace_back();
  for (const auto &[name, v] : ctx.m_vars) {
    const size_t size = get_type_size(v.type());
    m_var_info.back().emplace(name, Info{.location = Int(loc), .size = size});
    loc += size;
  }
  vm.add_resize_stack(loc);

  m_label_locations.emplace_back();
  m_goto_map.emplace_back();
  m_func_locations.emplace_back();

  const auto funcs_addr = vm.add_jmp(OPCODE::JMP, 0);

  for (const auto& [name, f] : ctx.m_funcs) {
    m_func_locations.back().emplace(f.id(), vm.get_current_location());
    process_context(f);
  }

  vm.conf_jmp(funcs_addr, vm.get_current_location());

  ctx.body->accept(*this, ctx);

  // Process gotos

  for(const auto& [addr, label] :  m_goto_map.back()) {
    vm.conf_jmp(addr, m_label_locations.back().at(label));
  }
  
  vm.add_halt();
}

void Generator::process_context(const Function& f) {
  
  size_t addr = vm.add_jmp(OPCODE::JMP, 0);

  m_const_info.emplace_back();
  for (const auto &[_, c] : f.ctx()->m_consts) {
    // only strings are stored
    if (c.category() != CONST_CAT::CC_CONST_STRING) continue;
    const auto str = c.get<std::string>();

    m_const_info.back().emplace(
      c.id(), Info{.location = Int(vm.get_current_location()), .size = str.length()+1}
    );
    vm.add_string(str);
  }

  for (auto c : f.ctx()->m_unamed_const_strings) {
    // only strings are stored
    const auto str = c->get<std::string>();
    m_const_info.back().emplace(
      c, Info{.location = Int(vm.get_current_location()), .size = str.length() + 1}
    );
    vm.add_string(str);
  }

  vm.conf_jmp(addr, vm.get_current_location());

  m_var_info.emplace_back();

  Int acc_size = 0;
  for(int i = f.type()->args().size()-1;i >= 0;--i) {
    const auto& arg = f.type()->args()[i];
    const Int type_size = get_type_size(arg.type());
    m_var_info.back().emplace(
      arg.id(), Info{.location= - type_size - 16 - acc_size, .size=size_t(type_size) }
    );
    acc_size += type_size;
  }

  if(f.type()->return_type()) {
    const Int type_size = get_type_size(f.type()->return_type());
    m_var_info.back().emplace(
      f.id(), Info{.location= - type_size - 16 - acc_size, .size=size_t(type_size) }
    );
  }

  size_t loc = 0;
  for (const auto &[name, v] : f.ctx()->m_vars) {
    if(m_var_info.back().contains(name)) continue;
    const size_t size = get_type_size(v.type());
    m_var_info.back().emplace(name, Info{.location = Int(loc), .size = size});
    loc += size;
  }
  vm.add_resize_stack(loc);

  m_label_locations.emplace_back();
  m_goto_map.emplace_back();
  m_func_locations.emplace_back();

  const auto funcs_addr = vm.add_jmp(OPCODE::JMP, 0);

  for (const auto &[name, g] : f.ctx()->m_funcs) {
    m_func_locations.back().emplace(g.id(), vm.get_current_location());
    process_context(g);
  }

  vm.conf_jmp(funcs_addr, vm.get_current_location());

  f.ctx()->body->accept(*this, *f.ctx());

  // Process gotos

  for (const auto &[addr, label] : m_goto_map.back()) {
    vm.conf_jmp(addr, m_label_locations.back().at(label));
  }

  vm.add_return(loc);

  m_var_info.pop_back();
  m_const_info.pop_back();
  m_label_locations.pop_back();
  m_goto_map.pop_back();
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
    for(int i = m_const_info.size()-1;i >= 0;--i) {
      if(const auto it = m_const_info[i].find(c->id()); it != m_const_info[i].end()) {
        vm.add_push<Int>(OPCODE::PUSH_Q, it->second.location);
        break;
      }
      if(const auto it = m_const_info[i].find(c); it != m_const_info[i].end()) {
        vm.add_push<Int>(OPCODE::PUSH_Q, it->second.location);
        break;
      }
    }
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

  int i = m_var_info.size()-1;
  while(i >= 1) {
    const auto it = m_var_info[i].find(var.base_var()->id());
    if(it != m_var_info[i].end()) {
      vm.add_push(OPCODE::PUSH_Q, it->second.location);
      vm.add_push_frame_pointer();
      vm.add_arithmetic_op(OPCODE::ADD_I);
      break;
    }
    --i;
  }

  if(i == 0) {
    vm.add_push(OPCODE::PUSH_Q, m_var_info.front().at(var.base_var()->id()).location);
  }

  for(const auto& selector : var.selectors()) {
    selector->accept(*this,ctx); // pushes offset for record/array and adds
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
  vm.add_arithmetic_op(OPCODE::ADD_I);
}

void Generator::visit(const ArraySelector& selector, const Context& ctx) {
  const Array* type = static_cast<const Array*>(selector.applied_on());
  size_t current_size = get_type_size(type->element_type());
  int i = 0;
  for(const auto& exp : selector.indices()) {
    exp->accept(*this, ctx); // pushes value
    vm.add_push(OPCODE::PUSH_Q, current_size);
    vm.add_arithmetic_op(OPCODE::MUL_I);
    vm.add_arithmetic_op(OPCODE::ADD_I);
    current_size *= m_type_lens.at(type->index_types()[i]);
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
        vm.add_read(OPCODE::READ_I);
        break;
      case CONST_CAT::CC_REAL:
        vm.add_read(OPCODE::READ_R);
        break;
      case CONST_CAT::CC_BOOL:
        vm.add_read(OPCODE::READ_B);
        break;
      case CONST_CAT::CC_CHAR:
        vm.add_read(OPCODE::READ_C);
        break;
    }
  }

  m_by_value = by_value;
}
void Generator::visit(const LabeledStatement& stmt, const Context &ctx) {
  m_label_locations.back().emplace(stmt.label(), vm.get_current_location());
  stmt.statement()->accept(*this, ctx);
}

void Generator::visit(const GotoStatement& stmt, const Context& ctx) {
  m_goto_map.back().emplace(vm.add_jmp(OPCODE::JMP, 0), stmt.label());
}

void Generator::visit(const IfStatement& stmt, const Context& ctx) {
  stmt.condition()->accept(*this, ctx); // puses boolean
  const size_t else_addr = vm.add_jmp(OPCODE::JMP_FALSE, 0);
  
  stmt.then_stmt()->accept(*this, ctx);
  const size_t end_addr = vm.add_jmp(OPCODE::JMP, 0);

  vm.conf_jmp(else_addr, vm.get_current_location());
  stmt.else_stmt()->accept(*this, ctx);
  vm.conf_jmp(end_addr, vm.get_current_location());
}

void Generator::visit(const WhileStatement& stmt, const Context& ctx) {
  const size_t loop_beg = vm.get_current_location();
  stmt.condition()->accept(*this, ctx);
  const size_t loop_end = vm.add_jmp(OPCODE::JMP_FALSE, 0);

  stmt.body()->accept(*this, ctx);
  vm.add_jmp(OPCODE::JMP, loop_beg);
  
  vm.conf_jmp(loop_end, vm.get_current_location());
}

void Generator::visit(const RepeatStatement& stmt, const Context& ctx) {
  const size_t loop_beg = vm.get_current_location();
  
  for(const auto& s : stmt.body()) {
    s->accept(*this, ctx);
  }

  stmt.condition()->accept(*this, ctx);
  vm.add_jmp(OPCODE::JMP_FALSE, loop_beg);
}


void Generator::visit(const ForStatement& stmt, const Context& ctx) {

  const auto cat = get_catagory(stmt.var()->type());

  { // Assignment
    bool by_value = m_by_value;
    m_by_value = false;
    stmt.var()->accept(*this, ctx);
    m_by_value = by_value;
    // Now we have the address of the variable in the top of the stack
    
    switch (cat) {
    case CONST_CAT::CC_INT:
    case CONST_CAT::CC_ENUM:
      vm.add_push(OPCODE::PUSH_Q, stmt.start().get<Int>());
      vm.add_move(OPCODE::STORE_Q);
      break;
    case CONST_CAT::CC_BOOL:
      vm.add_push(OPCODE::PUSH_B, stmt.start().get<bool>());
      vm.add_move(OPCODE::STORE_B);
      break;
    case CONST_CAT::CC_CHAR:
      vm.add_push(OPCODE::PUSH_B, stmt.start().get<char>());
      vm.add_move(OPCODE::STORE_B);
      break;
    }
  }

  const size_t loop_beg = vm.get_current_location();
  { // condition
    bool by_value = m_by_value;
    m_by_value = true;
    stmt.var()->accept(*this, ctx);
    m_by_value = by_value;
    // Now we have the value of the variable in the top of the stack

    switch (cat) {
    case CONST_CAT::CC_INT:
    case CONST_CAT::CC_ENUM:
      vm.add_push(OPCODE::PUSH_Q, stmt.end().get<Int>());
      vm.add_cmp(OPCODE::CMP_I);
      break;
    case CONST_CAT::CC_BOOL:
      vm.add_push(OPCODE::PUSH_B, stmt.end().get<bool>());
      vm.add_cmp(OPCODE::CMP_C);
      break;
    case CONST_CAT::CC_CHAR:
      vm.add_push(OPCODE::PUSH_B, stmt.end().get<char>());
      vm.add_cmp(OPCODE::CMP_C);
      break;
    }
    vm.add_flag(stmt.increasing() ? OPCODE::LE : OPCODE::GE);
  }


  const size_t loop_end = vm.add_jmp(OPCODE::JMP_FALSE, 0);

  stmt.body()->accept(*this, ctx);

  { // Assignment at end of the loop
    bool by_value = m_by_value;
    m_by_value = false;
    stmt.var()->accept(*this, ctx);
    m_by_value = by_value;
    // Now we have the address of the variable in the top of the stack

    by_value = m_by_value;
    m_by_value = true;
    stmt.var()->accept(*this, ctx);
    m_by_value = by_value;
    // push the value of the variable to the stack

    switch (cat) {
    case CONST_CAT::CC_INT:
    case CONST_CAT::CC_ENUM:
      vm.add_push(OPCODE::PUSH_Q, stmt.increasing() ? Int(1) : Int(-1));
      vm.add_arithmetic_op(OPCODE::ADD_I);
      vm.add_move(OPCODE::STORE_Q);
      break;
    case CONST_CAT::CC_BOOL:
    case CONST_CAT::CC_CHAR:
      vm.add_push(OPCODE::PUSH_B, stmt.increasing() ? char(1) : char(-1));
      vm.add_arithmetic_op(OPCODE::ADD_C);
      vm.add_move(OPCODE::STORE_B);
      break;
    }
  }

  vm.add_jmp(OPCODE::JMP, loop_beg);

  vm.conf_jmp(loop_end, vm.get_current_location());
}

void _get_int(VM& vm, const Const& c) {
  vm.add_duplicate(OPCODE::DUPL_Q);
  vm.add_push(OPCODE::PUSH_Q, c.get<Int>());
  vm.add_cmp(OPCODE::CMP_I);
  vm.add_flag(OPCODE::EQ);
}

void _get_char(VM &vm, const Const &c) {
  vm.add_duplicate(OPCODE::DUPL_B);
  vm.add_push(OPCODE::PUSH_B, c.get<char>());
  vm.add_cmp(OPCODE::CMP_C);
  vm.add_flag(OPCODE::EQ);
}

void _get_bool(VM &vm, const Const &c) {
  vm.add_duplicate(OPCODE::DUPL_B);
  vm.add_push(OPCODE::PUSH_B, c.get<bool>());
  vm.add_cmp(OPCODE::CMP_C);
  vm.add_flag(OPCODE::EQ);
}

void Generator::visit(const CaseStatement& stmt, const Context& ctx) {
  stmt.selector()->accept(*this, ctx);

  auto push = &_get_int;
  OPCODE POP;

  switch (get_catagory(stmt.selector()->type())) {
    case CONST_CAT::CC_INT:
    case CONST_CAT::CC_ENUM:
      push = &_get_int;
      POP = OPCODE::POP_Q;
      break;
    case CONST_CAT::CC_CHAR:
      push = &_get_char;
      POP = OPCODE::POP_B;
      break;
    case CONST_CAT::CC_BOOL:
      push = &_get_bool;
      POP = OPCODE::POP_B;
      break;
  }

  for(const auto& alt : stmt.alternatives()) {
    std::vector<size_t> jmp_locs;
    for(int i = 0;i < alt.labels().size()-1;++i) {
      const auto& c = alt.labels()[i];
      push(vm, c);
      jmp_locs.push_back(vm.add_jmp(OPCODE::JMP_TRUE, 0));
    }
    push(vm, alt.labels().back());
    auto end = vm.add_jmp(OPCODE::JMP_FALSE, 0);
    
    for(auto addr : jmp_locs) {
      vm.conf_jmp(addr, vm.get_current_location());
    }
    alt.statement()->accept(*this, ctx);

    vm.conf_jmp(end, vm.get_current_location());
  }

  vm.add_pop(POP);
} 

void Generator::visit(const ProcedureCall& stmt, const Context& ctx) {
  for(const auto& arg : stmt.args()) {
    arg->accept(*this, ctx);
  }
  size_t proc_loc = -1;
  for(int i = m_func_locations.size()-1;i >= 0;--i) {
    if(const auto it = m_func_locations[i].find(stmt.procedure()->id());it != m_func_locations[i].end()) {
      proc_loc = it->second;
      break;
    }
  }
  vm.add_call(proc_loc);
  
  Int args_size = 0;
  for(const auto& arg : stmt.procedure()->type()->args()) {
    args_size += get_type_size(arg.type());
  }
  vm.add_resize_stack(-args_size);
}

void Generator::visit(const FunctionCall& expr, const Context& ctx) {
  const auto ret_size = get_type_size(expr.function()->type()->return_type());
  vm.add_resize_stack(ret_size);
  for(const auto& arg : expr.args()) {
    arg->accept(*this, ctx);
  }
  size_t proc_loc = -1;
  for(int i = m_func_locations.size()-1;i >= 0;--i) {
    if(const auto it = m_func_locations[i].find(expr.function()->id());it != m_func_locations[i].end()) {
      proc_loc = it->second;
      break;
    }
  }
  vm.add_call(proc_loc);

  Int args_size = 0;
  for (const auto &arg : expr.function()->type()->args()) {
    args_size += get_type_size(arg.type());
  }
  vm.add_resize_stack(-args_size);
}



} // namespace pascal_compiler