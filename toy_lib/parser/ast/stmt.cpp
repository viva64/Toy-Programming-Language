#include "parser/ast/stmt.hpp"

namespace toy::ast
{
  ret_stmt::ret_stmt(node* retExpr) noexcept :
    node{ node::RetStmt },
    m_retExpr{ retExpr }
  {
    validate(m_retExpr);
  }

  const node* ret_stmt::ret_expr() const noexcept
  {
    return m_retExpr;
  }


  if_stmt::if_stmt(node& cond, node& ifBody, node* elseBody) noexcept :
    node{ node::IfStmt },
    m_cond{ &cond },
    m_ifBody{ &ifBody },
    m_elseBody{ elseBody }
  {
    validate(m_cond);
    validate(m_ifBody);
    validate(m_elseBody);
  }

  const node& if_stmt::condition() const noexcept
  {
    return *m_cond;
  }

  const node& if_stmt::if_body() const noexcept
  {
    return *m_ifBody;
  }
  const node* if_stmt::else_body() const noexcept
  {
    return m_elseBody;
  }


  while_stmt::while_stmt(node& cond, node& body) noexcept :
    node{ node::WhileStmt },
    m_cond{ &cond },
    m_body{ &body }
  {
    validate(m_cond);
    validate(m_body);
  }

  const node& while_stmt::condition() const noexcept
  {
    return *m_cond;
  }

  const node& while_stmt::body() const noexcept
  {
    return *m_body;
  }
}