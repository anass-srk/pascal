#pragma once

#include <vector>
#include <iostream>
#include <iomanip>
#include <array>
#include <string>
#include <optional>
#include <algorithm>

namespace pascal_vm
{
  template <typename T, typename... U>
  concept one_of = (std::is_same_v<T, U> || ...);

  template <typename T>
  concept VarType = std::integral<T> || std::floating_point<T>;

  // --- Opcodes (1 Byte) ---
  // TODO: Add versions that take intermediate values
  enum class OPCODE : uint8_t
  {
    NOP,

    // Load/Store
    LOADQ,
    LOADB,
    // Intermediate
    LOADIQ,
    LOADIB,
    LOADLQ,
    LOADLB,
    MOV,
    STOREQ,
    STOREB,
    STORELQ,
    STORELB,

    // Stack Operations (from registers)
    PUSHB,
    PUSHQ,
    POPB,
    POPQ,

    // Function Operations (stack shape : ret_addr | args | function vars)
    CALL, // takes argument to unwind the stack
    RET,  // takes function stack size

    // Arithmetic (Int)
    ADD_I,
    SUB_I,
    MUL_I,
    DIV_I,
    MOD_I,
    // Intermidiate version
    ADDI_I,
    SUBI_I,
    MULI_I,
    DIVI_I,
    MODI_I,

    // Arithmetic (Char)
    ADD_C,
    SUB_C,
    MUL_C,
    DIV_C,
    MOD_C,
    // Intermidiate version
    ADDI_C,
    SUBI_C,
    MULI_C,
    DIVI_C,
    MODI_C,

    // Arithmetic (Double)
    ADD_D,
    SUB_D,
    MUL_D,
    DIV_D,
    // Intermidiate version
    ADDI_D,
    SUBI_D,
    MULI_D,
    DIVI_D,

    // Comparison
    CMP_I,
    CMP_D,
    CMP_C,
    CMPI_I,
    CMPI_D,
    CMPI_C,

    // Jumps
    JMP,
    JLT,
    JGT,
    JLE,
    JGE,
    JEQ,
    JNE,

    // Basic IO
    READ_C,
    READ_I,
    READ_D,
    READ_S,
    WRITE_C,
    WRITE_I,
    WRITE_D,
    WRITE_S,

    DMP,
    HALT
  };

  inline const char* OPCODE_NAMES[] =
  {
    "NOP",
    "LOADQ",
    "LOADB",
    "LOADIQ",
    "LOADIB",
    "LOADLQ",
    "LOADLB",
    "MOV",
    "STOREQ",
    "STOREB",
    "STORELQ",
    "STORELB",
    "PUSHB",
    "PUSHQ",
    "POPB",
    "POPQ",
    "CALL",
    "RET",
    "ADD_I",
    "SUB_I",
    "MUL_I",
    "DIV_I",
    "MOD_I",
    "ADDI_I",
    "SUBI_I",
    "MULI_I",
    "DIVI_I",
    "MODI_I",
    "ADD_C",
    "SUB_C",
    "MUL_C",
    "DIV_C",
    "MOD_C",
    "ADDI_C",
    "SUBI_C",
    "MULI_C",
    "DIVI_C",
    "MODI_C",
    "ADD_D",
    "SUB_D",
    "MUL_D",
    "DIV_D",
    "ADDI_D",
    "SUBI_D",
    "MULI_D",
    "DIVI_D",
    "CMP_I",
    "CMP_D",
    "CMP_C",
    "CMPI_I",
    "CMPI_D",
    "CMPI_C",
    "JMP",
    "JLT",
    "JGT",
    "JLE",
    "JGE",
    "JEQ",
    "JNE",
    "READ_C",
    "READ_I",
    "READ_D",
    "READ_S",
    "WRITE_C",
    "WRITE_I",
    "WRITE_D",
    "WRITE_S",
    "DMP",
    "HALT"
  };

#if DEBUG_VM
  constexpr bool DEBUG_PRINT_OP = true;
#else
  constexpr bool DEBUG_PRINT_OP = false;
#endif

  inline const char* REG_NAMES[16] = {"R0","R1","R2","R3","R4","R5","R6","R7","R8","R9","R10","R11","R12","R13","R14","R15"};

  template <VarType T, VarType U = T>
  static void print_op(OPCODE op, uint8_t dest, uint8_t src1, uint8_t src2, std::optional<T> val1, std::optional<U> val2 = std::nullopt)
  {
    if constexpr (!DEBUG_PRINT_OP) return;
    std::cout << std::left <<
    std::setw(8) << OPCODE_NAMES[static_cast<uint8_t>(op)] << 
    std::setw(8) << (dest == 255 ? "NAN" : REG_NAMES[dest]) << 
    std::setw(8) << (src1 == 255 ? "NAN" : REG_NAMES[src1]) << 
    std::setw(8) << (src2 == 255 ? "NAN" : REG_NAMES[src2]) << std::setw(8);
    if (val1.has_value()) std::cout << val1.value() << std::setw(8);
    else std::cout << "NAN" << std::setw(8);
    if (val2.has_value()) std::cout << val2.value() << '\n';
    else std::cout << "NAN" << '\n';
  }

  static void print_pc(size_t pc)
  {
    if constexpr (DEBUG_PRINT_OP)
      std::cout << std::left << std::setw(8) << pc;
  }

  // --- CPU Flags ---
  struct Flags
  {
    bool Z = false;
    bool N = false;
  };

  // --- The Virtual Machine ---
  class VM
  {
    static constexpr size_t NUM_REGISTERS = 16;
    using RegValue = union {
      uint64_t u;
      int64_t i;
      double d;
      int8_t c;
      bool b;
      uint8_t byte;
    };

    std::vector<uint8_t> code;
    mutable std::array<RegValue, NUM_REGISTERS> registers;
    mutable std::vector<uint8_t> stack; // Runtime Stack
    mutable Flags flags;
    mutable size_t pc = 0;
    std::vector<size_t> jmp_locs;

  public:
    
    VM();

    // Runtime Execution
    // void load_program(const std::vector<uint8_t> &bytecode);

    size_t get_current_location() const {return code.size();}
    std::vector<uint8_t>& data() const {return stack;}
    const RegValue& get_register(uint8_t i) const {return registers[i];};

    void run() const;

    void dump_state() const;

  private:
    
    // Helper for Instructions
    template <VarType T>
    inline void add_value(T v)
    {
      if constexpr (sizeof(T) == 1)
      {
        code.push_back(static_cast<uint8_t>(v));
      }
      else
      {
        const uint8_t *p = reinterpret_cast<const uint8_t*>(&v);
        code.insert(code.end(), p, p + sizeof(T));
      }
    }

  public:

    // Helper for vars
    template <VarType T>
    inline void add_var(T v) const
    {
      if constexpr (sizeof(T) == 1)
      {
        stack.push_back(static_cast<uint8_t>(v));
      }
      else
      {
        const uint8_t *p = reinterpret_cast<const uint8_t*>(&v);
        stack.insert(stack.end(), p, p + sizeof(T));
      }
    }

    // Instructions

    template <VarType T>
    void add_load_inter(OPCODE op, uint8_t reg, T imm)
    {
      add_value(static_cast<uint8_t>(op));
      add_value(reg);
      add_value(imm);
      if(sizeof(T) == 1)
      {
        add_value<uint8_t>(0);
      }
    }

    void add_load(OPCODE op, uint8_t reg, uint64_t addr)
    {
      add_value(static_cast<uint8_t>(op));
      add_value(reg);
      add_value(addr);
    }

    // offset < 0 to access arguments (-16 to skip both return address and saved old base pointer value)
    // offset >= 0 to access function vars in the stack
    void add_load_local(OPCODE op, uint8_t reg, int32_t offset)
    {
      add_value(static_cast<uint8_t>(op));
      add_value(reg);
      add_value((offset >= 0 ? offset : offset - 16));
    }

    void add_store(OPCODE op, uint8_t reg, uint64_t addr)
    {
      add_value(static_cast<uint8_t>(op));
      add_value(reg);
      add_value(addr);
    }

    void add_store_local(OPCODE op, uint8_t reg, int32_t offset)
    {
      add_value(static_cast<uint8_t>(op));
      add_value(reg);
      add_value((offset >= 0 ? offset : offset - 16));
    }

    void add_mov(uint8_t dest, uint8_t src)
    {
      add_value(static_cast<uint8_t>(OPCODE::MOV));
      add_value(dest);
      add_value(src);
      add_value<uint8_t>(0);
    }

    void add_halt()
    {
      add_value(static_cast<uint8_t>(OPCODE::HALT));
    }


    inline void add_op(OPCODE op, uint8_t dest, uint8_t src1, uint8_t src2)
    {
      add_value(static_cast<uint8_t>(op));
      add_value(dest);
      add_value(src1);
      add_value(src2);
    }

    template <VarType T>
    inline void add_op_inter(OPCODE op, uint8_t dest, uint8_t src, T imm)
    {
      add_value(static_cast<uint8_t>(op));
      add_value(dest);
      add_value(src);
      if constexpr (sizeof(T) > 1)
      {
        add_value<uint8_t>(0);
      }
      add_value(imm);
    }

    inline void add_cmp(OPCODE op, uint8_t a, uint8_t b)
    {
      add_value(static_cast<uint8_t>(op));
      add_value(a);
      add_value(b);
      add_value<uint8_t>(0);
    }

    template <VarType T>
    inline void add_cmp_inter(OPCODE op, uint8_t a, T b)
    {
      add_value(static_cast<uint8_t>(op));
      add_value(a);
      add_value(b);
      if constexpr (sizeof(T) == 1)
      {
        add_value<uint8_t>(0);
      }
    }

    inline size_t add_jmp(OPCODE op)
    {
      const size_t size = code.size();
      jmp_locs.push_back(size);
      add_value(static_cast<uint8_t>(op));
      add_value<uint8_t>(0);
      add_value<int32_t>(0);
      return size;
    }

    inline bool conf_jmp(size_t jmp, size_t loc)
    {
      if(jmp+6 > code.size() || loc > code.size()) return false;
      if (!std::binary_search(jmp_locs.begin(), jmp_locs.end(), jmp)) return false;
      const int32_t offset = -(int32_t)(jmp - loc) - 6;
      std::memcpy(&code[jmp+2], &offset, sizeof(offset));
      return true;
    }

    inline void add_dump()
    {
      add_value(static_cast<uint8_t>(OPCODE::DMP));
      add_value<uint8_t>(0);
    }

    inline void add_push(OPCODE op, uint8_t reg)
    {
      add_value(static_cast<uint8_t>(op));
      add_value(reg);
    }

    inline void add_pop(OPCODE op, uint8_t reg)
    {
      add_value(static_cast<uint8_t>(op));
      add_value(reg);
    }

    // stack shape : args | ret_addr | function vars
    // pushes old base pointer value found in the last register
    inline void add_call(size_t func_addr)
    {
      add_value(static_cast<uint8_t>(OPCODE::CALL));
      add_value<uint8_t>(0);
      add_value(func_addr);
    }

    inline void add_ret(uint32_t func_stack_size)
    {
      add_value(static_cast<uint8_t>(OPCODE::RET));
      add_value<uint8_t>(0);
      add_value(func_stack_size);
    }

    inline void add_read(OPCODE opcode, uint8_t reg)
    {
      add_value(static_cast<uint8_t>(opcode));
      add_value(reg);
    }

    inline void add_read(OPCODE opcode, uint8_t reg, uint32_t len) // for array of chars
    {
      add_value(static_cast<uint8_t>(opcode));
      add_value(reg);
      add_value(len);
    }

    inline void add_write(OPCODE opcode, uint8_t reg)
    {
      add_value(static_cast<uint8_t>(opcode));
      add_value(reg);
    }

    inline void add_write(OPCODE opcode, uint8_t reg, uint32_t len) // for array of chars
    {
      add_value(static_cast<uint8_t>(opcode));
      add_value(reg);
      add_value(len);
    }

  private:

    // Fetching
    inline uint8_t fetch_byte() const
    {
      return code[pc++];
    }
    
    inline uint8_t fetch_reg() const
    {
      return code[pc++];
    }

    template <VarType T>
    inline T fetch_value() const
    {
      T val;
      std::memcpy(&val, &code[pc], sizeof(T));
      pc += sizeof(T);
      return val;
    }
    
    inline uint64_t fetch_addr() const
    {
      uint64_t val;
      std::memcpy(&val, &code[pc], 8);
      pc += 8;
      return val;
    }
    
    // To be used with jmp instructions, with 1 byte padding after the opcode and before the offset
    // Therefore, adding a jmp instructions + its operands uses 6 bytes
    inline int32_t fetch_offset() const
    {
      int32_t val;
      std::memcpy(&val, &code[pc], 4);
      pc += 4;
      return val;
    }

  };

}