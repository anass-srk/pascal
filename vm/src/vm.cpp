#include "vm.hpp"
#include <string>

namespace pascal_vm
{

VM::VM() : registers({}), flags({})
{
  code.reserve(1024);
}

void VM::dump_state() const
{
  std::cout << "\n################################\n";
  std::cout << "PC : " << std::right << std::setfill('0') <<  std::setw(10) << pc << std::setfill(' '); 
  std::cout << "    FLAGS(Z, N)    " << '(' << (flags.Z == true) << ", " << (flags.N == true) << ')' << '\n' ;
  std::cout << std::left;  
  std::cout << std::setw(8) << "Name" << std::setw(8) << "Char" << std::setw(8) << "Int" << std::setw(8) << "Double" << '\n';
  std::cout << "================================\n";
  for(int i = 0;i < registers.size();++i)
  {
    const auto& r = registers[i];
    std::cout << std::setw(8) << "R" + std::to_string(i) << std::setw(8) << int(r.c) << std::setw(8) << r.i << std::setw(8) << r.d << '\n';
  }
  std::cout << '\n';
}

#define get_op_args() \
  const uint8_t dest = fetch_reg(); \
  const uint8_t src1 = fetch_reg(); \
  const uint8_t src2 = fetch_reg(); \
  print_op(opcode, dest, src1, src2, std::optional<int8_t>{});

#define get_op_args_inter(T)  \
  const uint8_t dest = fetch_reg(); \
  const uint8_t src = fetch_reg();  \
  if constexpr (sizeof(T) > 1) fetch_byte(); \
  const T val = fetch_value<T>(); \
  print_op(opcode, dest, src, -1, std::optional(val));

#define get_cmp_args() \
  const uint8_t a = fetch_reg();  \
  const uint8_t b = fetch_reg();  \
  fetch_byte(); \
  print_op(opcode, a, b, -1, std::optional<int8_t>{});

#define get_cmp_args_inter(T)  \
  const uint8_t a = fetch_reg();  \
  const T b = fetch_value<T>();   \
  if constexpr (sizeof(T) == 1) fetch_byte(); \
  print_op(opcode, a, -1, -1, std::optional(b));

#define get_jmp_args()  \
  fetch_byte(); \
  const int32_t offset = fetch_offset();  \
  print_op(opcode, -1, -1, -1, std::optional(offset));

#define get_load_args_inter(T) \
  const uint8_t reg = fetch_reg();  \
  const T val = fetch_value<T>(); \
  if constexpr(sizeof(T) == 1) fetch_byte(); \
  print_op(opcode, reg, -1, -1, std::optional(val));

#define get_load_args()  \
  const uint8_t reg = fetch_reg();  \
  const uint64_t addr = fetch_addr();\
  print_op(opcode, reg, -1, -1, std::optional(addr));

#define get_store_args() \
  const uint8_t reg = fetch_reg();  \
  const uint64_t addr = fetch_addr();\
  print_op(opcode, reg, -1, -1, std::optional(addr));

#define get_load_local_args() \
  const uint8_t reg = fetch_reg();  \
  const int32_t offset = fetch_offset();\
  print_op(opcode, reg, -1, -1, std::optional(offset));

#define get_store_local_args() \
  const uint8_t reg = fetch_reg();  \
  const int32_t offset = fetch_offset();\
  print_op(opcode, reg, -1, -1, std::optional(offset));

#define get_push_args() \
  const uint8_t reg = fetch_reg();  \
  print_op(opcode, reg, -1, -1, std::optional<int8_t>{});

#define get_pop_args()  \
  const uint8_t reg = fetch_reg();  \
  print_op(opcode, reg, -1, -1, std::optional<int8_t>{});

#define get_io_args() \
  const uint8_t reg = fetch_reg();  \
  print_op(opcode, reg, -1, -1, std::optional<uint8_t>{});

#define get_io_string_args()           \
  const uint8_t reg = fetch_reg();           \
  const auto len = fetch_value<uint32_t>(); \
  print_op(opcode, reg, -1, -1, std::optional(len));

void VM::run() const
{
  bool running = true;
  while (running)
  {
  print_pc(pc);
  auto opcode = static_cast<OPCODE>(fetch_byte());
  switch(opcode)
  {

    case OPCODE::NOP:{
      continue;
    }break;
    case OPCODE::MOV:{
      const uint8_t dest = fetch_reg();
      const uint8_t src = fetch_reg();
      fetch_byte();
      registers[dest].u = registers[src].u;
      print_op(opcode, dest, src, -1, std::optional<int8_t>{});
    }break;

    case OPCODE::LOADIB:{
      get_load_args_inter(uint8_t)
      registers[reg].byte = val;
    }break;
    case OPCODE::LOADIQ:{
      get_load_args_inter(uint64_t)
      registers[reg].u = val;
    }break;

    case OPCODE::LOADB:{
      get_load_args()
      std::memcpy(&registers[reg].byte, &stack[addr], 1);
    }break;
    case OPCODE::LOADQ:{
      get_load_args()
      std::memcpy(&registers[reg].u, &stack[addr], 8);
    }break;
    
    case OPCODE::LOADLB:{
      get_load_local_args()
      std::memcpy(&registers[reg].byte, &stack[registers[NUM_REGISTERS-1].u + offset], 1);
    }break;
    case OPCODE::LOADLQ:{
      get_load_local_args()
      std::memcpy(&registers[reg].u, &stack[registers[NUM_REGISTERS-1].u + offset], 8);
    }break;
    
    case OPCODE::STOREB:{
      get_store_args()
      std::memcpy(&stack[addr], &registers[reg].byte, 1);
    }break;
    case OPCODE::STOREQ:{
      get_store_args()
      std::memcpy(&stack[addr], &registers[reg].u, 8);
    }break;

    case OPCODE::STORELB:{
      get_store_local_args()
      std::memcpy(&stack[registers[NUM_REGISTERS-1].u + offset], &registers[reg].byte, 1);
    }break;
    case OPCODE::STORELQ:{
      get_store_local_args()
      std::memcpy(&stack[registers[NUM_REGISTERS-1].u + offset], &registers[reg].u, 8);
    }break;
    
    
    case OPCODE::ADD_I:{
      get_op_args()
      registers[dest].i = registers[src1].i + registers[src2].i;
    }break;
    case OPCODE::SUB_I:{
      get_op_args()
      registers[dest].i = registers[src1].i - registers[src2].i;
    }break;
    case OPCODE::MUL_I:{
      get_op_args()
      registers[dest].i = registers[src1].i * registers[src2].i;
    }break;
    case OPCODE::DIV_I:{
      get_op_args()
      registers[dest].i = registers[src1].i / registers[src2].i;
    }break;
    case OPCODE::MOD_I:{
      get_op_args()
      registers[dest].i = registers[src1].i % registers[src2].i;
    }break;

    case OPCODE::ADDI_I:{
      get_op_args_inter(int64_t)
      registers[dest].i = registers[src].i + val;
    }break;
    case OPCODE::SUBI_I:{
      get_op_args_inter(int64_t)
      registers[dest].i = registers[src].i - val;
    }break;
    case OPCODE::MULI_I:{
      get_op_args_inter(int64_t)
      registers[dest].i = registers[src].i * val;
    }break;
    case OPCODE::DIVI_I:{
      get_op_args_inter(int64_t)
      registers[dest].i = registers[src].i / val;
    }break;
    case OPCODE::MODI_I:{
      get_op_args_inter(int64_t)
      registers[dest].i = registers[src].i % val;
    }break;

    case OPCODE::ADD_C:{
      get_op_args()
      registers[dest].c = registers[src1].c + registers[src2].c;
    }break;
    case OPCODE::SUB_C:{
      get_op_args()
      registers[dest].c = registers[src1].c - registers[src2].c;
    }break;
    case OPCODE::MUL_C:{
      get_op_args()
      registers[dest].c = registers[src1].c * registers[src2].c;
    }break;
    case OPCODE::DIV_C:{
      get_op_args()
      registers[dest].c = registers[src1].c / registers[src2].c;
    }break;
    case OPCODE::MOD_C:{
      get_op_args()
      registers[dest].c = registers[src1].c % registers[src2].c;
    }break;

    case OPCODE::ADD_D:{
      get_op_args()
      registers[dest].d = registers[src1].d + registers[src2].d;
    }break;
    case OPCODE::SUB_D:{
      get_op_args()
      registers[dest].d = registers[src1].d - registers[src2].d;
    }break;
    case OPCODE::MUL_D:{
      get_op_args()
      registers[dest].d = registers[src1].d * registers[src2].d;
    }break;
    case OPCODE::DIV_D:{
      get_op_args()
      registers[dest].d = registers[src1].d / registers[src2].d;
    }break;

    case OPCODE::ADDI_C:{
      get_op_args_inter(int8_t)
      registers[dest].c = registers[src].c + val;
    }break;
    case OPCODE::SUBI_C:{
      get_op_args_inter(int8_t)
      registers[dest].c = registers[src].c - val;
    }break;
    case OPCODE::MULI_C:{
      get_op_args_inter(int8_t)
      registers[dest].c = registers[src].c * val;
    }break;
    case OPCODE::DIVI_C:{
      get_op_args_inter(int8_t)
      registers[dest].c = registers[src].c / val;
    }break;
    case OPCODE::MODI_C:{
      get_op_args_inter(int8_t)
      registers[dest].c = registers[src].c % val;
    }break;

    case OPCODE::ADDI_D:{
      get_op_args_inter(double)
      registers[dest].d = registers[src].d + val;
    }break;
    case OPCODE::SUBI_D:{
      get_op_args_inter(double)
      registers[dest].d = registers[src].d - val;
    }break;
    case OPCODE::MULI_D:{
      get_op_args_inter(double)
      registers[dest].d = registers[src].d * val;
    }break;
    case OPCODE::DIVI_D:{
      get_op_args_inter(double)
      registers[dest].d = registers[src].d / val;
    }break;

    case OPCODE::CMP_I:{
      get_cmp_args()
      flags.N = (registers[a].i < registers[b].i);
      flags.Z = (registers[a].i == registers[b].i);
    }break;
    case OPCODE::CMPI_I:{
      get_cmp_args_inter(int64_t)
      flags.N = (registers[a].i < b);
      flags.Z = (registers[a].i == b);
    }break;

    case OPCODE::JMP:{
      get_jmp_args()
      pc += offset;
    }break;
    case OPCODE::JEQ:{
      get_jmp_args()
      if(flags.Z) pc += offset;
    }break;
    case OPCODE::JNE:{
      get_jmp_args()
      if(!flags.Z) pc += offset;
    }break;
    case OPCODE::JGT:{
      get_jmp_args()
      if(!flags.N && !flags.Z) pc += offset;
    }break;
    case OPCODE::JGE:{
      get_jmp_args()
      if(flags.Z || !flags.N) pc += offset;
    }break;
    case OPCODE::JLT:{
      get_jmp_args()
      if(flags.N && !flags.Z) pc += offset;
    }break;
    case OPCODE::JLE:{
      get_jmp_args()
      if(flags.Z || flags.N) pc += offset;
    }break;

    case OPCODE::PUSHB:{
      get_push_args()
      add_var<int8_t>(registers[reg].c);
    }break;
    case OPCODE::PUSHQ:{
      get_push_args()
      add_var<int64_t>(registers[reg].i);
    }break;
    case OPCODE::POPB:{
      get_pop_args()
      registers[reg].byte = stack.back();
      stack.pop_back();
    }break;
    case OPCODE::POPQ:{
      get_pop_args()
      std::memcpy(&registers[reg].i, &stack[stack.size()-8], 8);
      stack.resize(stack.size() - 8);
    }break;

    case OPCODE::CALL:{
      fetch_byte();
      const auto func_addr = fetch_value<size_t>();
      print_op(OPCODE::CALL, -1, -1, -1, std::optional(func_addr), std::optional(registers[NUM_REGISTERS-1].u));

      add_var(pc); // return address on the vars stack is pc (now pointing at next instruction)
      pc = func_addr;
      add_var(registers[NUM_REGISTERS-1].u);
      registers[NUM_REGISTERS-1].u = stack.size();
    }break;

    case OPCODE::RET:{
      fetch_byte();
      const auto func_stack_size = fetch_value<uint32_t>();

      stack.resize(stack.size() - func_stack_size);

      std::memcpy(&registers[NUM_REGISTERS-1], &stack[stack.size() - 8], 8);

      size_t ret_addr;
      std::memcpy(&ret_addr, &stack[stack.size() - 16], 8);
      stack.resize(stack.size() - 16);
      pc = ret_addr;

      print_op(OPCODE::RET, -1, -1, -1, std::optional(func_stack_size), std::optional(ret_addr));
    }break;

    case OPCODE::READ_I:{
      get_io_args()
      std::cin >> registers[reg].i;
    }break;
    case OPCODE::READ_C:{
      get_io_args()
      std::cin >> registers[reg].c;
    }break;
    case OPCODE::READ_D:{
      get_io_args()
      std::cin >> registers[reg].d;
    }break;
    case OPCODE::READ_S:{
      get_io_string_args()
      std::string tmp;
      std::cin >> tmp;
      char *str = reinterpret_cast<char *>(stack.data() + registers[reg].u);
      if(tmp.length() >= len){
        std::memcpy(str, tmp.data(), len);
      }else{
        std::memcpy(str, tmp.data(), tmp.length());
        str[tmp.length()+1] = '\0';
      }
    }break;
    case OPCODE::WRITE_I:{
      get_io_args()
      std::cout << registers[reg].i;
    }break;
    case OPCODE::WRITE_C:{
      get_io_args()
      std::cout << registers[reg].c;
    }break;
    case OPCODE::WRITE_D:{
      get_io_args()
      std::cout << registers[reg].d;
    }break;
    case OPCODE::WRITE_S:{
      get_io_string_args()
      const char *str = reinterpret_cast<char *>(stack.data() + registers[reg].u);
      for(uint32_t i = 0;i < len && str[i] != '\0';++i){
        std::cout << str[i];
      }
    }break;

    case OPCODE::DMP:{
      dump_state();
      fetch_byte();
    }break;
    case OPCODE::HALT:{
      running = false;
    }break;
    default:{
      std::cerr << "Invalid opcode : " << static_cast<int>(opcode) << "!!!\n";
      running = false;
    }break;
  }

  }
}

};