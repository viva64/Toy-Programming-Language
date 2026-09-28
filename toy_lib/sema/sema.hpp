#pragma once
#include "sema/symtab.hpp"
#include "sema/types.hpp"
#include "parser/ast/ast.hpp"

namespace toy
{
  class token;
}

namespace toy
{
  struct unary_cast
  {
    types::type m_exprType{};
    types::type m_opType{};
    types::type m_opToType{};
  };

  struct binary_cast
  {
    types::type m_exprType{};
    types::type m_lhsType{};
    types::type m_lhsToType{};
    types::type m_rhsType{};
    types::type m_rhsToType{};
  };

  class sema final
  {
  public:
    sema();
    ~sema() = default;

    sema(const sema&) = delete;
    sema& operator=(const sema&) = delete;
    sema(sema&&) = delete;
    sema& operator=(sema&&) = delete;

  public:
    static bool check_type_compatibility(types::type t1, types::type t2) noexcept;

    static types::type extract_type(const ast::node& node) noexcept;

    static types::type literal_type(const token& lit) noexcept;

    static types::type keyword_type(const token& lit) noexcept;

    static unary_cast unary_type(const ast::node& operand, eval::operation op) noexcept;

    static binary_cast binary_type(const ast::node& lhs, const ast::node& rhs, eval::operation op) noexcept;

    static const ast::func_decl* extract_function(const ast::node& node);

    static const ast::decl* extract_decl(const ast::node& node);

    void enter_new_scope();

    void exit_scope();

    void record(ast::decl& decl);

    ast::decl* lookup(symtab::name_type name);

    ast::decl* scoped_lookup(symtab::name_type name);

    void begin_function(ast::func_decl& fd);

    void end_function();

    unary_cast handle_return(const ast::node* retExpr);

  private:
    symtab m_symtab;
    symtab::scope* m_global{};
    symtab::scope* m_cur{};
    ast::func_decl* m_curFn{};
  };
}