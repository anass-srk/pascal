#include <gtest/gtest.h>
#include "../src/vm.hpp"
#include <cstring>
#include <sstream>
#include <string>

using namespace pascal_vm;

// Test Load, Store, Push, Pop, Mov (Memory & Stack Operations)
TEST(VMTest, MovOps)
{
  // Test 1: Load Immediate and Push
  {
    VM vm;

    const int64_t i = 2;
    vm.add_load_inter(OPCODE::LOADIQ, 0, i);

    const int8_t c = 'a';
    vm.add_load_inter(OPCODE::LOADIB, 2, c);

    const double d = 10.56;
    vm.add_load_inter(OPCODE::LOADIQ, 1, d);

    const bool b = true;
    vm.add_load_inter(OPCODE::LOADIB, 5, b);

    vm.add_push(OPCODE::PUSHB, 5);
    vm.add_push(OPCODE::PUSHQ, 1);
    vm.add_push(OPCODE::PUSHB, 2);
    vm.add_push(OPCODE::PUSHQ, 0);

    vm.add_halt();
    vm.run();

    ASSERT_EQ(vm.get_register(0).i, 2);
    ASSERT_EQ(vm.get_register(2).c, 'a');
    ASSERT_DOUBLE_EQ(vm.get_register(1).d, 10.56);
    ASSERT_EQ(vm.get_register(5).b, true);

    auto &stack = vm.data();
    ASSERT_EQ(stack.size(), 18);
    ASSERT_EQ(stack[0], b);

    double d_res;
    std::memcpy(&d_res, &stack[1], sizeof(double));
    ASSERT_DOUBLE_EQ(d_res, d);

    ASSERT_EQ(static_cast<char>(stack[9]), c);

    int64_t i_res;
    std::memcpy(&i_res, &stack[10], sizeof(int64_t));
    ASSERT_EQ(i_res, i);
  }

  // Test 2: Pop Operations
  {
    VM vm;

    const int64_t i = 2;
    vm.add_load_inter(OPCODE::LOADIQ, 0, i);
    vm.add_push(OPCODE::PUSHQ, 0);

    const int8_t c = 'a';
    vm.add_load_inter(OPCODE::LOADIB, 0, c);
    vm.add_push(OPCODE::PUSHB, 0);

    const double d = 10.56;
    vm.add_load_inter(OPCODE::LOADIQ, 0, d);
    vm.add_push(OPCODE::PUSHQ, 0);

    const bool b = true;
    vm.add_load_inter(OPCODE::LOADIB, 0, b);
    vm.add_push(OPCODE::PUSHB, 0);

    vm.add_pop(OPCODE::POPB, 6);
    vm.add_pop(OPCODE::POPQ, 7);
    vm.add_pop(OPCODE::POPB, 8);
    vm.add_pop(OPCODE::POPQ, 9);

    vm.add_halt();
    vm.run();

    ASSERT_EQ(vm.get_register(9).i, i);
    ASSERT_EQ(vm.get_register(8).c, c);
    ASSERT_DOUBLE_EQ(vm.get_register(7).d, d);
    ASSERT_EQ(vm.get_register(6).b, b);
  }

  // Test 3: Store and Load (Absolute Addressing)
  {
    VM vm;

    // Allocate stack space
    vm.add_push(OPCODE::PUSHB, 0);
    vm.add_push(OPCODE::PUSHB, 0);
    vm.add_push(OPCODE::PUSHQ, 0);
    vm.add_push(OPCODE::PUSHQ, 0);

    const int64_t i = 2;
    vm.add_load_inter(OPCODE::LOADIQ, 0, i);
    vm.add_store(OPCODE::STOREQ, 0, 0);

    const int8_t c = 'a';
    vm.add_load_inter(OPCODE::LOADIB, 1, c);
    vm.add_store(OPCODE::STOREB, 1, 8);

    const double d = 10.56;
    vm.add_load_inter(OPCODE::LOADIQ, 1, d);
    vm.add_store(OPCODE::STOREQ, 1, 9);

    const bool b = true;
    vm.add_load_inter(OPCODE::LOADIB, 0, b);
    vm.add_store(OPCODE::STOREB, 0, 17);

    vm.add_load(OPCODE::LOADQ, 3, 0);
    vm.add_load(OPCODE::LOADB, 4, 8);
    vm.add_load(OPCODE::LOADQ, 5, 9);
    vm.add_load(OPCODE::LOADB, 6, 17);

    vm.add_mov(10, 0);
    vm.add_mov(11, 1);

    vm.add_halt();
    vm.run();

    ASSERT_EQ(vm.get_register(3).i, i);
    ASSERT_EQ(vm.get_register(4).c, c);
    ASSERT_DOUBLE_EQ(vm.get_register(5).d, d);
    ASSERT_EQ(vm.get_register(6).b, b);
    ASSERT_EQ(vm.get_register(10).b, b);
    ASSERT_DOUBLE_EQ(vm.get_register(11).d, d);
  }
}

// Test all Arithmetic Operations (Int, Char, Double, Immediate variants)
TEST(VMTest, AllArithmeticOps)
{
  // --- Integer Arithmetic ---
  {
    VM vm;
    vm.add_load_inter(OPCODE::LOADIQ, 0, (int64_t)10);
    vm.add_load_inter(OPCODE::LOADIQ, 1, (int64_t)3);

    vm.add_op(OPCODE::ADD_I, 2, 0, 1); // 10 + 3 = 13
    vm.add_op(OPCODE::SUB_I, 3, 0, 1); // 10 - 3 = 7
    vm.add_op(OPCODE::MUL_I, 4, 0, 1); // 10 * 3 = 30
    vm.add_op(OPCODE::DIV_I, 5, 0, 1); // 10 / 3 = 3
    vm.add_op(OPCODE::MOD_I, 6, 0, 1); // 10 % 3 = 1

    vm.add_op_inter(OPCODE::ADDI_I, 7, 0, (int64_t)5);  // 10 + 5 = 15
    vm.add_op_inter(OPCODE::SUBI_I, 8, 0, (int64_t)5);  // 10 - 5 = 5
    vm.add_op_inter(OPCODE::MULI_I, 9, 0, (int64_t)2);  // 10 * 2 = 20
    vm.add_op_inter(OPCODE::DIVI_I, 10, 0, (int64_t)2); // 10 / 2 = 5
    vm.add_op_inter(OPCODE::MODI_I, 11, 0, (int64_t)3); // 10 % 3 = 1

    vm.add_halt();
    vm.run();

    ASSERT_EQ(vm.get_register(2).i, 13);
    ASSERT_EQ(vm.get_register(3).i, 7);
    ASSERT_EQ(vm.get_register(4).i, 30);
    ASSERT_EQ(vm.get_register(5).i, 3);
    ASSERT_EQ(vm.get_register(6).i, 1);
    ASSERT_EQ(vm.get_register(7).i, 15);
    ASSERT_EQ(vm.get_register(8).i, 5);
    ASSERT_EQ(vm.get_register(9).i, 20);
    ASSERT_EQ(vm.get_register(10).i, 5);
    ASSERT_EQ(vm.get_register(11).i, 1);
  }

}

// Test Control Flow (Jumps)
TEST(VMTest, ControlFlowJumps)
{

  auto test = [](OPCODE op)
  {
    VM vm;

    vm.add_load_inter(OPCODE::LOADIQ, 0, (int64_t)5);
    vm.add_load_inter(OPCODE::LOADIQ, 1, (int64_t)10);

    vm.add_cmp(OPCODE::CMP_I, 0, 1);
    size_t jmp_loc = vm.add_jmp(op);
    vm.add_load_inter(OPCODE::LOADIQ, 2, (int64_t)999);
    vm.conf_jmp(jmp_loc, vm.get_current_location());

    vm.add_cmp(OPCODE::CMP_I, 0, 0);
    size_t jmp_loc2 = vm.add_jmp(op);
    vm.add_load_inter(OPCODE::LOADIQ, 3, (int64_t)999);
    vm.conf_jmp(jmp_loc2, vm.get_current_location());

    vm.add_cmp(OPCODE::CMP_I, 1, 0);
    size_t jmp_loc3 = vm.add_jmp(op);
    vm.add_load_inter(OPCODE::LOADIQ, 4, (int64_t)999);
    vm.conf_jmp(jmp_loc3, vm.get_current_location());

    vm.add_halt();

    vm.run();
    return vm;
  };

  // Test JLT (Jump if: N=true, Z=false)
  {
    VM vm = test(OPCODE::JLT);
    ASSERT_NE(vm.get_register(2).i, 999); // 5 < 10 
    ASSERT_EQ(vm.get_register(3).i, 999); // 5 == 5
    ASSERT_EQ(vm.get_register(4).i, 999); // 5 > 10
  }

  // Test JGT (Jump if: N=false, Z=false)
  {
    VM vm = test(OPCODE::JGT);
    ASSERT_EQ(vm.get_register(2).i, 999); // 5 < 10
    ASSERT_EQ(vm.get_register(3).i, 999); // 5 == 5
    ASSERT_NE(vm.get_register(4).i, 999); // 5 > 10
  }

  // Test JEQ (Jump if: Z=true)
  {
    VM vm = test(OPCODE::JEQ);
    ASSERT_EQ(vm.get_register(2).i, 999); // 5 < 10
    ASSERT_NE(vm.get_register(3).i, 999); // 5 == 5
    ASSERT_EQ(vm.get_register(4).i, 999); // 5 > 10
  }

  // Test JNE (Jump if: Z=false)
  {
    VM vm = test(OPCODE::JNE);
    ASSERT_NE(vm.get_register(2).i, 999); // 5 < 10
    ASSERT_EQ(vm.get_register(3).i, 999); // 5 == 5
    ASSERT_NE(vm.get_register(4).i, 999); // 5 > 10
  }

  // Test JLE (Jump if: N=true OR Z=true)
  {
    VM vm = test(OPCODE::JLE);
    ASSERT_NE(vm.get_register(2).i, 999); // 5 < 10
    ASSERT_NE(vm.get_register(3).i, 999); // 5 == 5
    ASSERT_EQ(vm.get_register(4).i, 999); // 5 > 10
  }

  // Test JGE (Jump if: N=false OR Z=true)
  {
    VM vm = test(OPCODE::JGE);
    ASSERT_EQ(vm.get_register(2).i, 999); // 5 < 10
    ASSERT_NE(vm.get_register(3).i, 999); // 5 == 5
    ASSERT_NE(vm.get_register(4).i, 999); // 5 > 10
  }
}

// Test IO Operations
TEST(VMTest, IO_Ops)
{
  // Redirect cin/cout
  std::stringstream input;
  std::stringstream output;
  auto old_cin = std::cin.rdbuf(input.rdbuf());
  auto old_cout = std::cout.rdbuf(output.rdbuf());

  input << "42 3.14 z hello";

  VM vm;

  // Allocate string buffer on stack (enough for "hello")
  for (int i = 0; i < 8; ++i)
    vm.add_push(OPCODE::PUSHB, 0);

  vm.add_read(OPCODE::READ_I, 0);
  vm.add_read(OPCODE::READ_D, 1);
  vm.add_read(OPCODE::READ_C, 2);

  // READ_S (reads into address 0, max len 8)
  vm.add_load_inter(OPCODE::LOADIQ, 3, (int64_t)0);
  vm.add_read(OPCODE::READ_S, 3, 8);

  vm.add_write(OPCODE::WRITE_I, 0); // Should write 42
  vm.add_write(OPCODE::WRITE_D, 1); // Should write 3.14
  vm.add_write(OPCODE::WRITE_C, 2); // Should write z

  vm.add_load_inter(OPCODE::LOADIQ, 4, (int64_t)0);
  vm.add_write(OPCODE::WRITE_S, 4, 8); // Should write "hello"

  vm.add_halt();
  vm.run();

  // Restore buffers
  std::cin.rdbuf(old_cin);
  std::cout.rdbuf(old_cout);

  // Verify Reads
  ASSERT_EQ(vm.get_register(0).i, 42);
  ASSERT_DOUBLE_EQ(vm.get_register(1).d, 3.14);
  ASSERT_EQ(vm.get_register(2).c, 'z');
  ASSERT_STREQ((char *)vm.data().data(), "hello");

  // Verify Writes
  ASSERT_EQ(output.str(), "423.14zhello");
}