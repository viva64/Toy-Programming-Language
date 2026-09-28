#pragma once
#include "parser/ast/base.hpp"
#include "parser/ast/expr.hpp"
#include "parser/ast/decl.hpp"
#include "parser/ast/stmt.hpp"

namespace toy::ast
{
  template <typename T>
  concept ast_node = std::derived_from<T, node>;
}