#include "parser/ast/decl.hpp"

namespace toy::ast
{
  decl::decl(node_kind kind, const token& name, types::type t) noexcept :
    node{ kind },
    typed{ t },
    m_name{ name }
  { }

  const token& decl::name() const noexcept
  {
    return m_name;
  }


  var_decl::var_decl(const token& name, node& init, types::type t) noexcept :
    decl{ node::VarDecl, name, t },
    m_init{ &init }
  {
    validate(m_init);
  }

  const node& var_decl::initialiser() const noexcept
  {
    return *m_init;
  }


  param_decl::param_decl(const token& name, types::type t) noexcept :
    decl{ node::ParamDecl, name, t }
  { }


  func_decl::func_decl(const token& name) noexcept :
    decl{ node::FuncDecl, name, types::type::Function }
  { }

  void func_decl::attach_params(node& params) noexcept
  {
    m_params = &params;
    validate(m_params);
  }
  void func_decl::attach_body(node& body) noexcept
  {
    m_body = &body;
    validate(m_body);
  }

  const node& func_decl::params() const noexcept
  {
    return *m_params;
  }
  const node& func_decl::body() const noexcept
  {
    return *m_body;
  }

  types::type func_decl::return_type() const noexcept
  {
    return m_retType;
  }

  void func_decl::specify_ret(types::type t) noexcept
  {
    m_retType = t;
  }
}