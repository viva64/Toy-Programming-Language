#pragma once
#include "parser/token.hpp"
#include "parser/ast/base.hpp"

namespace toy::ast
{
  class decl : public node, public typed
  {
  public:
    decl() = delete;
    ~decl() = default;

    decl(const decl&) = delete;
    decl& operator=(const decl&) = delete;
    decl(decl&&) = delete;
    decl& operator=(decl&&) = delete;

    decl(node_kind kind, const token& name, types::type t) noexcept;

  public:
    const token& name() const noexcept;

  private:
    token m_name;
  };


  class var_decl final : public decl
  {
  public:
    var_decl() = delete;
    ~var_decl() = default;

    var_decl(const var_decl&) = delete;
    var_decl& operator=(const var_decl&) = delete;
    var_decl(var_decl&&) = delete;
    var_decl& operator=(var_decl&&) = delete;

    var_decl(const token& name, node& init, types::type t) noexcept;

  public:
    const node& initialiser() const noexcept;

  private:
    node* m_init;
  };


  class param_decl : public decl
  {
  public:
    param_decl() = delete;
    ~param_decl() = default;

    param_decl(const param_decl&) = delete;
    param_decl& operator=(const param_decl&) = delete;
    param_decl(param_decl&&) = delete;
    param_decl& operator=(param_decl&&) = delete;

    param_decl(const token& name, types::type t) noexcept;
  };


  class func_decl : public decl
  {
  public:
    func_decl() = delete;
    ~func_decl() = default;

    func_decl(const func_decl&) = delete;
    func_decl& operator=(const func_decl&) = delete;
    func_decl(func_decl&&) = delete;
    func_decl& operator=(func_decl&&) = delete;

    explicit func_decl(const token& name) noexcept;

  public:
    void attach_params(node& params) noexcept;
    void attach_body(node& body) noexcept;

    const node& params() const noexcept;

    const node& body() const noexcept;

    types::type return_type() const noexcept;

    void specify_ret(types::type t) noexcept;

  private:
    node* m_params{};
    node* m_body{};
    types::type m_retType{};
  };
}