#pragma once
#include "parser/token.hpp"
#include "sema/types.hpp"

namespace toy::ast
{
  enum class node_kind : std::uint8_t
  {
    Error,
    List,
    ImplicitCast,

    LiteralExpr,
    IdExpr,
    ParenExpr,
    UnaryExpr,
    BinaryExpr,
    CallExpr,
    AssignExpr,
    
    VarDecl,
    FuncDecl,
    ParamDecl,

    RetStmt,
    IfStmt,
    WhileStmt
  };

  class typed
  {
  public:
    typed() = delete;

    typed(const typed&) = delete;
    typed& operator=(const typed&) = delete;
    typed(typed&&) = delete;
    typed& operator=(typed&&) = delete;

    explicit typed(types::type t) noexcept :
      m_type{ t }
    { }

  public:
    auto type() const noexcept
    {
      return m_type;
    }

  private:
    types::type m_type{};
  };

  class node
  {
  public:
    using enum node_kind;
    friend class builder;

  public:
    node() = delete;
    virtual ~node() = default;

    node(const node&) = delete;
    node& operator=(const node&) = delete;
    node(node&&) = delete;
    node& operator=(node&&) = delete;

    explicit node(node_kind kind, bool valid = true) noexcept :
      m_kind{ kind },
      m_valid{ valid }
    { }

  protected:
    void validate(const node* child) noexcept
    {
      if (!child)
        return;

      if (!child->is_valid())
        make_invalid();
    }

    void make_invalid()
    {
      m_valid = false;
    }

  public:
    bool is_valid() const noexcept
    {
      return m_valid;
    }

    node_kind what() const noexcept
    {
      return m_kind;
    }

  private:
    node_kind m_kind;
    bool m_valid;
  };


  class list final : public node
  {
  public:
    using data_type = std::vector<node*>;
    using size_type = data_type::size_type;

  public:
    list() = delete;
    ~list() = default;

    list(const list&) = delete;
    list& operator=(const list&) = delete;
    list(list&&) = delete;
    list& operator=(list&&) = delete;

    explicit list(data_type items) :
      node{ node::List },
      m_data{ std::move(items) }
    {
      for (auto child : m_data)
        validate(child);
    }

  public:
    void attach(node& n)
    {
      m_data.push_back(&n);
    }

    const data_type& chidlren() const noexcept
    {
      return m_data;
    }

    data_type& chidlren() noexcept
    {
      return m_data;
    }

  private:
    data_type m_data;
  };


  class error : public node
  {
  public:
    using msg_type = std::string_view;

  public:
    error() = delete;
    ~error() = default;

    error(const error&) = delete;
    error& operator=(const error&) = delete;
    error(error&&) = delete;
    error& operator=(error&&) = delete;

    error(const token& tok, msg_type msg) noexcept :
      node{ node::Error, false },
      m_at{ tok },
      m_message{ msg }
    { }

  public:
    const token& at() const noexcept
    {
      return m_at;
    }

    msg_type message() const noexcept
    {
      return m_message;
    }

  private:
    token m_at;
    msg_type m_message;
  };
}