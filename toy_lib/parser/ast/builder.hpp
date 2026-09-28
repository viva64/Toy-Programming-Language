#pragma once
#include "parser/ast/ast.hpp"
#include "sema/sema.hpp"

namespace toy::ast
{
  class builder final
  {
  public:
    using node_ptr = std::unique_ptr<node>;
    using node_storage = std::forward_list<node_ptr>;
    using node_container = std::vector<node*>;
    using error_msg = error::msg_type;

  public:
    builder() = default;
    ~builder() = default;

    builder(const builder&) = delete;
    builder& operator=(const builder&) = delete;
    builder(builder&&) = delete;
    builder& operator=(builder&&) = delete;

  private:
    template <ast_node T, typename ...Args> requires std::constructible_from<T, Args...>
    void make(Args&& ...args);

  public:
    const node* root() const noexcept;

    const node* last() const noexcept;

    void clear_state() noexcept;

    void make_paren();
    void make_literal(const token& value);
    void make_id(decl& d);
    void make_unary(eval::operation op);
    void make_binary(eval::operation op);
    void make_assign(eval::operation op);
    void make_call();

    decl* make_var(const token& name);
    decl* make_param(const token& type, const token& name);
    func_decl* prepare_func(const token& name);
    void attach_params(func_decl& f) noexcept;
    void complete_func(func_decl& f) noexcept;

    void make_ret(const token& ret, sema& s);
    void make_empty_ret(const token& ret, sema& s);

    void make_if(bool hasElse);
    void make_while();

    void make_list(list::size_type count);

    void make_error(const token& at, error_msg message);

  private:
    void wrap_in_cast(node& op, types::type t);

    node* extract() noexcept;

    node* peek_last() noexcept;

  private:
    node_storage m_nodes;
    node_container m_state;
    node* m_root{};
  };
}