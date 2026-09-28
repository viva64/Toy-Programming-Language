#include "eval/evaluator.hpp"
#include "eval/value_map.hpp"

namespace toy
{
  namespace
  {
    eval::value parse_int(token::value_type src) noexcept
    {
      auto begin = src.data();
      auto end = begin + src.length();

      eval::value ret{};
      eval::int_type result{};
      auto convRes = std::from_chars(begin, end, result, 10);
      if (convRes.ec == std::errc{ 0 })
      {
        ret = result;
      }

      return ret;
    }

    eval::value parse_float(token::value_type src) noexcept
    {
      auto begin = src.data();
      auto end = begin + src.length();

      eval::value ret{};
      eval::float_type result{};
      auto convRes = std::from_chars(begin, end, result);
      if (convRes.ec == std::errc{ 0 })
      {
        ret = result;
      }

      return ret;
    }
  }
}

namespace toy
{
  evaluator::evaluator(sema& s) noexcept :
    m_sema{ &s },
    m_globalMap{ nullptr }
  { }

  eval::value evaluator::operator()(const ast::node* node)
  {
    if (!node || !node->is_valid())
      return eval::value{};

    process(*node);

    auto mainFn = m_sema->lookup("_main");
    if (!mainFn || mainFn->what() != ast::node::FuncDecl)
      return eval::value{};

    auto main = static_cast<const ast::func_decl*>(mainFn);
    m_curFrame = &m_callStack.emplace_front(&m_globalMap);
    process(main->body());
    m_callStack.pop_front();
    m_curFrame = nullptr;

    auto res = extract();
    m_stack.clear();
    return res;
  }


  void evaluator::visit(const ast::lit_expr& lit)
  {
    if (m_returning) return;

    auto&& litVal = lit.value();
    auto data = litVal.value();
    eval::value result;
    switch (litVal.what())
    {
    case token::IntNumber:   result = parse_int(data);   break;
    case token::FloatNumber: result = parse_float(data); break;
    case token::KwTrue:      result = true; break;
    case token::KwFalse:     result = false; break;
    }

    push(result);
  }

  void evaluator::visit(const ast::unary_expr& unary)
  {
    if (m_returning) return;
    auto operand = extract();
    push(operand.unary(unary.op()));
  }

  void evaluator::visit(const ast::binary_expr& binary)
  {
    if (m_returning) return;
    const auto op = binary.op();
    using enum eval::operation;
    if (op == LogAnd || op == LogOr)
      return;

    auto rhs = extract();
    auto lhs = extract();
    push(lhs.binary(op, rhs));
  }

  bool evaluator::preview(const ast::binary_expr& binary)
  {
    if (m_returning) return false;
    const auto op = binary.op();
    using enum eval::operation;
    if (op != LogAnd && op != LogOr)
      return true;

    process(binary.left());
    const auto lval = extract().to_bool();
    if (lval && op == LogOr)
    {
      push(eval::value{ true });
      return false;
    }
    
    if (!lval && op == LogAnd)
    {
      push(eval::value{ false });
      return false;
    }

    process(binary.right());
    const auto rval = extract().to_bool();
    const auto res = op == LogOr ? lval || rval : lval && rval;
    push(eval::value{ res });
    return false;
  }

  bool evaluator::preview(const ast::if_stmt& ifStmt)
  {
    if (m_returning) return false;
    process(ifStmt.condition());
    const auto condVal = extract().to_bool();
    if (condVal)
    {
      process(ifStmt.if_body());
      return false;
    }

    if (auto elseBlock = ifStmt.else_body())
    {
      process(*elseBlock);
    }
    return false;
  }

  bool evaluator::preview(const ast::while_stmt& whileStmt)
  {
    if (m_returning) return false;
    for (;;)
    {
      process(whileStmt.condition());
      if (!extract().to_bool())
        break;

      process(whileStmt.body());
    }

    return false;
  }

  bool evaluator::preview(const ast::assign_expr& assign)
  {
    if (m_returning) return false;
    process(assign.init());
    auto value = extract();
    auto decl = sema::extract_decl(assign.assignee());
    value_map().store(decl, value);
    return false;
  }

  void evaluator::visit(const ast::var_decl& var)
  {
    if (m_returning) return;
    auto value = extract();
    value_map().store(&var, value);
  }

  void evaluator::visit(const ast::id_expr& id)
  {
    if (m_returning) return;
    auto value = value_map().load(&id.declaration());
    push(value.value_or(eval::value{}));
  }

  void evaluator::visit(const ast::ret_stmt&)
  {
    m_returning = true;
  }

  void evaluator::visit(const ast::impl_cast& cast)
  {
    if (m_returning) return;
    auto value = extract();
    push(value.cast_to(cast.type()));
  }

  bool evaluator::preview(const ast::call_expr& call)
  {
    if (m_returning) return false;
    auto func = sema::extract_function(call.callee());
    if (!func)
    {
      push(eval::value{});
      return false;
    }

    auto&& args = call.args();
    auto&& params = func->params();
    if (args.what() != ast::node::List || params.what() != ast::node::List)
    {
      push(eval::value{});
      return false;
    }

    auto&& argList   = static_cast<const ast::list&>(args).chidlren();
    auto&& paramList = static_cast<const ast::list&>(params).chidlren();

    auto&& nextFrame = m_callStack.emplace_front(&m_globalMap);
    for (auto paramIt = paramList.begin(); auto arg : argList)
    {
      auto param = *paramIt;
      process(*arg);
      auto argValue = extract();
      nextFrame.store(param, argValue);
      ++paramIt;
    }

    m_curFrame = &nextFrame;
    process(func->body());
    m_returning = false;
    m_callStack.pop_front();
    m_curFrame = !m_callStack.empty() ? &m_callStack.front() : nullptr;

    return false;
  }

  bool evaluator::preview(const ast::func_decl&)
  {
    return false;
  }


  eval::value evaluator::extract()
  {
    if (m_stack.empty())
      return eval::value{};

    auto res = m_stack.back();
    m_stack.pop_back();
    return res;
  }

  void evaluator::push(eval::value val)
  {
    m_stack.push_back(val);
  }

  void evaluator::process(const ast::node& node)
  {
    base::operator()(&node);
  }

  eval::val_map& evaluator::value_map() noexcept
  {
    if (m_curFrame)
      return *m_curFrame;

    return m_globalMap;
  }
}