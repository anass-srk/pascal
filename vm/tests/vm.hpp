#include <gtest/gtest.h>
#include "../src/vm.hpp"

using namespace pascal_vm;

TEST(VMTest, MovOps)
{
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
    ASSERT_EQ(vm.get_register(1).d, 10.56);
    ASSERT_EQ(vm.get_register(5).b, true);

    int res = 0;
    ASSERT_EQ(vm.data()[0], b);
    
    res = std::memcmp(&vm.data()[1], &d, sizeof(d));
    ASSERT_EQ(res, 0);
    
    ASSERT_EQ(vm.data()[9], c);

    res = std::memcmp(&vm.data()[10], &i, sizeof(i));
    ASSERT_EQ(res, 0);
  }
  
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
    ASSERT_EQ(vm.get_register(7).d, d);
    ASSERT_EQ(vm.get_register(6).b, b);
  }

  {
    VM vm;

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
    ASSERT_EQ(vm.get_register(5).d, d);
    ASSERT_EQ(vm.get_register(6).b, b);
    ASSERT_EQ(vm.get_register(10).b, b);
    ASSERT_EQ(vm.get_register(11).d, d);
  }
}