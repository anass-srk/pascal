#include "../src/vm.hpp"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>

namespace pascal_vm {

using Real = double;
using Int = int64_t;

class IORedirect {
public:
  std::string str() const { return m_output.str(); }

  IORedirect(std::string s = "")
    : m_input(std::move(s)), m_old_output(std::cout.rdbuf(m_output.rdbuf())),
      m_old_input(std::cin.rdbuf(m_input.rdbuf())) {}

  ~IORedirect() {
    std::cout.rdbuf(m_old_output);
    std::cin.rdbuf(m_old_input);
  }

private:
  std::ostringstream m_output;
  std::istringstream m_input;
  std::streambuf *m_old_input, *m_old_output;
};

// ==================== BASIC STACK OPERATIONS ====================

TEST(VMTest, PushAndPop_Int64) {
  VM vm;
  vm.add_push(OPCODE::PUSH_Q, Int(42));
  vm.add_pop(OPCODE::POP_Q);
  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.data().size(), 0);
}

TEST(VMTest, PushAndPop_Byte) {
  VM vm;
  vm.add_push(OPCODE::PUSH_B, int8_t(127));
  vm.add_pop(OPCODE::POP_B);
  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.data().size(), 0);
}

TEST(VMTest, Push_MultipleValues) {
  VM vm;
  vm.add_push(OPCODE::PUSH_Q, Int(10));
  vm.add_push(OPCODE::PUSH_Q, Int(20));
  vm.add_pop(OPCODE::POP_Q);
  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<Int>(), 10);
}

// ==================== ARITHMETIC INTEGERS ====================

TEST(VMTest, Add_Integers) {
  VM vm;
  vm.add_push(OPCODE::PUSH_Q, Int(30));
  vm.add_push(OPCODE::PUSH_Q, Int(12));
  vm.add_arithmetic_op(OPCODE::ADD_I);
  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<Int>(), 42);
}

TEST(VMTest, Subtract_Integers) {
  VM vm;
  vm.add_push(OPCODE::PUSH_Q, Int(50));
  vm.add_push(OPCODE::PUSH_Q, Int(8));
  vm.add_arithmetic_op(OPCODE::SUB_I);
  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<Int>(), 42);
}

TEST(VMTest, Multiply_Integers) {
  VM vm;
  vm.add_push(OPCODE::PUSH_Q, Int(7));
  vm.add_push(OPCODE::PUSH_Q, Int(6));
  vm.add_arithmetic_op(OPCODE::MUL_I);
  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<Int>(), 42);
}

TEST(VMTest, Divide_Integers) {
  VM vm;
  vm.add_push(OPCODE::PUSH_Q, Int(100));
  vm.add_push(OPCODE::PUSH_Q, Int(4));
  vm.add_arithmetic_op(OPCODE::DIV_I);
  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<Int>(), 25);
}

TEST(VMTest, IntegerDivision_Truncates) {
  VM vm;
  vm.add_push(OPCODE::PUSH_Q, Int(7));
  vm.add_push(OPCODE::PUSH_Q, Int(3));
  vm.add_arithmetic_op(OPCODE::DIV_I);
  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<Int>(), 2);
}

TEST(VMTest, Subtract_NegativeResult) {
  VM vm;
  vm.add_push(OPCODE::PUSH_Q, Int(5));
  vm.add_push(OPCODE::PUSH_Q, Int(10));
  vm.add_arithmetic_op(OPCODE::SUB_I);
  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<Int>(), -5);
}

// ==================== ARITHMETIC REALS ====================

TEST(VMTest, Add_Reals) {
  VM vm;
  vm.add_push(OPCODE::PUSH_Q, Real(3.5));
  vm.add_push(OPCODE::PUSH_Q, Real(2.5));
  vm.add_arithmetic_op(OPCODE::ADD_R);
  vm.add_halt();
  vm.run();

  EXPECT_DOUBLE_EQ(vm.fetch_data<Real>(), 6.0);
}

TEST(VMTest, Subtract_Reals) {
  VM vm;
  vm.add_push(OPCODE::PUSH_Q, Real(10.0));
  vm.add_push(OPCODE::PUSH_Q, Real(3.5));
  vm.add_arithmetic_op(OPCODE::SUB_R);
  vm.add_halt();
  vm.run();

  EXPECT_DOUBLE_EQ(vm.fetch_data<Real>(), 6.5);
}

TEST(VMTest, Multiply_Reals) {
  VM vm;
  vm.add_push(OPCODE::PUSH_Q, Real(4.0));
  vm.add_push(OPCODE::PUSH_Q, Real(2.5));
  vm.add_arithmetic_op(OPCODE::MUL_R);
  vm.add_halt();
  vm.run();

  EXPECT_DOUBLE_EQ(vm.fetch_data<Real>(), 10.0);
}

TEST(VMTest, Divide_Reals) {
  VM vm;
  vm.add_push(OPCODE::PUSH_Q, Real(10.0));
  vm.add_push(OPCODE::PUSH_Q, Real(4.0));
  vm.add_arithmetic_op(OPCODE::DIV_R);
  vm.add_halt();
  vm.run();

  EXPECT_DOUBLE_EQ(vm.fetch_data<Real>(), 2.5);
}

// ==================== ARITHMETIC CHARS ====================

TEST(VMTest, Add_Chars) {
  VM vm;
  vm.add_push(OPCODE::PUSH_B, char(5));
  vm.add_push(OPCODE::PUSH_B, 'A');
  vm.add_arithmetic_op(OPCODE::ADD_C);
  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<char>(), 'F');
}

TEST(VMTest, Subtract_Chars) {
  VM vm;
  vm.add_push(OPCODE::PUSH_B, 'z');
  vm.add_push(OPCODE::PUSH_B, 'a');
  vm.add_arithmetic_op(OPCODE::SUB_C);
  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<char>(), char(25));
}

TEST(VMTest, Multiply_Chars) {
  VM vm;
  vm.add_push(OPCODE::PUSH_B, char(10));
  vm.add_push(OPCODE::PUSH_B, char(3));
  vm.add_arithmetic_op(OPCODE::MUL_C);
  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<char>(), char(30));
}

TEST(VMTest, Divide_Chars) {
  VM vm;
  vm.add_push(OPCODE::PUSH_B, 'A');
  vm.add_push(OPCODE::PUSH_B, char(2));
  vm.add_arithmetic_op(OPCODE::DIV_C);
  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<char>(), char(32));
}

// ==================== LOGICAL OPERATIONS ====================

TEST(VMTest, AND_Operation) {
  VM vm;
  vm.add_push<bool>(OPCODE::PUSH_B, 1);
  vm.add_push<bool>(OPCODE::PUSH_B, 1);
  vm.add_logic_op(OPCODE::AND);

  vm.add_push<bool>(OPCODE::PUSH_B, 1);
  vm.add_push<bool>(OPCODE::PUSH_B, 0);
  vm.add_logic_op(OPCODE::AND);

  vm.add_push<bool>(OPCODE::PUSH_B, 0);
  vm.add_push<bool>(OPCODE::PUSH_B, 1);
  vm.add_logic_op(OPCODE::AND);

  vm.add_push<bool>(OPCODE::PUSH_B, 0);
  vm.add_push<bool>(OPCODE::PUSH_B, 0);
  vm.add_logic_op(OPCODE::AND);

  vm.add_halt();
  vm.run();

  for (bool b : {false, false, false, true}) {
    EXPECT_EQ(vm.fetch_data<bool>(), b);
  }
}

TEST(VMTest, OR_Operation) {
  VM vm;
  vm.add_push<bool>(OPCODE::PUSH_B, 1);
  vm.add_push<bool>(OPCODE::PUSH_B, 1);
  vm.add_logic_op(OPCODE::OR);

  vm.add_push<bool>(OPCODE::PUSH_B, 1);
  vm.add_push<bool>(OPCODE::PUSH_B, 0);
  vm.add_logic_op(OPCODE::OR);

  vm.add_push<bool>(OPCODE::PUSH_B, 0);
  vm.add_push<bool>(OPCODE::PUSH_B, 1);
  vm.add_logic_op(OPCODE::OR);

  vm.add_push<bool>(OPCODE::PUSH_B, 0);
  vm.add_push<bool>(OPCODE::PUSH_B, 0);
  vm.add_logic_op(OPCODE::OR);

  vm.add_halt();
  vm.run();

  for (bool b : {false, true, true, true}) {
    EXPECT_EQ(vm.fetch_data<bool>(), b);
  }
}

TEST(VMTest, NOT_Operation) {
  VM vm;
  vm.add_push<bool>(OPCODE::PUSH_B, 1);
  vm.add_not_op();

  vm.add_push<bool>(OPCODE::PUSH_B, 0);
  vm.add_not_op();

  vm.add_halt();
  vm.run();

  EXPECT_TRUE(vm.fetch_data<bool>());
  EXPECT_FALSE(vm.fetch_data<bool>());
}

// ==================== COMPARISON INTEGERS ====================

TEST(VMTest, Compare_Int) {
  VM vm;
  vm.add_push(OPCODE::PUSH_Q, Int(20));
  vm.add_push(OPCODE::PUSH_Q, Int(10));
  vm.add_cmp(OPCODE::CMP_I);

  vm.add_push(OPCODE::PUSH_Q, Int(42));
  vm.add_push(OPCODE::PUSH_Q, Int(42));
  vm.add_cmp(OPCODE::CMP_I);

  vm.add_push(OPCODE::PUSH_Q, Int(10));
  vm.add_push(OPCODE::PUSH_Q, Int(100));
  vm.add_cmp(OPCODE::CMP_I);

  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<char>(), char(-1));
  EXPECT_EQ(vm.fetch_data<char>(), char(0));
  EXPECT_EQ(vm.fetch_data<char>(), char(1));
}

TEST(VMTest, Compare_Real) {
  VM vm;
  vm.add_push(OPCODE::PUSH_Q, Real(20.8));
  vm.add_push(OPCODE::PUSH_Q, Real(20.6));
  vm.add_cmp(OPCODE::CMP_R);

  vm.add_push(OPCODE::PUSH_Q, Real(42.0));
  vm.add_push(OPCODE::PUSH_Q, Real(42.0));
  vm.add_cmp(OPCODE::CMP_R);

  vm.add_push(OPCODE::PUSH_Q, Real(-10.14));
  vm.add_push(OPCODE::PUSH_Q, Real(100));
  vm.add_cmp(OPCODE::CMP_R);

  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<char>(), char(-1));
  EXPECT_EQ(vm.fetch_data<char>(), char(0));
  EXPECT_EQ(vm.fetch_data<char>(), char(1));
}

TEST(VMTest, Compare_Char) {
  VM vm;
  vm.add_push(OPCODE::PUSH_B, char('Z'));
  vm.add_push(OPCODE::PUSH_B, char('A'));
  vm.add_cmp(OPCODE::CMP_C);

  vm.add_push(OPCODE::PUSH_B, char(-1));
  vm.add_push(OPCODE::PUSH_B, char(1));
  vm.add_cmp(OPCODE::CMP_C);

  vm.add_push(OPCODE::PUSH_B, char('?'));
  vm.add_push(OPCODE::PUSH_B, char('?'));
  vm.add_cmp(OPCODE::CMP_C);

  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<char>(), char(0));
  EXPECT_EQ(vm.fetch_data<char>(), char(-1));
  EXPECT_EQ(vm.fetch_data<char>(), char(1));
}

// ==================== FLAG OPERATIONS ====================

TEST(VMTest, LE_flag) {
  VM vm;
  vm.add_push(OPCODE::PUSH_B, char(-1));
  vm.add_flag(OPCODE::LE);

  vm.add_push(OPCODE::PUSH_B, char(0));
  vm.add_flag(OPCODE::LE);

  vm.add_push(OPCODE::PUSH_B, char(1));
  vm.add_flag(OPCODE::LE);

  vm.add_halt();
  vm.run();

  for (bool b : {false, true, true}) {
    EXPECT_EQ(vm.fetch_data<bool>(), b);
  }
}

TEST(VMTest, LT_flag) {
  VM vm;
  vm.add_push(OPCODE::PUSH_B, char(-1));
  vm.add_flag(OPCODE::LT);

  vm.add_push(OPCODE::PUSH_B, char(0));
  vm.add_flag(OPCODE::LT);

  vm.add_push(OPCODE::PUSH_B, char(1));
  vm.add_flag(OPCODE::LT);

  vm.add_halt();
  vm.run();

  for (bool b : {false, false, true}) {
    EXPECT_EQ(vm.fetch_data<bool>(), b);
  }
}

TEST(VMTest, EQ_flag) {
  VM vm;
  vm.add_push(OPCODE::PUSH_B, char(-1));
  vm.add_flag(OPCODE::EQ);

  vm.add_push(OPCODE::PUSH_B, char(0));
  vm.add_flag(OPCODE::EQ);

  vm.add_push(OPCODE::PUSH_B, char(1));
  vm.add_flag(OPCODE::EQ);

  vm.add_halt();
  vm.run();

  for (bool b : {false, true, false}) {
    EXPECT_EQ(vm.fetch_data<bool>(), b);
  }
}

TEST(VMTest, NE_flag) {
  VM vm;
  vm.add_push(OPCODE::PUSH_B, char(-1));
  vm.add_flag(OPCODE::NE);

  vm.add_push(OPCODE::PUSH_B, char(0));
  vm.add_flag(OPCODE::NE);

  vm.add_push(OPCODE::PUSH_B, char(1));
  vm.add_flag(OPCODE::NE);

  vm.add_halt();
  vm.run();

  for (bool b : {true, false, true}) {
    EXPECT_EQ(vm.fetch_data<bool>(), b);
  }
}

TEST(VMTest, GT_flag) {
  VM vm;
  vm.add_push(OPCODE::PUSH_B, char(-1));
  vm.add_flag(OPCODE::GT);

  vm.add_push(OPCODE::PUSH_B, char(0));
  vm.add_flag(OPCODE::GT);

  vm.add_push(OPCODE::PUSH_B, char(1));
  vm.add_flag(OPCODE::GT);

  vm.add_halt();
  vm.run();

  for (bool b : {true, false, false}) {
    EXPECT_EQ(vm.fetch_data<bool>(), b);
  }
}

TEST(VMTest, GE_flag) {
  VM vm;
  vm.add_push(OPCODE::PUSH_B, char(-1));
  vm.add_flag(OPCODE::GE);

  vm.add_push(OPCODE::PUSH_B, char(0));
  vm.add_flag(OPCODE::GE);

  vm.add_push(OPCODE::PUSH_B, char(1));
  vm.add_flag(OPCODE::GE);

  vm.add_halt();
  vm.run();

  for (bool b : {true, true, false}) {
    EXPECT_EQ(vm.fetch_data<bool>(), b);
  }
}

// ==================== JUMP INSTRUCTIONS ====================

TEST(VMTest, JMP_Absolute) {
  VM vm;

  vm.add_push(OPCODE::PUSH_Q, Int(0));
  size_t jump_loc = vm.add_jmp(OPCODE::JMP, 0);
  vm.add_push(OPCODE::PUSH_Q, Int(1));

  ASSERT_TRUE(vm.conf_jmp(jump_loc, vm.get_current_location()));
  vm.add_halt();

  vm.run();

  EXPECT_EQ(vm.fetch_data<Int>(), 0);
}

TEST(VMTest, JMP_TRUE_WhenTrue) {
  VM vm;

  vm.add_push(OPCODE::PUSH_Q, Int(0));
  vm.add_push(OPCODE::PUSH_B, true);

  size_t jump_loc = vm.add_jmp(OPCODE::JMP_TRUE, 0);

  vm.add_push(OPCODE::PUSH_Q, Int(1));

  ASSERT_TRUE(vm.conf_jmp(jump_loc, vm.get_current_location()));
  vm.add_halt();

  vm.run();

  EXPECT_EQ(vm.fetch_data<Int>(), 0);
}

TEST(VMTest, JMP_TRUE_WhenFalse) {
  VM vm;

  vm.add_push(OPCODE::PUSH_Q, Int(0));
  vm.add_push(OPCODE::PUSH_B, false);

  size_t jump_loc = vm.add_jmp(OPCODE::JMP_TRUE, 0);

  vm.add_push(OPCODE::PUSH_Q, Int(1));

  ASSERT_TRUE(vm.conf_jmp(jump_loc, vm.get_current_location()));
  vm.add_halt();

  vm.run();

  EXPECT_EQ(vm.fetch_data<Int>(), 1);
}

TEST(VMTest, JMP_FALSE_WhenTrue) {
  VM vm;

  vm.add_push(OPCODE::PUSH_Q, Int(0));
  vm.add_push(OPCODE::PUSH_B, true);

  size_t jump_loc = vm.add_jmp(OPCODE::JMP_FALSE, 0);

  vm.add_push(OPCODE::PUSH_Q, Int(1));

  ASSERT_TRUE(vm.conf_jmp(jump_loc, vm.get_current_location()));
  vm.add_halt();

  vm.run();

  EXPECT_EQ(vm.fetch_data<Int>(), 1);
}

TEST(VMTest, JMP_FALSE_WhenFalse) {
  VM vm;

  vm.add_push(OPCODE::PUSH_Q, Int(0));
  vm.add_push(OPCODE::PUSH_B, false);

  size_t jump_loc = vm.add_jmp(OPCODE::JMP_FALSE, 0);

  vm.add_push(OPCODE::PUSH_Q, Int(1));

  ASSERT_TRUE(vm.conf_jmp(jump_loc, vm.get_current_location()));
  vm.add_halt();

  vm.run();

  EXPECT_EQ(vm.fetch_data<Int>(), 0);
}

// ==================== STACK OPERATIONS ====================

TEST(VMTest, MODSTK_IncreaseStackSize) {
  VM vm;
  vm.add_resize_stack(16);
  vm.add_halt();
  vm.run();

  EXPECT_GE(vm.data().size(), 16);
}

TEST(VMTest, MODSTK_DecreaseStackSize) {
  VM vm;
  vm.add_push(OPCODE::PUSH_Q, Int(1));
  vm.add_push(OPCODE::PUSH_Q, Int(2));
  vm.add_resize_stack(-8);
  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.data().size(), 8);
}

// ==================== MEMORY OPERATIONS ====================

TEST(VMTest, STORE_Q_and_LOAD_Q) {
  VM vm;
  vm.add_resize_stack(16);

  vm.add_push(OPCODE::PUSH_Q, Int(0));
  vm.add_push(OPCODE::PUSH_Q, Int(42));
  vm.add_move(OPCODE::STORE_Q);

  vm.add_push(OPCODE::PUSH_Q, Int(8));
  vm.add_push(OPCODE::PUSH_Q, Int(-7));
  vm.add_move(OPCODE::STORE_Q);

  vm.add_push(OPCODE::PUSH_Q, Int(0));
  vm.add_load(OPCODE::LOAD_Q);

  vm.add_push(OPCODE::PUSH_Q, Int(8));
  vm.add_load(OPCODE::LOAD_Q);

  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<Int>(), -7);
  EXPECT_EQ(vm.fetch_data<Int>(), 42);
}

TEST(VMTest, STORE_B_and_LOAD_B) {
  VM vm;
  vm.add_resize_stack(8);

  vm.add_push(OPCODE::PUSH_Q, Int(4));
  vm.add_push(OPCODE::PUSH_B, int8_t(99));
  vm.add_move(OPCODE::STORE_B);

  vm.add_push(OPCODE::PUSH_Q, Int(5));
  vm.add_push(OPCODE::PUSH_B, int8_t(-11));
  vm.add_move(OPCODE::STORE_B);

  vm.add_push(OPCODE::PUSH_Q, Int(4));
  vm.add_load(OPCODE::LOAD_B);

  vm.add_push(OPCODE::PUSH_Q, Int(5));
  vm.add_load(OPCODE::LOAD_B);

  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<int8_t>(), -11);
  EXPECT_EQ(vm.fetch_data<int8_t>(), 99);
}

// ==================== COMPLEX INTEGRATION TESTS ====================

TEST(VMTest, MultipleArithmeticOps) {
  VM vm;

  vm.add_push(OPCODE::PUSH_Q, Int(3));
  vm.add_push(OPCODE::PUSH_Q, Int(2));
  vm.add_arithmetic_op(OPCODE::MUL_I);

  vm.add_push(OPCODE::PUSH_Q, Int(4));
  vm.add_arithmetic_op(OPCODE::ADD_I);

  vm.add_push(OPCODE::PUSH_Q, Int(5));
  vm.add_arithmetic_op(OPCODE::SUB_I);

  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<Int>(), 5);
}

TEST(VMTest, CompareAndBranchSequence) {
  IORedirect output;
  VM vm;

  vm.add_resize_stack(8);

  vm.add_push(OPCODE::PUSH_Q, Int(0));
  vm.add_push(OPCODE::PUSH_Q, Int(5));
  vm.add_move(OPCODE::STORE_Q);

  size_t loop_beg = vm.get_current_location();

  vm.add_push(OPCODE::PUSH_Q, Int(0));
  vm.add_load(OPCODE::LOAD_Q);
  vm.add_push(OPCODE::PUSH_Q, Int(7));
  vm.add_cmp(OPCODE::CMP_I);
  vm.add_flag(OPCODE::GT);

  size_t jmp_addr = vm.add_jmp(OPCODE::JMP_TRUE, 0);

  vm.add_push(OPCODE::PUSH_Q, Int(0));
  vm.add_load(OPCODE::LOAD_Q);

  vm.add_write(OPCODE::WRITE_I);

  vm.add_push(OPCODE::PUSH_Q, Int(0));

  vm.add_push(OPCODE::PUSH_Q, Int(0));
  vm.add_load(OPCODE::LOAD_Q);
  vm.add_push(OPCODE::PUSH_Q, Int(1));
  vm.add_arithmetic_op(OPCODE::ADD_I);

  vm.add_move(OPCODE::STORE_Q);

  vm.add_jmp(OPCODE::JMP, loop_beg);

  ASSERT_TRUE(vm.conf_jmp(jmp_addr, vm.get_current_location()));

  vm.add_halt();

  vm.run();

  EXPECT_EQ(output.str(), "567");
}

TEST(VMTest, IO) {

  IORedirect redirect("Alice\n42\n0\n3.14\n");

  VM vm;

  const auto jmp_addr = vm.add_jmp(OPCODE::JMP, 0);
  const std::string str = "What's you name, age, digit, PI ?";
  const auto str_addr = vm.add_string(str);
  ASSERT_TRUE(vm.conf_jmp(jmp_addr, vm.get_current_location()));

  vm.add_resize_stack(27); // Int + Real + char[10] + char

  vm.add_push(OPCODE::PUSH_Q, Int(str_addr));
  vm.add_write(OPCODE::WRITE_CONST_S);

  vm.add_push(OPCODE::PUSH_Q, Int(16)); // char[10] address
  vm.add_push(OPCODE::PUSH_Q, Int(10)); // length
  vm.add_read_string();

  vm.add_push(OPCODE::PUSH_Q, Int(0)); // Int
  vm.add_read(OPCODE::READ_I);

  vm.add_push(OPCODE::PUSH_Q, Int(26)); // Char
  vm.add_read(OPCODE::READ_C);

  vm.add_push(OPCODE::PUSH_Q, Int(8)); // Real
  vm.add_read(OPCODE::READ_R);

  vm.add_push(OPCODE::PUSH_Q, Int(0)); // Int
  vm.add_load(OPCODE::LOAD_Q);
  vm.add_write(OPCODE::WRITE_I);

  vm.add_push(OPCODE::PUSH_Q, Int(8)); // Real
  vm.add_load(OPCODE::LOAD_Q);
  vm.add_write(OPCODE::WRITE_R);

  vm.add_push(OPCODE::PUSH_Q, Int(16)); // char[10] address
  vm.add_write(OPCODE::WRITE_S);

  vm.add_push(OPCODE::PUSH_Q, Int(26)); // Char
  vm.add_load(OPCODE::LOAD_B);
  vm.add_write(OPCODE::WRITE_C);

  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<char>(), '0');
  EXPECT_TRUE((vm.fetch_data<std::array<char, 10>>() ==
              std::array<char, 10>{'A', 'l', 'i', 'c', 'e'}));
  EXPECT_EQ(vm.fetch_data<Real>(), 3.14);
  EXPECT_EQ(vm.fetch_data<Int>(), 42);

  EXPECT_EQ(redirect.str(), str + "423.14Alice0");
}

TEST(VMTest, Duplicate) {
  VM vm;

  vm.add_push(OPCODE::PUSH_Q, Int(999999));
  vm.add_duplicate(OPCODE::DUPL_Q);
  vm.add_push(OPCODE::PUSH_B, char(-1));
  vm.add_duplicate(OPCODE::DUPL_B);

  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.fetch_data<char>(), -1);
  EXPECT_EQ(vm.fetch_data<char>(), -1);
  EXPECT_EQ(vm.fetch_data<Int>(), 999999);
  EXPECT_EQ(vm.fetch_data<Int>(), 999999);
  EXPECT_TRUE(vm.data().empty());
}

TEST(VMTest, Function) {
  VM vm;

  const auto beg_addr = vm.add_jmp(OPCODE::JMP, 0);

  const auto func_beg = vm.get_current_location();

  //  two params f (int i,int j) -> int (i*j)
  //  return value | args | return address | old frame pointer | consts + vars
  //  fp -> stack size after all is pushed (excluding consts and vars)

  //  return value address
  //  addr = -8 (size of the var to point at the start) - 16 (ret_addr + old fp) - 16 (args) = -40
  vm.add_push_frame_pointer();      
  vm.add_push(OPCODE::PUSH_Q, Int(-40)); 
  vm.add_arithmetic_op(OPCODE::ADD_I);

  //  arg 1
  vm.add_push_frame_pointer();
  vm.add_push(OPCODE::PUSH_Q, Int(-32));
  vm.add_arithmetic_op(OPCODE::ADD_I);
  vm.add_load(OPCODE::LOAD_Q);

  //  arg 2
  vm.add_push_frame_pointer();
  vm.add_push(OPCODE::PUSH_Q, Int(-24));
  vm.add_arithmetic_op(OPCODE::ADD_I);
  vm.add_load(OPCODE::LOAD_Q);

  vm.add_arithmetic_op(OPCODE::MUL_I);

  //  move result to return address
  vm.add_move(OPCODE::STORE_Q); 
  //  0 = sizeof(local vars)
  vm.add_return(0);

  vm.conf_jmp(beg_addr, vm.get_current_location());

  //  allocate space for return address
  vm.add_resize_stack(8);

  vm.add_push(OPCODE::PUSH_Q, Int(3));
  vm.add_push(OPCODE::PUSH_Q, Int(-4));
  
  vm.add_call(func_beg);

  //  deallocate args (2 Int)
  vm.add_resize_stack(-16);

  vm.add_halt();
  vm.run();

  EXPECT_EQ(vm.data().size(), 8);
  EXPECT_EQ(vm.fetch_data<Int>(), -12);
}

TEST(VMTest, Procedure) {
  // 3 inputs
  IORedirect redirect("22\n-4\n0\n");
  VM vm;

  const auto beg_addr = vm.add_jmp(OPCODE::JMP, 0);

  const std::string ask_str = "Enter an integer :\n";
  const auto ask = vm.add_string(ask_str);

  // f(Int i) -> void
  const auto proc_beg = vm.get_current_location();
  
  const auto proc_jmp = vm.add_jmp(OPCODE::JMP, 0);

  const std::string pos_str = "Positive !\n";
  const auto pos = vm.add_string(pos_str);
  const std::string neg_str = "Negative !\n";
  const auto neg = vm.add_string(neg_str);
  const std::string nul_str = "Null !\n";
  const auto nul = vm.add_string(nul_str);
  
  vm.conf_jmp(proc_jmp, vm.get_current_location());


  //  Load arg (Int)
  vm.add_push_frame_pointer();
  vm.add_push(OPCODE::PUSH_Q, Int(-24));
  vm.add_arithmetic_op(OPCODE::ADD_I);
  vm.add_load(OPCODE::LOAD_Q);

  vm.add_push(OPCODE::PUSH_Q, Int(0));

  //  Compare arg with 0
  vm.add_cmp(OPCODE::CMP_I);
  vm.add_flag(OPCODE::GT);

  const auto less_equals = vm.add_jmp(OPCODE::JMP_FALSE, 0);
  
  //  arg > 0
  vm.add_push(OPCODE::PUSH_Q, pos);
  vm.add_write(OPCODE::WRITE_CONST_S);

  const auto greater_end = vm.add_jmp(OPCODE::JMP, 0);

  vm.conf_jmp(less_equals, vm.get_current_location());

  //  Load arg (Int)
  vm.add_push_frame_pointer();
  vm.add_push(OPCODE::PUSH_Q, Int(-24));
  vm.add_arithmetic_op(OPCODE::ADD_I);
  vm.add_load(OPCODE::LOAD_Q);

  vm.add_push(OPCODE::PUSH_Q, Int(0));

  //  Compare arg with 0
  vm.add_cmp(OPCODE::CMP_I);
  vm.add_flag(OPCODE::EQ);

  const auto less = vm.add_jmp(OPCODE::JMP_FALSE, 0);

  //  arg == 0
  vm.add_push(OPCODE::PUSH_Q, nul);
  vm.add_write(OPCODE::WRITE_CONST_S);

  const auto equals_end = vm.add_jmp(OPCODE::JMP, 0);

  vm.conf_jmp(less, vm.get_current_location());

  //  arg < 0 (else)
  vm.add_push(OPCODE::PUSH_Q, neg);
  vm.add_write(OPCODE::WRITE_CONST_S);

  vm.conf_jmp(greater_end, vm.get_current_location());
  vm.conf_jmp(equals_end, vm.get_current_location());

  vm.add_return(0);

  vm.conf_jmp(beg_addr, vm.get_current_location());

  vm.add_resize_stack(8); // one variable : int i at address 0

  // Ask for input
  vm.add_push(OPCODE::PUSH_Q, ask);
  vm.add_write(OPCODE::WRITE_CONST_S);

  // Read input
  vm.add_push(OPCODE::PUSH_Q, Int(0));
  vm.add_read(OPCODE::READ_I);

  // push local variable as arg for procedure
  vm.add_push(OPCODE::PUSH_Q, Int(0));
  vm.add_load(OPCODE::LOAD_Q);

  vm.add_call(proc_beg);
  vm.add_pop(OPCODE::POP_Q);

  // Ask for input
  vm.add_push(OPCODE::PUSH_Q, ask);
  vm.add_write(OPCODE::WRITE_CONST_S);

  // Read input
  vm.add_push(OPCODE::PUSH_Q, Int(0));
  vm.add_read(OPCODE::READ_I);

  // push local variable as arg for procedure
  vm.add_push(OPCODE::PUSH_Q, Int(0));
  vm.add_load(OPCODE::LOAD_Q);

  vm.add_call(proc_beg);
  vm.add_pop(OPCODE::POP_Q);

  // Ask for input
  vm.add_push(OPCODE::PUSH_Q, ask);
  vm.add_write(OPCODE::WRITE_CONST_S);

  // Read input
  vm.add_push(OPCODE::PUSH_Q, Int(0));
  vm.add_read(OPCODE::READ_I);

  // push local variable as arg for procedure
  vm.add_push(OPCODE::PUSH_Q, Int(0));
  vm.add_load(OPCODE::LOAD_Q);

  vm.add_call(proc_beg);
  vm.add_pop(OPCODE::POP_Q);

  vm.add_halt();
  vm.run();

  EXPECT_EQ(redirect.str(), ask_str + pos_str + ask_str + neg_str + ask_str + nul_str);
  EXPECT_EQ(vm.data().size(), 8);
  EXPECT_EQ(vm.fetch_data<Int>(), 0); // last value of the local var from the input
}

} // namespace pascal_vm
