#pragma once
#include "parser/token.hpp"
#include "parser/ast/base.hpp"

namespace toy::ast
{
  class ret_stmt : public node
  {
  public:
    ret_stmt() = delete;
    ~ret_stmt() = default;

    ret_stmt(const ret_stmt&) = delete;
    ret_stmt& operator=(const ret_stmt&) = delete;
    ret_stmt(ret_stmt&&) = delete;
    ret_stmt& operator=(ret_stmt&&) = delete;

    explicit ret_stmt(node* retExpr) noexcept;

  public:
    const node* ret_expr() const noexcept;

  private:
    node* m_retExpr{};
  };


  class if_stmt : public node
  {
  public:
    if_stmt() = delete;
    ~if_stmt() = default;

    if_stmt(const if_stmt&) = delete;
    if_stmt& operator=(const if_stmt&) = delete;
    if_stmt(if_stmt&&) = delete;
    if_stmt& operator=(if_stmt&&) = delete;

    if_stmt(node& cond, node& ifBody, node* elseBody) noexcept;

  public:
    const node& condition() const noexcept;

    const node& if_body() const noexcept;

    const node* else_body() const noexcept;

  private:
    node* m_cond;
    node* m_ifBody;
    node* m_elseBody;
  };


  class while_stmt : public node
  {
  public:
    while_stmt() = delete;
    ~while_stmt() = default;

    while_stmt(const while_stmt&) = delete;
    while_stmt& operator=(const while_stmt&) = delete;
    while_stmt(while_stmt&&) = delete;
    while_stmt& operator=(while_stmt&&) = delete;

    while_stmt(node& cond, node& body) noexcept;

  public:
    const node& condition() const noexcept;

    const node& body() const noexcept;

  private:
    node* m_cond;
    node* m_body;
  };
}