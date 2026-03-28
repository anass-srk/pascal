#include "vm.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <string>

namespace pascal_vm {

void print_op(OPCODE op) {
  if (!DEBUG_PRINT_OP) return;
  std::cout << std::left << std::setw(8) << OPCODE_NAMES[int(op)] << '\n';
}

template <typename T>
void print_op(OPCODE op, T imm) {
  if (!DEBUG_PRINT_OP) return;
  std::cout << std::left << std::setw(8) << OPCODE_NAMES[int(op)] << std::setw(8) << imm << '\n';
}

#define pop_args(T)               \
  print_op(op);                   \
  const auto b = fetch_data<T>(); \
  const auto a = fetch_data<T>();
  

using Int = int64_t;
using Real = double;

void VM::run() const {
  while(true){
  const auto op = fetch_op();
  switch (op) {
    case OPCODE::PUSH_FP:{
      print_op(op);
      add_var(fp);
    }break;
    case OPCODE::PUSH_Q:{
      const auto data = fetch_code<Int>();
      print_op(op, data);
      add_var(data);
    }break;
    case OPCODE::PUSH_B:{
      const auto data = fetch_code<int8_t>();
      print_op(op, data);
      add_var(data);
    }break;

    case OPCODE::NOT:{
      print_op(op);
      const auto data = fetch_data<bool>();
      add_var(!data);
    }break;

    case OPCODE::ADD_I:{
      pop_args(Int)
      add_var(a+b);
    }break;
    case OPCODE::SUB_I:{
      pop_args(Int)
      add_var(a-b);
    }break;
    case OPCODE::MUL_I:{
      pop_args(Int)
      add_var(a*b);
    }break;
    case OPCODE::DIV_I:{
      pop_args(Int)
      add_var(a/b);
    }break;

    case OPCODE::ADD_C:{
      pop_args(char)
      add_var<char>(a+b);
    }break;
    case OPCODE::SUB_C:{
      pop_args(char)
      add_var<char>(a-b);
    }break;
    case OPCODE::MUL_C:{
      pop_args(char)
      add_var<char>(a*b);
    }break;
    case OPCODE::DIV_C:{
      pop_args(char)
      add_var<char>(a/b);
    }break;

    case OPCODE::ADD_R:{
      pop_args(Real)
      add_var(a+b);
    }break;
    case OPCODE::SUB_R:{
      pop_args(Real)
      add_var(a-b);
    }break;
    case OPCODE::MUL_R:{
      pop_args(Real)
      add_var(a*b);
    }break;
    case OPCODE::DIV_R:{
      pop_args(Real)
      add_var(a/b);
    }break;

    case OPCODE::AND:{
      pop_args(bool)
      add_var(a && b);
    }break;
    case OPCODE::OR:{
      pop_args(bool)
      add_var(a || b);
    }break;

    case OPCODE::CMP_I:{
      pop_args(Int)
      add_var((a == b ? char(0) : (a < b ? char(-1) : char(1))));
    }break;
    case OPCODE::CMP_C:{
      pop_args(char)
      add_var((a == b ? char(0) : (a < b ? char(-1) : char(1))));
    }break;
    case OPCODE::CMP_R:{
      pop_args(Real)
      add_var((a == b ? char(0) : (a < b ? char(-1) : char(1))));
    }break;

    case OPCODE::LE: {
      print_op(op);
      const char arg = fetch_data<char>();
      add_var(arg == 1 ? false : true);
    } break;
    case OPCODE::LT: {
      print_op(op);
      const char arg = fetch_data<char>();
      add_var(arg == -1 ? true : false);
    } break;
    case OPCODE::EQ: {
      print_op(op);
      const char arg = fetch_data<char>();
      add_var(arg == 0 ? true : false);
    } break;
    case OPCODE::NE: {
      print_op(op);
      const char arg = fetch_data<char>();
      add_var(arg == 0 ? false : true);
    } break;
    case OPCODE::GT: {
      print_op(op);
      const char arg = fetch_data<char>();
      add_var(arg == 1 ? true : false);
    } break;
    case OPCODE::GE: {
      print_op(op);
      const char arg = fetch_data<char>();
      add_var(arg == -1 ? false : true);
    } break;

    case OPCODE::JMP: {
      const auto data = fetch_addr();
      print_op(op, data);
      pc = data;
    } break;
    case OPCODE::JMP_TRUE: {
      const auto data = fetch_addr();
      print_op(op, data);

      const bool arg = fetch_data<bool>();
      if(arg) pc = data;
    } break;
    case OPCODE::JMP_FALSE: {
      const auto data = fetch_addr();
      print_op(op, data);

      const bool arg = fetch_data<bool>();
      if(!arg) pc = data;
    } break;

    case OPCODE::LOAD_Q: {
      print_op(op);
      const auto addr = fetch_data<size_t>();
      
      size_t data;
      std::memcpy(&data, &stack[addr], sizeof(data));
      add_var(data);
    } break;
    case OPCODE::LOAD_B: {
      print_op(op);
      const auto addr = fetch_data<size_t>();
      add_var(stack[addr]);
    } break;

    case OPCODE::STORE_Q: {
      print_op(op);
      const auto data = fetch_data<size_t>();
      const auto addr = fetch_data<size_t>();
      std::memcpy(&stack[addr], &data, sizeof(data));
    }break;
    case OPCODE::STORE_B: {
      print_op(op);
      const auto data = fetch_data<char>();
      const auto addr = fetch_data<size_t>();
      std::memcpy(&stack[addr], &data, sizeof(data));
    }break;

    case OPCODE::LOAD_N: {
      const auto size = fetch_code<uint32_t>();
      print_op(op, size);
      const auto addr = fetch_data<size_t>();
      stack.insert(stack.end(), stack.begin() + addr, stack.begin() + addr + size);
    } break;
    case OPCODE::STORE_N: {
      const auto size = fetch_code<uint32_t>();
      print_op(op, size);
      const auto data_addr = stack.size() - size;
      size_t dest_addr;
      std::memcpy(&dest_addr, &stack[data_addr - sizeof(dest_addr)], sizeof(dest_addr));
      std::memcpy(&stack[dest_addr], &stack[data_addr] , size);
      stack.resize(stack.size() - size - sizeof(dest_addr));
    } break;

    case OPCODE::POP_Q: {
      print_op(op);
      fetch_data<size_t>();
    }break;
    case OPCODE::POP_B: {
      print_op(op);
      fetch_data<char>();
    }break;

    case OPCODE::DUPL_B: {
      print_op(op);
      stack.push_back(stack.back());
    } break;

    case OPCODE::DUPL_Q: {
      print_op(op);
      const uint8_t* ptr = &stack[stack.size() - sizeof(size_t)];
      stack.insert(stack.end(), ptr, ptr + sizeof(size_t));
    } break;

    case OPCODE::MODSTK: {
      const auto data = fetch_code<int32_t>();
      print_op(op, data);
      stack.resize(Int(stack.size()) + data, 0); // The usage of 0 is important for string ptrs
    } break;

    case OPCODE::CALL: {
      const auto addr = fetch_code<size_t>();
      add_var(pc);      // return address
      add_var(fp);      // old frame pointer
      fp = stack.size();  // new frame pointer points at function vars/consts
      pc = addr;          // function location
    } break;
    case OPCODE::RET: {
      const auto size = fetch_code<size_t>();
      stack.resize(stack.size() - size);
      fp = fetch_data<size_t>();
      pc = fetch_data<size_t>();
    } break;

    case OPCODE::C2I: {
      print_op(op);
      const auto c = fetch_data<char>();
      add_var(Int(c));
    } break;

    case OPCODE::READ_I: {
      print_op(op);
      const auto addr = fetch_data<size_t>();

      Int data;
      std::cin >> data;

      std::memcpy(&stack[addr], &data, sizeof(data));
    } break;
    case OPCODE::READ_R: {
      print_op(op);
      const auto addr = fetch_data<size_t>();

      Real data;
      std::cin >> data;

      std::memcpy(&stack[addr], &data, sizeof(data));
    } break;
    case OPCODE::READ_S: {
      print_op(op);
      const auto len = fetch_data<size_t>();
      const auto addr = fetch_data<size_t>();

      std::string data;
      std::getline(std::cin, data);

      if(data.length() >= len) {
        std::memcpy(&stack[addr], data.data(), len-1);
        stack[addr+len-1] = '\0';
      } else {
        std::memcpy(&stack[addr], data.data(), data.length());
        stack[addr+data.length()] = '\0';
      }

    } break;
    case OPCODE::READ_C: {
      print_op(op);
      const auto addr = fetch_data<size_t>();

      char data;
      std::cin >> data;

      std::memcpy(&stack[addr], &data, sizeof(data));
    } break;
    case OPCODE::READ_B: {
      print_op(op);
      const auto addr = fetch_data<size_t>();

      bool data;
      std::cin >> data;

      std::memcpy(&stack[addr], &data, sizeof(data));
    } break;

    case OPCODE::WRITE_I: {
      print_op(op);
      const auto data = fetch_data<Int>();
      std::cout << data;
    } break;
    case OPCODE::WRITE_R: {
      print_op(op);
      const auto data = fetch_data<Real>();
      std::cout << data;
    } break;
    case OPCODE::WRITE_C: {
      print_op(op);
      const auto data = fetch_data<char>();
      std::cout << data;
    } break;
    case OPCODE::WRITE_B: {
      print_op(op);
      const auto data = fetch_data<bool>();
      std::cout << data;
    } break;
    case OPCODE::WRITE_S: {
      print_op(op);
      const auto addr = fetch_data<size_t>();
      std::cout << reinterpret_cast<const char*>(&stack[addr]);
    } break;
    case OPCODE::WRITE_CONST_S: {
      print_op(op);
      const auto addr = fetch_data<size_t>();
      std::cout << reinterpret_cast<const char *>(&code[addr]);
    } break;

    case OPCODE::HALT:{
      print_op(op);
      return;
    }break;
  }
}
}



}