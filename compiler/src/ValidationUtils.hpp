#pragma once
#include "Semantics.hpp"
#include "Ast.hpp"

namespace pascal_compiler::validation {

inline bool is_basic_type(const Type* type) {
  if (!type) return false;
  std::string_view name = type->get_underlying_type()->id();
  return name == CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] ||
         name == CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)] ||
         name == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)] ||
         name == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)] ||
         name == CONST_CAT_NAMES[int(CONST_CAT::CC_STRING)];
}

inline bool is_boolean_type(const Type* type) {
  if (!type) return false;
  return type->get_underlying_type()->id() == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)];
}

inline bool is_enum_type(const Type* type) {
  if (!type) return false;
  return type->get_underlying_type()->category() == TYPE_CAT::TC_ENUM;
}

inline bool is_array_of_char(const Type* type) {
  if (!type || type->category() != TYPE_CAT::TC_ARRAY) return false;
  const auto* arr = static_cast<const Array*>(type);
  return arr->element_type()->get_underlying_type()->id() == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)];
}

inline bool is_io_compatible(const Type *type) {
  if (!type) return false;
  std::string_view name = type->get_underlying_type()->id();
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

inline bool is_valid_unary_operand(UnaryOp op, const Type* type) {
  if (!type) return false;
  std::string_view name = type->get_underlying_type()->id();
  if (op == UnaryOp::Not) {
    return name == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)];
  } else { // Plus or Minus
    return name == CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)];
  }
}

inline bool is_relational_op(RelOp op) {
  return op == RelOp::Lt || op == RelOp::Le ||
         op == RelOp::Gt || op == RelOp::Ge;
}

inline bool is_equality_op(RelOp op) {
  return op == RelOp::Eq || op == RelOp::Ne;
}

inline bool is_arithmetic_op(ALOp op) {
  return op == ALOp::Add || op == ALOp::Sub ||
         op == ALOp::Mul || op == ALOp::Div;
}

inline bool is_logical_op(ALOp op) {
  return op == ALOp::And || op == ALOp::Or;
}

inline bool is_valid_binary_operand(RelOp op, const Type* left, const Type* right) {
  if (!left || !right) return false;
  const auto* left_under = left->get_underlying_type();
  const auto* right_under = right->get_underlying_type();
  if (left_under != right_under) return false;
  
  std::string_view name = left_under->id();
  if (is_relational_op(op)) {
    return name == CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)];
  } else if(is_equality_op(op)) {
    return name == CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_ENUM)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_STRING)];
  }
  return false;
}

inline bool is_valid_binary_operand(ALOp op, const Type* left, const Type* right) {
  if (!left || !right) return false;
  const auto* left_under = left->get_underlying_type();
  const auto* right_under = right->get_underlying_type();
  if (left_under != right_under) return false;

  std::string_view name = left_under->id();

  if (is_arithmetic_op(op)) {
    return name == CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)] ||
           name == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)] ||
           (name == CONST_CAT_NAMES[int(CONST_CAT::CC_STRING)] && op == ALOp::Add);
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
  std::string_view name = underlying->id();
  return name == CONST_CAT_NAMES[int(CONST_CAT::CC_INT)] ||
         name == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)] ||
         name == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)] ||
         underlying->category() == TYPE_CAT::TC_ENUM;
}

}