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

  // --- Double Arithmetic ---
  {
    VM vm;

    vm.add_load_inter(OPCODE::LOADIQ, 0, 10.0);
    vm.add_load_inter(OPCODE::LOADIQ, 1, 4.0);

    vm.add_op(OPCODE::ADD_D, 2, 0, 1); // 10.0 + 4.0 = 14.0
    vm.add_op(OPCODE::SUB_D, 3, 0, 1); // 10.0 - 4.0 = 6.0
    vm.add_op(OPCODE::MUL_D, 4, 0, 1); // 10.0 * 4.0 = 40.0
    vm.add_op(OPCODE::DIV_D, 5, 0, 1); // 10.0 / 4.0 = 2.5

    vm.add_op_inter(OPCODE::ADDI_D, 7, 0, 5.0);  // 10.0 + 5.0 = 15.0
    vm.add_op_inter(OPCODE::SUBI_D, 8, 0, 5.0);  // 10.0 - 5.0 = 5.0
    vm.add_op_inter(OPCODE::MULI_D, 9, 0, 2.0);  // 10.0 * 2.0 = 20.0
    vm.add_op_inter(OPCODE::DIVI_D, 10, 0, 2.5); // 10.0 / 2.5 = 4.0

    vm.add_halt();
    vm.run();

    ASSERT_DOUBLE_EQ(vm.get_register(2).d, 14.0);
    ASSERT_DOUBLE_EQ(vm.get_register(3).d, 6.0);
    ASSERT_DOUBLE_EQ(vm.get_register(4).d, 40.0);
    ASSERT_DOUBLE_EQ(vm.get_register(5).d, 2.5);

    ASSERT_DOUBLE_EQ(vm.get_register(7).d, 15.0);
    ASSERT_DOUBLE_EQ(vm.get_register(8).d, 5.0);
    ASSERT_DOUBLE_EQ(vm.get_register(9).d, 20.0);
    ASSERT_DOUBLE_EQ(vm.get_register(10).d, 4.0);
  }

  // --- Char Arithmetic ---
  {
    VM vm;
    vm.add_load_inter(OPCODE::LOADIB, 0, (int8_t)10);
    vm.add_load_inter(OPCODE::LOADIB, 1, (int8_t)3);

    vm.add_op(OPCODE::ADD_C, 2, 0, 1); // 10 + 3 = 13
    vm.add_op(OPCODE::SUB_C, 3, 0, 1); // 10 - 3 = 7
    vm.add_op(OPCODE::MUL_C, 4, 0, 1); // 10 * 3 = 30
    vm.add_op(OPCODE::DIV_C, 5, 0, 1); // 10 / 3 = 3
    vm.add_op(OPCODE::MOD_C, 6, 0, 1); // 10 % 3 = 1

    vm.add_op_inter(OPCODE::ADDI_C, 7, 0, (int8_t)5); // 10 + 5 = 15
    vm.add_op_inter(OPCODE::SUBI_C, 8, 0, (int8_t)5); // 10 - 5 = 5
    vm.add_op_inter(OPCODE::MULI_C, 9, 0, (int8_t)5); // 10 * 5 = 50
    vm.add_op_inter(OPCODE::DIVI_C, 10, 0, (int8_t)4); // 10 / 4 = 2
    vm.add_op_inter(OPCODE::MODI_C, 11, 0, (int8_t)6);  // 10 % 6 = 4

    vm.add_halt();
    vm.run();

    ASSERT_EQ(vm.get_register(2).c, 13);
    ASSERT_EQ(vm.get_register(3).c, 7);
    ASSERT_EQ(vm.get_register(4).c, 30);
    ASSERT_EQ(vm.get_register(5).c, 3);
    ASSERT_EQ(vm.get_register(6).c, 1);

    ASSERT_EQ(vm.get_register(7).c, 15);
    ASSERT_EQ(vm.get_register(8).c, 5);
    ASSERT_EQ(vm.get_register(9).c, 50);
    ASSERT_EQ(vm.get_register(10).c, 2);
    ASSERT_EQ(vm.get_register(11).c, 4);
  }
}

TEST(VMTest, CmpOps)
{
  auto test1 = []<typename T>(OPCODE op, T a, T b)
  {
    VM vm;

    constexpr auto loadi = (sizeof(T) > 1 ? OPCODE::LOADIQ : OPCODE::LOADIB);

    vm.add_load_inter(loadi, 0, a);
    vm.add_load_inter(loadi, 1, b);

    vm.add_cmp(op, 0, 1);

    vm.add_halt();
    vm.run();
    return vm.get_flags();
  };

  auto test2 = []<typename T>(OPCODE op, T a, T b)
  {
    VM vm;

    constexpr auto loadi = (sizeof(T) > 1 ? OPCODE::LOADIQ : OPCODE::LOADIB);
    constexpr auto load = (sizeof(T) > 1 ? OPCODE::LOADQ : OPCODE::LOADB);

    vm.add_load_inter(loadi, 0, a);

    vm.add_cmp_inter(op, 0, b);

    vm.add_halt();
    vm.run();
    return vm.get_flags();
  };

  auto test = [&]<typename T>(T a, T b, Flags f)
  {
    auto cmp = OPCODE::CMP_I;
    auto cmpi = OPCODE::CMPI_I;

    if(std::is_same_v<T, double>)
    {
      cmp = OPCODE::CMP_D;
      cmpi = OPCODE::CMPI_D;
    }
    else if (std::is_same_v<T, int8_t>)
    {
      cmp = OPCODE::CMP_C;
      cmpi = OPCODE::CMPI_C;
    }

    auto f1 = test1(cmp, a, b);
    auto f2 = test2(cmpi, a, b);
    
    ASSERT_EQ(f1.N, f2.N);
    ASSERT_EQ(f1.Z, f2.Z);
    ASSERT_EQ(f1.N, f.N);
    ASSERT_EQ(f1.Z, f.Z);
  };

  test((int64_t)5, (int64_t)5, Flags{.Z=true, .N=false});
  test((int64_t)-1, (int64_t)5, Flags{.Z=false, .N=true});
  test((int64_t)-1, (int64_t)-10, Flags{.Z=false, .N=false});
  
  test((double)5, (double)5, Flags{.Z=true, .N=false});
  test((double)-1, (double)5, Flags{.Z=false, .N=true});
  test((double)-1, (double)-10, Flags{.Z=false, .N=false});

  test((int8_t)5, (int8_t)5, Flags{.Z=true, .N=false});
  test((int8_t)-1, (int8_t)5, Flags{.Z=false, .N=true});
  test((int8_t)-1, (int8_t)-10, Flags{.Z=false, .N=false});
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

// Test basic CALL and RET: a function that does nothing .
TEST(VMTest, CallRetSimple)
{
  VM vm;

  // Jump over the function
  size_t jmp_to_main = vm.add_jmp(OPCODE::JMP);

  // --- Function code (does nothing) ---
  size_t func_addr = vm.get_current_location();
  vm.add_ret(0);

  // --- Main code ---
  vm.conf_jmp(jmp_to_main, vm.get_current_location());

  // Set last register (R15) to a known value (101) to verify it is restored
  vm.add_load_inter(OPCODE::LOADIQ, VM::NUM_REGISTERS-1, (uint64_t)101);

  vm.add_call(func_addr);
  vm.add_halt();

  vm.run();

  // After return, R15 should be restored to its original value (100)
  ASSERT_EQ(vm.get_register(15).u, 101);
  // Stack should be empty (no arguments, no locals)
  ASSERT_EQ(vm.data().size(), 0);
}

// Test function with a local variables + cleanup.
TEST(VMTest, CallRetWithLocals)
{
  VM vm;

  size_t jmp_to_main = vm.add_jmp(OPCODE::JMP);

  // --- Function code ---
  size_t func_addr = vm.get_current_location();
  // Allocate 9 bytes
  vm.add_push(OPCODE::PUSHQ, 0);
  vm.add_push(OPCODE::PUSHB, 0);
  
  vm.add_load_inter(OPCODE::LOADIQ, 1, (int64_t)42);
  vm.add_store_local(OPCODE::STORELQ, 1, 0);
  vm.add_load_inter(OPCODE::LOADIB, 1, (int8_t)'A');
  vm.add_store_local(OPCODE::STORELB, 1, 8);

  vm.add_load_local(OPCODE::LOADLQ, 2, 0);
  vm.add_load_local(OPCODE::LOADLB, 3, 8);

  // Return, cleaning up the local variables (8 bytes)
  vm.add_ret(9);

  // --- Main code ---
  vm.conf_jmp(jmp_to_main, vm.get_current_location());

  vm.add_call(func_addr);
  vm.add_halt();

  vm.run();

  ASSERT_EQ(vm.get_register(2).i, 42);
  ASSERT_EQ(vm.get_register(3).c, 'A');

  ASSERT_EQ(vm.data().size(), 0);
}

// Test function that accesses arguments.
TEST(VMTest, CallRetWithArgs)
{
  VM vm;

  size_t jmp_to_main = vm.add_jmp(OPCODE::JMP);

  // --- Function code ---
  size_t func_addr = vm.get_current_location();
  // Expects two arguments (8 byte + 1 byte).
  vm.add_load_local(OPCODE::LOADLQ, 1, -8); // first argument
  vm.add_load_local(OPCODE::LOADLB, 2, -9); // second argument
  vm.add_op(OPCODE::ADD_I, 0, 1, 2);        // R0 = arg1 + arg2
  vm.add_ret(0);                            // no locals

  // --- Main code ---
  size_t main_start = vm.get_current_location();
  vm.conf_jmp(jmp_to_main, main_start);

  // Push arguments in reverse order
  vm.add_load_inter(OPCODE::LOADIB, 1, (int8_t)5);
  vm.add_push(OPCODE::PUSHB, 1);
  vm.add_load_inter(OPCODE::LOADIQ, 2, (int64_t)10);
  vm.add_push(OPCODE::PUSHQ, 2);

  vm.add_call(func_addr);

  // After return, arguments remain on stack
  vm.add_pop(OPCODE::POPQ, 3); // pop 10
  vm.add_pop(OPCODE::POPB, 4); // pop 5
  vm.add_halt();

  vm.run();

  ASSERT_EQ(vm.get_register(0).i, 15); // 10 + 5
  ASSERT_EQ(vm.data().size(), 0);      // stack empty after pops
}

// Test nested calls: main => func1 => func2
TEST(VMTest, NestedCalls)
{
  VM vm;

  size_t jmp_to_main = vm.add_jmp(OPCODE::JMP);

  // --- Function 2 (doubles its argument) ---
  size_t func2_addr = vm.get_current_location();
  vm.add_load_local(OPCODE::LOADLQ, 1, -8); // argument
  vm.add_op_inter(OPCODE::MULI_I, 0, 1, (int64_t)2);
  vm.add_ret(0);

  // --- Function 1 (calls func2, then adds 5) ---
  size_t func1_addr = vm.get_current_location();
  // Push argument for func2 (the same argument we received)
  vm.add_load_local(OPCODE::LOADLQ, 1, -8); // argument
  vm.add_push(OPCODE::PUSHQ, 1);
  vm.add_call(func2_addr);
  // Pop the argument after return and discard it
  vm.add_pop(OPCODE::POPQ, 2);
  // Add 5 to result
  vm.add_op_inter(OPCODE::ADDI_I, 0, 0, (int64_t)5);
  vm.add_ret(0);

  // --- Main code ---
  vm.conf_jmp(jmp_to_main, vm.get_current_location());

  // Push argument for func1
  vm.add_load_inter(OPCODE::LOADIQ, 1, (int64_t)7);
  vm.add_push(OPCODE::PUSHQ, 1);
  vm.add_call(func1_addr);
  // Pop argument
  vm.add_pop(OPCODE::POPQ, 2);
  vm.add_halt();

  vm.run();

  // Expected: (7 * 2) + 5 = 19
  ASSERT_EQ(vm.get_register(0).i, 19);
  ASSERT_EQ(vm.data().size(), 0);
}

// Tests for MODSTK (stack modification)
TEST(VMTest, StackOps)
{

  // Test MODSTK with positive size: allocate extra stack space, store/load values
  {
    VM vm;
  
    // Initially stack is empty
    ASSERT_EQ(vm.data().size(), 0);
  
    // Allocate 16 bytes
    vm.add_mod_stack(16);
  
    // Write some data
    vm.add_load_inter(OPCODE::LOADIQ, 0, (int64_t)999);
    vm.add_store(OPCODE::STOREQ, 0, 0);
    vm.add_load_inter(OPCODE::LOADIB, 1, (int8_t)'X');
    vm.add_store(OPCODE::STOREB, 1, 8);
  
    vm.add_load(OPCODE::LOADQ, 2, 0);
    vm.add_load(OPCODE::LOADB, 3, 8);
  
    vm.add_halt();
    vm.run();
  
    // Verify stack size (should still be 16)
    ASSERT_EQ(vm.data().size(), 16);
  
    // Verify loaded values
    ASSERT_EQ(vm.get_register(0).i, 999);
    ASSERT_EQ(vm.get_register(1).c, 'X');
    ASSERT_EQ(vm.get_register(2).i, 999);
    ASSERT_EQ(vm.get_register(3).c, 'X');
  }
  
  // Test MODSTK with negative size: shrink the stack, then push/pop at new top
  {
    VM vm;
  
    vm.add_mod_stack(24);
  
    // Write some data at various offsets
    vm.add_load_inter(OPCODE::LOADIQ, 0, (int64_t)100);
    vm.add_store(OPCODE::STOREQ, 0, 0);
    vm.add_load_inter(OPCODE::LOADIB, 1, (int8_t)'w');
    vm.add_store(OPCODE::STOREB, 1, 8);
    vm.add_load_inter(OPCODE::LOADIB, 2, (int8_t)'A');
    vm.add_store(OPCODE::STOREB, 2, 9);
  
    // Now shrink by 15 bytes
    vm.add_mod_stack(-15);
  
    vm.add_load_inter(OPCODE::LOADIQ, 3, (int64_t)300);
    vm.add_push(OPCODE::PUSHQ, 3);
  
    vm.add_pop(OPCODE::POPQ, 4);
  
    vm.add_load(OPCODE::LOADQ, 5, 0);
    vm.add_load(OPCODE::LOADB, 6, 8);
  
    vm.add_halt();
    vm.run();
  
    // Final stack size should be 9
    ASSERT_EQ(vm.data().size(), 9);
  
    // Verify data
    ASSERT_EQ(vm.get_register(0).i, 100);
    ASSERT_EQ(vm.get_register(1).c, 'w');
    ASSERT_EQ(vm.get_register(2).c, 'A');
    ASSERT_EQ(vm.get_register(3).i, 300);
  
    ASSERT_EQ(vm.get_register(4).i, 300);
    ASSERT_EQ(vm.get_register(5).i, 100);
    ASSERT_EQ(vm.get_register(6).c, 'w');
  }
  
  // Test MODSTK with zero size: no change
  {
    VM vm;
  
    vm.add_push(OPCODE::PUSHQ, 0);
  
    vm.add_mod_stack(0); // should do nothing
  
    vm.add_pop(OPCODE::POPQ, 0);
    vm.add_halt();
    vm.run();
  
    ASSERT_EQ(vm.data().size(), 0);
  }
  
  // Test multiple MODSTK operations in sequence
  {
    VM vm;
  
    // Start empty
    vm.add_mod_stack(10);  // size = 10
    vm.add_mod_stack(5);   // size = 15
    vm.add_mod_stack(-3);  // size = 12
    vm.add_mod_stack(-12); // size = 0
  
    // Push a value to verify stack is usable
    vm.add_load_inter(OPCODE::LOADIQ, 0, (int64_t)42);
    vm.add_push(OPCODE::PUSHQ, 0);
    vm.add_pop(OPCODE::POPQ, 1);
    vm.add_halt();
    vm.run();
  
    ASSERT_EQ(vm.data().size(), 0);
    ASSERT_EQ(vm.get_register(1).i, 42);
  }

}
