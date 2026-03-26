#include "Ast.hpp"
#include "Parser.hpp"
#include "Semantics.hpp"
#include "Visitor.hpp"
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <variant>

#include "../../vm/src/vm.hpp"

namespace pascal_compiler 
{

class Generator : CombinedVisitor
{

  struct Info{
    Int location;
    size_t size;
  };

  std::vector<std::unordered_map<std::string_view, Info>> m_var_info;                // variables
  std::vector<std::unordered_map<std::variant<const Const*, std::string_view>, Info>> m_const_info;        // strings
  std::vector<std::unordered_map<std::string_view, size_t>> m_func_locations;  // functions/procedures

  std::vector<std::unordered_map<const Label*, size_t>> m_label_locations;
  std::vector<std::unordered_map<size_t, const Label*>> m_goto_map;

  std::unordered_map<const Type*, size_t> m_type_sizes; // for variables only
  std::unordered_map<const Record*, std::unordered_map<std::string_view, size_t>> m_record_offsets;
  std::unordered_map<const Type*, size_t> m_type_lens; // for enums and subranges, saves the number of elements
  bool m_by_value = true; // if visiting VariableAccess should push the value or the reference (reference only in assignment) 

  pascal_vm::VM vm;

  void process_type(const Type*);

  CONST_CAT get_catagory(const Type* t)
  {
    const auto name = t->get_underlying_type()->id();
    if(name == CONST_CAT_NAMES[int(CONST_CAT::CC_INT)]) return CONST_CAT::CC_INT;
    if(name == CONST_CAT_NAMES[int(CONST_CAT::CC_REAL)]) return CONST_CAT::CC_REAL;
    if(name == CONST_CAT_NAMES[int(CONST_CAT::CC_CHAR)]) return CONST_CAT::CC_CHAR;
    if(name == CONST_CAT_NAMES[int(CONST_CAT::CC_BOOL)]) return CONST_CAT::CC_BOOL;
    if(name == CONST_CAT_NAMES[int(CONST_CAT::CC_ENUM)]) return CONST_CAT::CC_ENUM;
    return CONST_CAT::CC_CONST_STRING;
  }

  size_t get_type_size(const Type *); // For variables only
  void process_context(const Context&);
  void process_context(const Function&);

public:
  
  Generator(const Context& ctx) { process_context(ctx); }
  Generator(Parser &&parser) { 
    parser.parse();
    process_context(*parser.getCtx());
  }

  const auto& data() const { return vm.bytecode(); }

private:

  void visit(const LiteralExpression&, const Context&) override;
  void visit(const UnaryExpression&, const Context&) override;
  void visit(const BinaryExpression&, const Context&) override;
  void visit(const NExpression&, const Context&) override;
  void visit(const VariableAccess&, const Context&) override;   //Push address of the variable into the stack
  void visit(const ArraySelector&, const Context&) override; 
  void visit(const FieldSelector&, const Context&) override;
  void visit(const FunctionCall&, const Context&) override;

  void visit(const WriteStatement&, const Context&) override;
  void visit(const ReadStatement&, const Context&) override;

  void visit(const LabeledStatement&, const Context& ctx) override;
  void visit(const CompoundStatement&, const Context&) override;
  void visit(const AssignmentStatement&, const Context&) override;  
  void visit(const ProcedureCall&, const Context& ctx) override;
  void visit(const GotoStatement&, const Context& ctx) override;
  void visit(const WhileStatement&, const Context& ctx) override;
  void visit(const RepeatStatement&, const Context& ctx) override;
  void visit(const ForStatement&, const Context& ctx) override;
  void visit(const IfStatement&, const Context& ctx) override;
  void visit(const CaseStatement&, const Context& ctx) override;
};

}