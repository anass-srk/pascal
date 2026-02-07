#pragma once

#include <vector>
#include <iostream>
#include <array>

namespace pascal_vm
{
  template <typename T, typename... U>
  concept one_of = (std::is_same_v<T, U> || ...);

  using std::uint64_t;
  using std::int8_t;
  using std::uint8_t;

  template <typename T>
  concept Type = std::is_same_v<T, int8_t> || std::is_same_v<T, int64_t> || std::is_same_v<T, double>;

  // --- Opcodes (1 Byte) ---
  // TODO: Add versions that take intermediate values
  enum class OPCODE : int8_t
  {
    // Load/Store
    LOAD_I,
    LOAD_D,
    LOAD_B,
    MOV,
    STORE_I,
    STORE_D,
    STORE_B,

    // Stack Operations (for intermediate values for functions)
    PUSH,
    POP,

    // Arithmetic (Int)
    ADD_I,
    SUB_I,
    MUL_I,
    DIV_I,
    MOD_I,
    // Arithmetic (Double)
    ADD_D,
    SUB_D,
    MUL_D,
    DIV_D,
    // Arithmetic (Char)
    ADD_B,
    SUB_B,
    MUL_B,
    DIV_B,
    MOD_B,
    // Comparison
    CMP_I,
    CMP_D,
    CMP_B,
    // Jumps
    JMP,
    JLT,
    JGT,
    JLE,
    JGE,
    JEQ,
    JNE,

    HALT
  };

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
    using RegValue = std::uint64_t;

    std::vector<uint8_t> code;
    mutable std::array<RegValue, NUM_REGISTERS> registers;
    mutable std::vector<RegValue> stack; // Runtime Stack
    mutable Flags flags;
    mutable size_t pc = 0;

  public:
    
    VM();

    // Runtime Execution
    void load_program(const std::vector<uint8_t> &bytecode);

    void run();

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

    // Instructions

    using LoadMap = TypeToEnumMap<
      TypeEnum<int8_t, OPCODE::LOAD_B>,
      TypeEnum<int64_t, OPCODE::LOAD_I>,
      TypeEnum<double, OPCODE::LOAD_D>
    >;

    template <Type T>
    void add_load(uint8_t reg, T imm)
    {
      add_value(LoadMap::get<T>());
      add_value(reg);
      add_value(imm);
    }

    using StoreMap = TypeToEnumMap<
      TypeEnum<int8_t, OPCODE::STORE_B>,
      TypeEnum<int64_t, OPCODE::STORE_I>,
      TypeEnum<double, OPCODE::STORE_D>
    >;

    template <Type T>
    void add_store(uint8_t reg, uint64_t addr)
    {
      add_value(StoreMap::get<T>());
      add_value(reg);
      add_value(addr);
    }

    void add_mov(uint8_t dest, uint8_t src)
    {
      add_value(static_cast<int8_t>(OPCODE::MOV));
      add_value(dest);
      add_value(src);
    }

    void add_halt()
    {
      add_value(static_cast<int8_t>(OPCODE::HALT));
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
    
    inline int64_t fetch_int64() const
    {
      int64_t val;
      std::memcpy(&val, &code[pc], 8);
      pc += 8;
      return val;
    }
    
    // To be used with jmp instructions, with 1 byte padding after the opcode and before the offset
    inline int32_t fetch_offset() const
    {
      int32_t val;
      std::memcpy(&val, &code[++pc], 4);
      pc += 4;
      return val;
    }

    // Execution
    void step() const;
  };

}