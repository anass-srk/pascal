#pragma once
#include "Semantics.hpp"
#include "Ast.hpp"

namespace pascal_compiler::validation {

inline bool is_basic_type(const Type* type) {
  if (!type) return false;
  std::string_view name = type->get_underlying_type()->m_name;
  return name == CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] ||
         name == CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)] ||
         name == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)] ||
         name == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)];
}

inline bool is_boolean_type(const Type* type) {
  if (!type) return false;
  return type->get_underlying_type()->m_name == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)];
}

inline bool is_enum_type(const Type* type) {
  if (!type) return false;
  return type->get_underlying_type()->m_type == TYPE_CAT::TC_ENUM;
}

inline bool is_array_of_char(const Type* type) {
  if (!type || type->m_type != TYPE_CAT::TC_ARRAY) return false;
  const auto* arr = static_cast<const Array*>(type);
  return arr->m_etype->get_underlying_type()->m_name == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)];
}

inline bool is_writable_type(const Type* type) {
  if (!type) return false;
  std::string_view name = type->get_underlying_type()->m_name;
  if (name == CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] ||
      name == CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)] ||
      name == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)] ||
      name == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)] ||
      name == CONST_CAT_NAMES[int(CONST_CAT::CC_STRING)] ||
      is_array_of_char(type)) {
    return true;
  }
  return false;
}

inline bool is_readable_type(const Type* type) {
  if (!type) return false;
  std::string_view name = type->get_underlying_type()->m_name;
  if (name == CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] ||
      name == CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)] ||
      name == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)] ||
      name == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)] ||
      is_array_of_char(type)) {
    return true;
  }
  return false;
}

inline bool is_valid_unary_operand(UnaryOp op, const Type* type) {
  if (!type) return false;
  std::string_view name = type->get_underlying_type()->m_name;
  if (op == UnaryOp::Not) {
    return name == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)];
  } else { // Plus or Minus
    return name == CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)];
  }
}

inline bool is_relational_op(BinaryOp op) {
  return op == BinaryOp::Eq || op == BinaryOp::Ne ||
         op == BinaryOp::Lt || op == BinaryOp::Le ||
         op == BinaryOp::Gt || op == BinaryOp::Ge;
}

inline bool is_arithmetic_op(BinaryOp op) {
  return op == BinaryOp::Add || op == BinaryOp::Sub ||
         op == BinaryOp::Mul || op == BinaryOp::Div;
}

inline bool is_logical_op(BinaryOp op) {
  return op == BinaryOp::And || op == BinaryOp::Or;
}

inline bool is_valid_binary_operand(BinaryOp op, const Type* left, const Type* right) {
  if (!left || !right) return false;
  const auto* left_under = left->get_underlying_type();
  const auto* right_under = right->get_underlying_type();
  if (left_under != right_under) return false;
  
  std::string_view name = left_under->m_name;
  if (is_relational_op(op)) {
    return name == CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)];
  } else if (is_arithmetic_op(op)) {
    return name == CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)];
  } else if (is_logical_op(op)) {
    return name == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)];
  }
  return false;
}

inline bool types_compatible(const Type* t1, const Type* t2) {
  if (!t1 || !t2) return false;
  return t1->get_underlying_type() == t2->get_underlying_type();
}

inline bool is_case_selector_type(const Type* type) {
  if (!type) return false;
  const auto* underlying = type->get_underlying_type();
  std::string_view name = underlying->m_name;
  return name == CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] ||
         name == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)] ||
         name == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)] ||
         underlying->m_type == TYPE_CAT::TC_ENUM;
}

}