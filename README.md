# Pascal Compiler & Virtual Machine

A modern C++20 implementation that compiles Pascal source code to bytecode and executes it via a stack-based virtual machine.

## Project Overview

This project implements a complete Pascal compiler and stack-based virtual machine. The compiler translates Pascal source code into custom bytecode, which is then executed by a custom VM. The implementation follows modern C++20 practices with a focus on separation of concerns, and educational value.

**Current Version:** 0.1.0  
**License:** MIT License (see LICENSE.txt)

## Design Philosophy

- **Modular Architecture**: Compiler pipeline divided into distinct phases (Lexing, Parsing, Semantics, Code Generation)
- **Type Safety**: Comprehensive type checking and semantic analysis before code generation
- **Modern C++20**: Leveraging latest C++ features for cleaner, safer code
- **Educational Focus**: Clear structure suitable for learning compiler design and implementation
- **Testing**: Comprehensive unit tests for both compiler and VM components

## Features

### Language Features

#### Basic Types
- **Int**: Integer type for arithmetic operations
- **Real**: Floating-point numbers for precise calculations
- **Char**: Single character values
- **Bool**: Boolean values (true/false)
- **String**: String literals for text processing

#### Complex Types
- **Arrays**: Multi-dimensional arrays with flexible indexing types
- **Records**: Structured data types with named fields
- **Subranges**: Bounded ranges based on ordinal types
- **Enumerations**: User-defined ordinal types with named values

#### Control Flow Statements
- **if/else**: Conditional execution
- **case statements**: Multi-way branching with constant labels
- **while loops**: Pre-condition iteration
- **repeat/until loops**: Post-condition iteration
- **for loops**: Counted iteration (with `to` and `downto`)
- **goto with labels**: Unconditional jumps for advanced control flow

#### Functions and Procedures
- Support for both functions (returning values) and procedures
- By-value and by-reference (var) parameters (by values only at the moment)
- Nested function definitions
- Forward declarations

#### Built-in I/O Operations
- **read()**: Input for variables of various types
- **write()**: Output for expressions and literals

#### Program Structure
- Program header with identifier
- Declaration blocks: labels, constants, types, variables
- Function and procedure definitions
- Main statement block

### Implementation Features

- **Modern C++20 compilation** with CMake build system
- **Comprehensive type checking** with detailed error messages
- **Symbol table management** for scoping and name resolution
- **Visitor pattern** for AST traversal and code generation
- **Bytecode generation** from AST
- **Stack-based virtual machine** with 45+ opcodes
- **Unit testing** with GoogleTest framework
- **Error reporting** with line and column information

## Project Structure

```
pascal/
├── compiler/
│   ├── src/
│   │   ├── Lexer.cpp/hpp      # Tokenization and token definitions
│   │   ├── Parser.cpp/hpp     # Recursive descent parser
│   │   ├── Ast.cpp/hpp        # Abstract syntax tree nodes with validation
│   │   ├── Semantics.cpp/hpp  # Type checking and symbol management
│   │   ├── Generator.cpp/hpp  # Bytecode code generation
│   │   ├── ValidationUtils.hpp
│   │   ├── Visitor.hpp        # Visitor pattern interfaces
│   │   └── Main.cpp           # Compiler CLI entry point
│   └── tests/                 # Compiler unit tests
├── vm/
│   ├── src/
│   │   ├── vm.cpp/hpp         # Virtual machine implementation
│   │   └── Main.cpp           # VM CLI entry point
│   └── tests/                 # VM unit tests
├── tests/                     # Example Pascal programs
│   ├── hello.pas              # User input with record types
│   ├── calc.pas               # Interactive calculator
│   ├── fib.pas                # Recursive Fibonacci
│   └── adv_calc.pas           # Expression parser
├── CMakeLists.txt             # Build configuration
├── gram.txt                   # Pascal grammar reference
└── LICENSE.txt                # MIT license
```

**Component Descriptions:**

- **compiler/src/Lexer**: Tokenizes source code into lexical tokens with position tracking
- **compiler/src/Parser**: Recursive descent parser that builds Abstract Syntax Tree from tokens
- **compiler/src/Ast**: Defines all AST node types representing Pascal expressions/statements
- **compiler/src/Semantics**: Performs type checking, symbol table construction, and semantic validation
- **compiler/src/Generator**: Transforms AST into bytecode instructions using visitor pattern
- **compiler/src/Visitor**: Visitor interfaces for AST traversal and code generation
- **vm/src/vm**: Stack-based virtual machine that executes bytecode instructions
- **gram.txt**: Complete Pascal grammar documentation for reference
- **tests/**: Example Pascal programs demonstrating various language features

## Architecture

### Compiler Pipeline

```
┌─────────────────────────────────────────────────────────────┐
│                    Pascal Source Code                        │
│                     (program.pas)                            │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                     Lexer                                    │
│        • Character stream → Token stream                   │
│        • Token: type, value, position                      │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                    Parser                                   │
│        • Recursive descent parser                           │
│        • Tokens → Abstract Syntax Tree (AST)                │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                Semantics Analysis                           │
│        • Symbol table construction                         │
│        • Type checking and validation                       │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                   Code Generator                            │
│        • AST → Bytecode instructions                        │
│        • Instruction: opcode + immediate values             │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                  Bytecode File                              │
│                   (.bin format)                             │
└─────────────────────────────────────────────────────────────┘
```

### Virtual Machine Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Bytecode (.bin)                          │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                   Program Counter (PC)                       ││
│                   ───────►│  ◄─────────│
│                                                            ││
│                   Frame Pointer (FP)                       ││
│                   ───────►│  ◄─────────│
│                                                            ││
│         ┌────────────────────────────────┐                ││
│         │      Runtime Stack             │                ││
│         │  ┌──────────────────────────┐ │                ││
│         │  │  Return Address          │ │                ││
│         │  ├──────────────────────────┤ │                ││
│         │  │  Old Frame Pointer       │ │                ││
│         │  ├──────────────────────────┤ │                ││
│         │  │  Local Variables        │ │                ││
│         │  │  (function parameters)  │ │                ││
│         │  └──────────────────────────┘ │                ││
│         └────────────────────────────────┘                ││
│                                                            ││
│         ┌────────────────────────────────┐                ││
│         │   Opcode Decoder & Executor    │                ││
│         └────────────────────────────────┘                ││
└─────────────────────────────────────────────────────────────┘

Opcode Categories:
├── Stack Operations: PUSH_Q, PUSH_B, POP_Q, POP_B
├── Load/Store:    LOAD_Q, LOAD_B, STORE_Q, STORE_B
├── Arithmetic:    ADD_I, MUL_I, DIV_I, ADD_R, MUL_R, DIV_R
├── Comparison:    CMP_I, CMP_R, CMP_C, LE, LT, EQ, NE, GT, GE
├── Logic:         AND, OR, NOT
├── Control:       JMP, JMP_TRUE, JMP_FALSE, CALL, RET
├── I/O:           READ_I, WRITE_I, WRITE_CONST_S
└── Utility:       HALT, DUPL_Q, DUPL_B, MODSTK
```

### Key Design Patterns

- **Visitor Pattern**: Used for AST traversal across Expression, Statement, and Selector(field for records and element for arrays) visitors
- **Recursive Descent Parser**: Grammar rules directly mapped to parsing methods
- **RAII**: Extensive use of unique_ptr/shared_ptr for resource management (with const PTR* for non-owning processes)
- **Single Responsibility**: Each component (Lexer/Parser/Semantics/Generator) handles a specific phase

### Bytecode Format

- Binary file format with 8-byte size header
- Sequential instruction encoding without alignment
- Opcode format:
  - Opcodes: 1 byte
  - Immediate values: 1 or 8 bytes appended inline
- String literals: Embedded directly with null termination

## Building the Project

### Prerequisites

- **CMake**: Version 3.10.0 or higher
- **C++ Compiler**: GCC 10+, Clang 12+, or MSVC 2019+ with C++20 support
- **Git**: For cloning the repository

### Build Steps

```bash
# Clone the repository
git clone <repository-url>
cd pascal

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build .
```

### Available Build Targets

- **compiler**: Main compiler executable for compiling Pascal source files
- **vm**: Virtual machine executable for running bytecode files
- **unit_tests_compiler**: Compiler unit tests (GoogleTest based)
- **unit_tests_vm**: VM unit tests (GoogleTest based)

## Usage Guide

### Compiling Pascal Programs

- **Basic compilation**: 
  ```bash
  ./compiler program.pas
  ```
  Creates `program.bin` by default

- **Custom output file**:
  ```bash
  ./compiler program.pas -o output.bin
  ```

- **Error reporting**: Compiler provides detailed errors with line and column information

### Running Compiled Programs

- **Execute bytecode**:
  ```bash
  ./vm program.bin
  ```

- **File naming**: VM accepts filename with or without `.bin` extension

### Complete Workflow Example

1. **Write Pascal code** in `my_program.pas`
2. **Compile to bytecode**:
   ```bash
   ./compiler my_program.pas
   ```
3. **Run the compiled program**:
   ```bash
   ./vm my_program.bin
   ```
4. **Troubleshooting**: Check compiler output for syntax/semantic errors

### Command Line Interface

- **Compiler**:
  - Expects 1 or 3 arguments: source file (with optional `-o output_file`)
  - Provides usage hints on error
  - Auto-adds `.pas` extension if not provided

- **VM**:
  - Expects exactly 1 argument: bytecode file
  - Auto-adds `.bin` extension if not provided

## Sample Programs

### hello.pas (17 lines)

Demonstrates string input via `read()`, record types with fields, and validation loops with `repeat`/`until`.

**Features**:
- Program structure with `const` declarations
- `var` with `record` containing `array` and fields
- User I/O with prompts and data reading
- Input validation with boolean logic

**Run**:
```bash
./compiler tests/hello.pas
./vm tests/hello.bin
```

### calc.pas (35 lines)

Interactive calculator supporting arithmetic operations: +, -, *, /.

**Features**:
- Function definitions with boolean returns
- `while` loops for user interaction
- `case` statements for operation selection
- Boolean logic with logical operators

**Run**:
```bash
./compiler tests/calc.pas
./vm tests/calc.bin
```

### fib.pas (18 lines)

Recursive Fibonacci function accepting input from 0-20.

**Features**:
- Function recursion
- Integer validation with range checks
- Function calls within expressions
- Mathematical computation

**Run**:
```bash
./compiler tests/fib.pas
./vm tests/fib.bin
```

### adv_calc.pas (122 lines)

Mathematical expression parser supporting integers, parentheses, and operator precedence.

**Features**:
- Arrays for character-to-integer mapping (Should support type-conversion sometime in the future)
- Forward declarations for functions
- Nested functions and procedures
- Comprehensive expression evaluation with precedence
- Lexing and parsing of mathematical expressions
- Infix-to-postfix conversion implementation

**Run**:
```bash
./compiler tests/adv_calc.pas
./vm tests/adv_calc.bin
```

Each example demonstrates specific language constructs and patterns for Pascal programming.

## Testing

### Running Test Suite

- **Compiler tests only**:
  ```bash
  ./unit_tests_compiler
  ```

- **VM tests only**:
  ```bash
  ./unit_tests_vm
  ```

### Test Coverage

#### Compiler Tests

- **Lexer**: Token generation and error handling, identifier recognition, literal parsing
- **Parser**: AST construction validation, grammar rule compliance, error detection
- **Semantics**: Type checking and scope management, symbol table construction, error detection

#### VM Tests

- **Opcode execution**: Arithmetic, logic, comparison, stack operations
- **Stack management**: Push, pop, load, store operations
- **Function call/return**: Frame pointer handling, parameter passing
- **I/O operations**: Input/output for various types

## Language Reference

### Grammar Overview

**Program Structure**:
```
program := PROGRAM ID ';' block '.'
```

**Declaration Sections**:
- Labels: `LABEL (INT | ID) { ',' (INT | ID) } ';'`
- Constants: `CONST NAME '=' constant ';' { NAME '=' constant ';' }`
- Types: `TYPE NAME '=' type ';' { NAME '=' type ';' }`
- Variables: `VAR id_list ':' type ';' { id_list ':' type ';' }`

**Statement Types**:
- Simple: assignment, procedure call, goto
- Structured: compound, conditional, repetitive
- Compound: `BEGIN statement_sequence END`

**Expression Syntax**:
- Relational: `simple_expression [ relational_operator simple_expression ]`
- Simple: `[ '+' | '-' ] term { addition_operator term }`
- Terms: `factor { multiplication_operator factor }`
- Factors: literals, variables, function calls, parenthesized expressions, unary NOT

**Type Categories**:
- Basic types: integer types, char, boolean, real
- Structured types: arrays, records
- Subranges: `constant '..' constant`
- Enumerations: `'(' id_list ')'`

### Supported Opcodes

#### Arithmetic
- **Integer**: `ADD_I`, `SUB_I`, `MUL_I`, `DIV_I`
- **Real**: `ADD_R`, `SUB_R`, `MUL_R`, `DIV_R`
- **Character**: `ADD_C`, `SUB_C`, `MUL_C`, `DIV_C`

#### Stack Manipulation
- `PUSH_Q`, `PUSH_B`, `PUSH_FP`, `POP_Q`, `POP_B`
- `DUPL_Q`, `DUPL_B`, `MODSTK`

#### Memory Operations
- `LOAD_Q`, `LOAD_B`, `STORE_Q`, `STORE_B`

#### Comparison
- `CMP_I`, `CMP_R`, `CMP_C`, `LE`, `LT`, `EQ`, `NE`, `GT`, `GE`

#### Logical Operations
- `AND`, `OR`, `NOT`

#### Control Flow
- `JMP`, `JMP_TRUE`, `JMP_FALSE`, `CALL`, `RET`

#### I/O Operations
- `READ_I`, `READ_R`, `READ_C`, `READ_B`, `READ_S`
- `WRITE_I`, `WRITE_R`, `WRITE_C`, `WRITE_B`, `WRITE_S`, `WRITE_CONST_S`

#### Utility
- `C2I` (char to int conversion), `HALT`

## Troubleshooting

### Debugging Tips

- Enable debug output in VM:
  Edit `vm/src/CMakeLists.txt` and set `DEBUG_VM=1`
  Rebuild and run with verbose output

- Low-level bytecode inspection:
  Use debugger (gdb...) to examine bytecode execution flow
  Compare expected vs actual instruction sequences

- Validate parsing:
  Check `gram.txt` for correct grammar rules
  Add debug print statements in Parser for rule matching

## Development Notes

### Extending the Compiler

To add new language features:

1. **Tokens**: Add to `Lexer.hpp` in `TOKEN_TYPE` enum
2. **Grammar**: Implement parsing rules in `Parser.cpp`
3. **AST**: Create node definitions in `Ast.hpp/cpp`
4. **Semantics**: Add type checking in `Semantics.cpp`
5. **Generator**: Emit bytecode in `Generator.cpp`
6. **Visitor**: Implement visitor methods for AST node types

## License & Credits

**License**: MIT License (c) 2025 Anass Serroukh

Full license text available in LICENSE.txt file.

This project implements a comprehensive Pascal compiler and virtual machine designed for educational purposes and demonstrating compiler construction principles using modern C++20 practices.
