#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>


namespace pascal_vm {

template <typename T, typename... U>
concept one_of = (std::is_same_v<T, U> || ...);

template <typename T>
concept VarType = std::integral<T> || std::floating_point<T>;

// --- Opcodes (1 Byte) ---
// immediate arg = embedded in code
enum class OPCODE : uint8_t {
  PUSH_S, // immediate arg (8-byte absolute address of string (32-bit len +
          // char*)) from constants -> push into string_stack (for strings added using add_string)
  PUSH_Q, // immediate arg (8-bytes) -> push into stack
  PUSH_B, // immediate arg (1-byte)  -> push into stack
  PUSH_FP, // no arg, pushes frame pointer into stack

  NOT, // negates boolean at the top of the stack

  // these operations pop 2 elements from the stack (Int, Real, Char), apply
  // operation -> push result into stack
  ADD_I,
  SUB_I,
  MUL_I,
  DIV_I,

  ADD_R,
  SUB_R,
  MUL_R,
  DIV_R,

  ADD_C,
  SUB_C,
  MUL_C,
  DIV_C,

  ADD_S, // for strings (into stack)

  // pop 2 bools, push operation result
  AND,
  OR,

  // pop 2 elements, compare : left < right -> push char(-1) | left == right ->
  // push char(0) | left > right -> push char(1) simplifies jumping instructions
  // : type agnostic + no need to have flags (there's no else if, so the result
  // is checked once in if)
  CMP_I,
  CMP_R,
  CMP_C,
  CMP_S, // when not equal, compares in alphanumeric order

  // pops 1 char, pushes boolean. It's true if and only if
  LE, // -1 or 0
  LT, // -1
  EQ, //  0
  NE, // -1 or 1
  GT, //  1       
  GE, //  0 or 1

  JMP, // immediate absolute address in the code

  // immediate absolute address in the code, pops 1 boolean
  JMP_TRUE,
  JMP_FALSE,

  MODSTK, // immediate 32-bit num to increase/decrease the stack

  // pops 2 elements addr + data, moves data to addr
  STORE_Q,
  STORE_B,
  STORE_S,

  // pops addr, pushes data
  LOAD_Q,
  LOAD_B,
  LOAD_S,

  // pops top of the stack
  POP_Q,
  POP_B,
  POP_S,

  WRITE_I,
  WRITE_R,
  WRITE_C,
  WRITE_S,

  READ_I,
  READ_R,
  READ_C,
  READ_S,

  HALT,
  MAX = HALT
};

inline const char* OPCODE_NAMES[int(OPCODE::MAX)+1] = {
  "PUSH_S",  
  "PUSH_Q",  
  "PUSH_B",  
  "PUSH_FP", 
  "NOT", 
  "ADD_I",
  "SUB_I",
  "MUL_I",
  "DIV_I",
  "ADD_R",
  "SUB_R",
  "MUL_R",
  "DIV_R",
  "ADD_C",
  "SUB_C",
  "MUL_C",
  "DIV_C",
  "ADD_S", 
  "AND",
  "OR",
  "CMP_I",
  "CMP_R",
  "CMP_C",
  "CMP_S", 
  "LE", 
  "LT", 
  "EQ", 
  "NE", 
  "GT", 
  "GE",
  "JMP",
  "JMP_TRUE",
  "JMP_FALSE",
  "MODSTK",
  "STORE_Q",
  "STORE_B",
  "STORE_S",
  "LOAD_Q",
  "LOAD_B",
  "LOAD_S",
  "POP_Q",
  "POP_B",
  "POP_S",
  "WRITE_I",
  "WRITE_R",
  "WRITE_C",
  "WRITE_S",
  "READ_I",
  "READ_R",
  "READ_C",
  "READ_S",
  "HALT"
};

#if DEBUG_VM
constexpr bool DEBUG_PRINT_OP = true;
#else
constexpr bool DEBUG_PRINT_OP = false;
#endif

class VM {
public:
private:
  std::vector<uint8_t> code;
  mutable std::vector<uint8_t> stack; // Runtime Stack
  mutable size_t pc = 0;
  mutable size_t fp = 0; // Frame pointer
  std::vector<size_t> jmp_locs;

  template <VarType T> inline void add_value(T v) {
    if constexpr (sizeof(T) == 1) {
      code.push_back(static_cast<uint8_t>(v));
    } else {
      const uint8_t *p = reinterpret_cast<const uint8_t *>(&v);
      code.insert(code.end(), p, p + sizeof(T));
    }
  }

  OPCODE fetch_op() const {
    return static_cast<OPCODE>(code[pc++]);
  };

  template <VarType T>
  T fetch_code() const {
    T res;
    std::memcpy(&res, &code[pc], sizeof(T));
    pc += sizeof(T);
    return res;
  }

  inline size_t fetch_addr() const {
    return fetch_code<size_t>();
  }

  template <typename T> T fetch_data() const {
    T res;
    std::memcpy(&res, &stack[stack.size() - sizeof(T)], sizeof(T));
    stack.resize(stack.size() - sizeof(T));
    return res;
  }

  template <typename T> inline void add_var(T v) const {
    if constexpr (sizeof(T) == 1) {
      stack.push_back(static_cast<uint8_t>(v));
    } else {
      const uint8_t *p = reinterpret_cast<const uint8_t *>(&v);
      stack.insert(stack.end(), p, p + sizeof(T));
    }
  }

public:
  VM() {}

  size_t get_current_location() const { return code.size(); }
  std::vector<uint8_t> &data() const { return stack; }

  void run() const;

  void dump_state() const;

  /// @brief Adds string length + string data to code
  void add_string(const std::string& s) {
    add_value(uint32_t(s.length()));
    code.insert(code.end(), s.begin(), s.end());
  }

  template <VarType T> bool add_push(OPCODE op, T imm) {
    switch (op) {
      case OPCODE::PUSH_B:
      case OPCODE::PUSH_Q:
        add_value(static_cast<uint8_t>(op));
        add_value(imm);
        return true;
      default: 
        return false;
    }
  }

  void add_push_frame_pointer() {
    add_value(static_cast<uint8_t>(OPCODE::PUSH_FP));
  }

  void add_push_string(size_t loc) {
    add_value(static_cast<uint8_t>(OPCODE::PUSH_S));
    add_value(loc);
    // std::string s;
    
    // uint32_t len;
    // std::memcpy(&len, &code[loc], sizeof(len));
    // loc += sizeof(len);
    
    // s.reserve(len);
    // const size_t end = loc + len;
    // for(;loc < end;++loc) {
    //   s.push_back(code[loc]);
    // }
  }

  void add_not_op() {
    add_value(static_cast<uint8_t>(OPCODE::NOT));
  }

  bool add_arithmetic_op(OPCODE op) {
    switch (op) {
    case OPCODE::ADD_I:
    case OPCODE::SUB_I:
    case OPCODE::MUL_I:
    case OPCODE::DIV_I:
    case OPCODE::ADD_R:
    case OPCODE::SUB_R:
    case OPCODE::MUL_R:
    case OPCODE::DIV_R:
    case OPCODE::ADD_C:
    case OPCODE::SUB_C:
    case OPCODE::MUL_C:
    case OPCODE::DIV_C:
      add_value(static_cast<uint8_t>(op));
      return true;
    default:
      return false;
    }
  }

  void add_string_op() {
    add_value(static_cast<uint8_t>(OPCODE::ADD_S));
  }

  bool add_logic_op(OPCODE op) {
    switch (op) {
      case OPCODE::AND:
      case OPCODE::OR:
        add_value(static_cast<uint8_t>(op));
        return true;
      default:
        return false;
    }
  }

  bool add_cmp(OPCODE op) {
    switch (op) {
      case OPCODE::CMP_C:
      case OPCODE::CMP_I:
      case OPCODE::CMP_R:
      case OPCODE::CMP_S:
        add_value(static_cast<uint8_t>(op));
        return true;
      default:
        return false;
    }
  }

  bool add_flag(OPCODE op) {
    switch(op) {
      case OPCODE::LE:
      case OPCODE::LT:
      case OPCODE::EQ:
      case OPCODE::NE:
      case OPCODE::GE:
      case OPCODE::GT:
        add_value(static_cast<uint8_t>(op));
        return true;
      default:
        return false;
    }
  }

  size_t add_jmp(OPCODE op, size_t loc) {
    switch (op) {
      case OPCODE::JMP:
      case OPCODE::JMP_TRUE:
      case OPCODE::JMP_FALSE:{
        add_value(static_cast<uint8_t>(op));
        const auto res = code.size();
        jmp_locs.push_back(res);
        add_value(loc);
        return res;
      }
      default:
        return -1;
    }
  }

  bool conf_jmp(size_t loc, size_t dest) {
    if((loc + sizeof(loc) > code.size()) ||
      !std::binary_search(jmp_locs.begin(), jmp_locs.end(), loc)) 
      return false;
    std::memcpy(&code[loc], &dest, sizeof(dest));
    return true;
  }

  void add_resize_stack(int32_t diff) {
    add_value(static_cast<uint8_t>(OPCODE::MODSTK));
    add_value(diff);
  }

  bool add_move(OPCODE op) {
    switch (op) {
      case OPCODE::STORE_B:
      case OPCODE::STORE_Q:
      case OPCODE::STORE_S:
        add_value(static_cast<uint8_t>(op));
        return true;
      default:
        return false;
    }
  }

  bool add_load(OPCODE op) {
    switch (op) {
    case OPCODE::LOAD_B:
    case OPCODE::LOAD_Q:
    case OPCODE::LOAD_S:
      add_value(static_cast<uint8_t>(op));
      return true;
    default:
      return false;
    }
  }

  bool add_pop(OPCODE op) {
    switch (op) {
    case OPCODE::POP_B:
    case OPCODE::POP_Q:
    case OPCODE::POP_S:
      add_value(static_cast<uint8_t>(op));
      return true;
    default:
      return false;
    }
  }

  bool add_read(OPCODE op) {
    switch (op) {
    case OPCODE::READ_I:
    case OPCODE::READ_R:
    case OPCODE::READ_C:
    case OPCODE::READ_S:
      add_value(static_cast<uint8_t>(op));
      return true;
    default:
      return false;
    }
  }

  bool add_write(OPCODE op) {
    switch (op) {
    case OPCODE::WRITE_I:
    case OPCODE::WRITE_R:
    case OPCODE::WRITE_C:
    case OPCODE::WRITE_S:
      add_value(static_cast<uint8_t>(op));
      return true;
    default:
      return false;
    }
  }

  void add_halt() {
    add_value(static_cast<uint8_t>(OPCODE::HALT));
  }
};

}