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
  concept Type = std::is_same_v<T, bool> || std::is_same_v<T, int8_t> || std::is_same_v<T, int64_t> || std::is_same_v<T, double>;

  // --- Opcodes (1 Byte) ---
  // TODO: Add versions that take intermediate values
  enum class OPCODE : uint8_t
  {
    NOP,

    // Load/Store
    LOAD_I,
    LOAD_D,
    LOAD_B,
    // Intermediate
    LOADI_I,
    LOADI_D,
    LOADI_B,
    MOV,
    STORE_I,
    STORE_D,
    STORE_B,

    // Stack Operations (for intermediate values for functions)
    PUSHB,
    PUSHQ,
    POPB,
    POPQ,

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
    MODI_D,

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

    DMP,
    HALT
  };

  inline const char* OPCODE_NAMES[] =
  {
    "NOP",
    "LOAD_I",
    "LOAD_D",
    "LOAD_B",
    "LOADI_I",
    "LOADI_D",
    "LOADI_B",
    "MOV",
    "STORE_I",
    "STORE_D",
    "STORE_B",
    "PUSHB",
    "PUSHQ",
    "POPB",
    "POPQ",
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
    "MODI_D",
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
    "DMP",
    "HALT"
  };

#ifdef DEBUG_VM
  constexpr bool DEBUG_PRINT_OP = true;
#else
  constexpr bool DEBUG_PRINT_OP = false;
#endif

  inline const char* REG_NAMES[16] = {"R0","R1","R2","R3","R4","R5","R6","R7","R8","R9","R10","R11","R12","R13","R14","R15"};

  template <typename T>
  void print_op(OPCODE op, uint8_t dest, uint8_t src1, uint8_t src2, std::optional<T> val)
  requires (std::integral<T> || std::floating_point<T>)
  {
    if constexpr (!DEBUG_PRINT_OP) return;
    std::cout << '\n' << std::left << std::setw(8) << OPCODE_NAMES[static_cast<uint8_t>(op)] << 
    std::setw(8) << (dest == 255 ? "NAN" : REG_NAMES[dest]) << 
    std::setw(8) << (src1 == 255 ? "NAN" : REG_NAMES[src1]) << 
    std::setw(8) << (src2 == 255 ? "NAN" : REG_NAMES[src2]) << std::setw(8);
    if (val.has_value()) std::cout << val.value() << '\n';
    else std::cout << "NAN" << '\n';
  }

  template <typename Key, OPCODE Value>
  struct TypeEnum
  {
    using key = Key;
    static constexpr OPCODE value = Value;
  };

  template <typename P, typename... Ps>
  struct TypeToEnumMap
  {
    template <typename T>
    static constexpr OPCODE get()
    {
      if constexpr (std::is_same_v<typename P::key, T>)
        return P::value;
      else
        return TypeToEnumMap<Ps...>::template get<T>();
    }
  };

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
      char c;
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

    void run() const;

    void dump_state() const;

  private:
    
    // Helper for Instructions
    template <typename T>
    inline void add_value(T v) requires one_of<T, uint8_t, int8_t, int32_t, int64_t, uint64_t, double>
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
    template <typename T>
    inline void add_var(T v) const requires one_of<T, uint8_t, int8_t, int32_t, int64_t, uint64_t, double>
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
    using LoadMap = TypeToEnumMap<
      TypeEnum<int8_t, OPCODE::LOADI_B>,
      TypeEnum<int64_t, OPCODE::LOADI_I>,
      TypeEnum<double, OPCODE::LOADI_D>
    >;

    template <Type T>
    void add_load_inter(uint8_t reg, T imm)
    {
      add_value(static_cast<uint8_t>(LoadMap::get<T>()));
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

    using StoreMap = TypeToEnumMap<
      TypeEnum<int8_t, OPCODE::STORE_B>,
      TypeEnum<int64_t, OPCODE::STORE_I>,
      TypeEnum<double, OPCODE::STORE_D>
    >;

    template <Type T>
    void add_store(uint8_t reg, uint64_t addr)
    {
      add_value(static_cast<uint8_t>(StoreMap::get<T>()));
      add_value(reg);
      add_value(addr);
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

    template <Type T>
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

    template <Type T>
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

    template <Type T>
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