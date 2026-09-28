#include "parser/ast/expr.hpp"
#include "parser/ast/decl.hpp"

namespace toy::ast
{
  lit_expr::lit_expr(const token& val, types::type t) noexcept :
    expr{ node::LiteralExpr, t },
    m_value{ val }
  { }

  const token& lit_expr::value() const noexcept
  {
    return m_value;
  }


  id_expr::id_expr(decl& declaration) noexcept :
    expr{ node::IdExpr, declaration.type() },
    m_decl{ &declaration }
  {
    validate(m_decl);
  }

  const decl& id_expr::declaration() const noexcept
  {
    return *m_decl;
  }

  const token& id_expr::name() const noexcept
  {
    return m_decl->name();
  }


  paren_expr::paren_expr(node& internal, types::type t) noexcept :
    expr{ node::ParenExpr, t },
    m_expr{ &internal }
  {
    validate(m_expr);
  }

  const node& paren_expr::internal_expr() const noexcept
  {
    return *m_expr;
  }


  unary_expr::unary_expr(node& operand, eval::operation op, types::type t) noexcept :
    expr{ node::UnaryExpr, t },
    m_operand{ &operand },
    m_op{ op }
  {
    validate(m_operand);
  }

  const node& unary_expr::operand() const noexcept
  {
    return *m_operand;
  }

  eval::operation unary_expr::op() const noexcept
  {
    return m_op;
  }


  binary_expr::binary_expr(node& lhs, node& rhs, eval::operation op, types::type t) noexcept :
    expr{ node::BinaryExpr, t },
    m_lhs{ &lhs },
    m_rhs{ &rhs },
    m_op{ op }
  {
    validate(m_lhs);
    validate(m_rhs);
  }

  const node& binary_expr::left() const noexcept
  {
    return *m_lhs;
  }
  const node& binary_expr::right() const noexcept
  {
    return *m_rhs;
  }

  eval::operation binary_expr::op() const noexcept
  {
    return m_op;
  }


  assign_expr::assign_expr(node& assignee, node& init, eval::operation op, types::type t) noexcept :
    expr{ node::AssignExpr, t },
    m_assignee{ &assignee },
    m_init{ &init },
    m_op{ op }
  {
    validate(m_assignee);
    validate(m_init);
  }

  const node& assign_expr::assignee() const noexcept
  {
    return *m_assignee;
  }
  const node& assign_expr::init() const noexcept
  {
    return *m_init;
  }

  eval::operation assign_expr::op() const noexcept
  {
    return m_op;
  }


  call_expr::call_expr(node& callee, node& args, types::type t) noexcept :
    expr{ node::CallExpr, t },
    m_callee{ &callee },
    m_args{ &args }
  {
    validate(m_callee);
    validate(m_args);
  }

  const node& call_expr::callee() const noexcept
  {
    return *m_callee;
  }
  const node& call_expr::args() const noexcept
  {
    return *m_args;
  }


  impl_cast::impl_cast(node& op, types::type t) noexcept :
    expr{ node::ImplicitCast, t },
    m_operand{ &op }
  {
    validate(m_operand);
  }

  const node& impl_cast::operand() const noexcept
  {
    return *m_operand;
  }
}