#pragma once
#include <vector>
#include "../src/Parser.hpp"
#include "../src/Visitor.hpp"

using namespace pascal_compiler;

using ConstType = decltype(Const::m_val);

template <typename T>
bool checkLiteral(const Expression *expr, T expected, CONST_CAT cat)
{
  auto lit = dynamic_cast<const LiteralExpression *>(expr);
  if (!lit)
    return false;
  if (lit->value->m_cat != cat)
    return false;
  if (!std::holds_alternative<T>(lit->value->m_val))
    return false;
  return std::get<T>(lit->value->m_val) == expected;
}

static bool checkIntLiteral(const Expression *expr, Int expected)
{
  return checkLiteral(expr, expected, CONST_CAT::CC_INT);
}

static bool checkRealLiteral(const Expression *expr, Real expected)
{
  return checkLiteral(expr, expected, CONST_CAT::CC_REAL);
}

static bool checkStringLiteral(const Expression *expr, const std::string &expected)
{
  return checkLiteral(expr, expected, CONST_CAT::CC_STRING);
}

static bool checkCharLiteral(const Expression *expr, char expected)
{
  return checkLiteral(expr, expected, CONST_CAT::CC_CHAR);
}

static bool checkBoolLiteral(const Expression *expr, bool expected)
{
  return checkLiteral(expr, expected, CONST_CAT::CC_BOOL);
}

static bool checkVariableAccess(const Expression *expr, std::string_view varName)
{
  auto va = dynamic_cast<const VariableAccess *>(expr);
  if (!va)
    return false;
  return va->baseVar->m_name == varName;
}

// Check if expression is a FunctionCall and get function name
static bool checkFunctionCall(const Expression *expr, std::string_view funcName)
{
  auto fc = dynamic_cast<const FunctionCall *>(expr);
  if (!fc)
    return false;
  return fc->function->m_name == funcName;
}

// Check if expression is a FunctionCall and verify argument count
static bool checkFunctionCallWithArgs(const Expression *expr, std::string_view funcName, size_t expectedArgs)
{
  auto fc = dynamic_cast<const FunctionCall *>(expr);
  if (!fc)
    return false;
  if (fc->function->m_name != funcName)
    return false;
  return fc->args.size() == expectedArgs;
}

// Check if statement is a ProcedureCall and get procedure name
static bool checkProcedureCall(const Statement *stmt, std::string_view procName)
{
  auto pc = dynamic_cast<const ProcedureCall *>(stmt);
  if (!pc)
    return false;
  return pc->procedure->m_name == procName;
}