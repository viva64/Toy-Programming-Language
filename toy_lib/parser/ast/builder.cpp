#include "parser/ast/builder.hpp"

namespace toy::ast
{
  static token dummy{ "", token::Error };

  template <ast_node T, typename ...Args> 
    requires std::constructible_from<T, Args...>
  void builder::make(Args&& ...args)
  {
    auto newNode = m_nodes.emplace_front(std::make_unique<T>(std::forward<Args>(args)...)).get();
    m_state.push_back(newNode);
    m_root = newNode;
  }

  const node* builder::root() const noexcept
  {
    return m_root;
  }

  const node* builder::last() const noexcept
  {
    return m_state.empty() ? nullptr : m_state.back();
  }

  void builder::clear_state() noexcept
  {
    m_state.clear();
  }

  void builder::make_paren()
  {
    if (auto internal = extract())
    {
      make<paren_expr>(*internal, sema::extract_type(*internal));
    }
  }

  void builder::make_literal(const token& value)
  {
    make<lit_expr>(value, sema::literal_type(value));
  }

  void builder::make_id(decl& d)
  {
    make<id_expr>(d);
  }

  void builder::make_unary(eval::operation op)
  {
    auto operand = extract();
    if (!operand)
      return;

    const auto unaryType = sema::unary_type(*operand, op);
    if (unaryType.m_opType != unaryType.m_opToType)
    {
      wrap_in_cast(*operand, unaryType.m_opToType);
      operand = extract();
    }

    make<unary_expr>(*operand, op, unaryType.m_exprType);
  }

  void builder::make_binary(eval::operation op)
  {
    auto rhs = extract();
    auto lhs = extract();
    if (!lhs || !rhs)
      return;

    const auto binaryType = sema::binary_type(*lhs, *rhs, op);
    if (binaryType.m_lhsType != binaryType.m_lhsToType)
    {
      wrap_in_cast(*lhs, binaryType.m_lhsToType);
      lhs = extract();
    }
    if (binaryType.m_rhsType != binaryType.m_rhsToType)
    {
      wrap_in_cast(*rhs, binaryType.m_rhsToType);
      rhs = extract();
    }
    make<binary_expr>(*lhs, *rhs, op, binaryType.m_exprType);
  }

  void builder::make_assign(eval::operation op)
  {
    auto init = extract();
    auto assignee = extract();
    if (!assignee || !init)
      return;

    const auto initType = sema::extract_type(*init);
    const auto varType  = sema::extract_type(*assignee);
    bool err{};
    if (initType == types::type::Void)
    {
      make_error(dummy, "Cannot assign from void");
      err = true;
    }
    else if (initType == types::type::Function)
    {
      make_error(dummy, "Cannot assign from function");
      err = true;
    }
    else if (varType == types::type::Function)
    {
      make_error(dummy, "Cannot assign to function");
      err = true;
    }
    else if (initType != varType)
    {
      wrap_in_cast(*init, varType);
      init = extract();
    }

    make<assign_expr>(*assignee, *init, op, varType);
    if (err)
      make_list(2u);
  }

  void builder::make_call()
  {
    auto args = extract();
    auto callee = extract();
    if (!args || !callee)
      return;

    auto func = sema::extract_function(*callee);
    if (!func || !func->is_valid())
    {
      make_error(dummy, "Expected a function");
      make<call_expr>(*callee, *args, types::type::Error);
      make_list(2u);
      return;
    }

    auto&& argList = static_cast<list*>(args)->chidlren();
    auto&& paramList = static_cast<const list*>(&func->params())->chidlren();
    if (argList.size() != paramList.size())
    {
      make_error(dummy, "Wrong number of arguments in function call");
      make<call_expr>(*callee, *args, types::type::Error);
      make_list(2u);
      return;
    }

    for (auto paramIt = paramList.begin(); auto&& arg : argList)
    {
      auto param = *paramIt;
      const auto argT   = sema::extract_type(*arg);
      const auto paramT = sema::extract_type(*param);
      ++paramIt;
      if (argT == paramT)
        continue;

      wrap_in_cast(*arg, paramT);
      if (!sema::check_type_compatibility(argT, paramT))
      {
        make_error(dummy, "Incompatible types");
        make_list(2u);
        args->make_invalid();
      }
      arg = extract();
    }
    const auto retType = func->return_type();
    make<call_expr>(*callee, *args, retType);
  }

  decl* builder::make_var(const token& name)
  {
    auto init = peek_last();
    if (!init)
    {
      return {};
    }

    const auto initType = sema::extract_type(*init);
    bool err{};
    if (initType == types::type::Void)
    {
      make_error(dummy, "Cannot assign from void");
      err = true;
    }
    else if (initType == types::type::Function)
    {
      make_error(dummy, "Cannot assign from function");
      err = true;
    }

    if (err)
    {
      make_list(2u);
    }
    init = extract();
    make<var_decl>(name, *init, initType);
    auto var = static_cast<decl*>(m_root);
    return var;
  }

  decl* builder::make_param(const token& type, const token& name)
  {
    make<param_decl>(name, sema::keyword_type(type));
    return static_cast<decl*>(m_root);
  }

  func_decl* builder::prepare_func(const token& name)
  {
    make<func_decl>(name);
    return static_cast<func_decl*>(m_root);
  }
  void builder::attach_params(func_decl& f) noexcept
  {
    auto params = extract();
    if (!params)
      return;
    f.attach_params(*params);
  }
  void builder::complete_func(func_decl& f) noexcept
  {
    auto body = extract();
    if (!body)
      return;
    f.attach_body(*body);
  }

  void builder::make_ret(const token& ret, sema& s)
  {
    auto expr = peek_last();
    const auto castInfo = s.handle_return(expr);
    const auto retOk = expr && castInfo.m_exprType != types::type::Error;
    if (!retOk)
    {
      make_error(ret, "Incompatible return type");
      make_list(2u);
    }
    
    expr = extract();
    if (castInfo.m_opType != castInfo.m_opToType)
    {
      wrap_in_cast(*expr, castInfo.m_opToType);
      expr = extract();
    }
    make<ret_stmt>(expr);
  }
  void builder::make_empty_ret(const token& ret, sema& s)
  {
    const auto castInfo = s.handle_return(nullptr);
    if(castInfo.m_opType != castInfo.m_opToType)
    {
      make_error(ret, "Incompatible return type");
      auto expr = extract();
      make<ret_stmt>(expr);
      return;
    }
    make<ret_stmt>(nullptr);
  }

  void builder::make_if(bool hasElse)
  {
    auto elseBody = hasElse ? extract() : nullptr;
    auto ifBody = extract();
    auto cond = extract();
    if (cond && ifBody)
    {
      make<if_stmt>(*cond, *ifBody, elseBody);
    }
  }

  void builder::make_while()
  {
    auto body = extract();
    auto cond = extract();
    if (cond && body)
    {
      make<while_stmt>(*cond, *body);
    }
  }

  void builder::make_list(list::size_type count)
  {
    const auto availableSz = static_cast<list::size_type>(m_state.size());
    if (availableSz < count)
    {
      // error
      return;
    }

    const auto startPos = availableSz - count;
    auto beg = std::next(m_state.begin(), startPos);
    list::data_type items;
    items.reserve(count);
    for (auto it = beg; it != m_state.end(); ++it)
    {
      items.push_back(*it);
    }

    m_state.erase(beg, m_state.end());
    make<list>(std::move(items));
  }

  void builder::make_error(const token& at, error_msg message)
  {
    make<error>(at, message);
  }


  node* builder::extract() noexcept
  {
    if (m_state.empty())
      return {};

    auto item = m_state.back();
    m_state.pop_back();
    return item;
  }

  node* builder::peek_last() noexcept
  {
    return m_state.empty() ? nullptr : m_state.back();
  }

  void builder::wrap_in_cast(node& op, types::type t)
  {
    make<impl_cast>(op, t);
  }
}