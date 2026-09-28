#pragma once
#include "parser/token.hpp"
#include "parser/ast/base.hpp"
#include "eval/value.hpp"

namespace toy::ast
{
  class decl;

  class expr : public node, public typed
  {
  public:
    expr() = delete;
    ~expr() = default;

    expr(const expr&) = delete;
    expr& operator=(const expr&) = delete;
    expr(expr&&) = delete;
    expr& operator=(expr&&) = delete;

    expr(node_kind kind, types::type t) noexcept :
      node{ kind },
      typed{ t }
    {
      if (t == types::type::Error)
        make_invalid();
    }
  };


  class lit_expr : public expr
  {
  public:
    lit_expr() = delete;
    ~lit_expr() = default;

    lit_expr(const lit_expr&) = delete;
    lit_expr& operator=(const lit_expr&) = delete;
    lit_expr(lit_expr&&) = delete;
    lit_expr& operator=(lit_expr&&) = delete;

    lit_expr(const token& val, types::type t) noexcept;

  public:
    const token& value() const noexcept;

  private:
    token m_value;
  };


  class id_expr : public expr
  {
  public:
    id_expr() = delete;
    ~id_expr() = default;

    id_expr(const lit_expr&) = delete;
    id_expr& operator=(const id_expr&) = delete;
    id_expr(id_expr&&) = delete;
    id_expr& operator=(id_expr&&) = delete;

    id_expr(decl& declaration) noexcept;

  public:
    const decl& declaration() const noexcept;

    const token& name() const noexcept;

  private:
    decl* m_decl;
  };


  class paren_expr : public expr
  {
  public:
    paren_expr() = delete;
    ~paren_expr() = default;

    paren_expr(const paren_expr&) = delete;
    paren_expr& operator=(const paren_expr&) = delete;
    paren_expr(paren_expr&&) = delete;
    paren_expr& operator=(paren_expr&&) = delete;

    paren_expr(node& internal, types::type t) noexcept;

  public:
    const node& internal_expr() const noexcept;

  private:
    node* m_expr;
  };

  class unary_expr : public expr
  {
  public:
    unary_expr() = delete;
    ~unary_expr() = default;

    unary_expr(const unary_expr&) = delete;
    unary_expr& operator=(const unary_expr&) = delete;
    unary_expr(unary_expr&&) = delete;
    unary_expr& operator=(unary_expr&&) = delete;

    unary_expr(node& operand, eval::operation op, types::type t) noexcept;

  public:
    const node& operand() const noexcept;

    eval::operation op() const noexcept;

  private:
    node* m_operand;
    eval::operation m_op;
  };


  class binary_expr : public expr
  {
  public:
    binary_expr() = delete;
    ~binary_expr() = default;

    binary_expr(const binary_expr&) = delete;
    binary_expr& operator=(const binary_expr&) = delete;
    binary_expr(binary_expr&&) = delete;
    binary_expr& operator=(binary_expr&&) = delete;

    binary_expr(node& lhs, node& rhs, eval::operation op, types::type t) noexcept;

  public:
    const node& left() const noexcept;
    const node& right() const noexcept;

    eval::operation op() const noexcept;

  private:
    node* m_lhs;
    node* m_rhs;
    eval::operation m_op;
  };


  class assign_expr : public expr
  {
  public:
    assign_expr() = delete;
    ~assign_expr() = default;

    assign_expr(const assign_expr&) = delete;
    assign_expr& operator=(const assign_expr&) = delete;
    assign_expr(assign_expr&&) = delete;
    assign_expr& operator=(assign_expr&&) = delete;

    assign_expr(node& assignee, node& init, eval::operation op, types::type t) noexcept;

  public:
    const node& assignee() const noexcept;
    const node& init() const noexcept;

    eval::operation op() const noexcept;

  private:
    node* m_assignee;
    node* m_init;
    eval::operation m_op;
  };


  class call_expr : public expr
  {
  public:
    call_expr() = delete;
    ~call_expr() = default;

    call_expr(const call_expr&) = delete;
    call_expr& operator=(const call_expr&) = delete;
    call_expr(call_expr&&) = delete;
    call_expr& operator=(call_expr&&) = delete;

    call_expr(node& callee, node& args, types::type t) noexcept;

  public:
    const node& callee() const noexcept;
    const node& args() const noexcept;

  private:
    node* m_callee;
    node* m_args;
  };


  class impl_cast : public expr
  {
  public:
    impl_cast() = delete;
    ~impl_cast() = default;

    impl_cast(const impl_cast&) = delete;
    impl_cast& operator=(const impl_cast&) = delete;
    impl_cast(impl_cast&&) = delete;
    impl_cast& operator=(impl_cast&&) = delete;

    impl_cast(node& op, types::type t) noexcept;

  public:
    const node& operand() const noexcept;

  private:
    node* m_operand;
  };
}