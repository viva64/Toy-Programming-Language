#include "sema/sema.hpp"
#include "parser/token.hpp"
#include "parser/ast/ast.hpp"
#include "parser/ast/visitor.hpp"

namespace toy
{
  namespace
  {
    class type_extractor : public ast::visitor<type_extractor>
    {
    public:
      using base = ast::visitor<type_extractor>;

    public:
      auto operator()(const ast::node& node)
      {
        base::operator()(&node);
        return m_type;
      }

      bool preview(const ast::typed& typed)
      {
        m_type = typed.type();
        return false;
      }

    private:
      types::type m_type{ types::type::Error };
    };

    class func_extractor : public ast::visitor<func_extractor>
    {
    public:
      using base = ast::visitor<func_extractor>;

    public:
      const ast::func_decl* operator()(const ast::node& node)
      {
        if (!node.is_valid())
          return {};

        base::operator()(&node);
        return m_func;
      }

      bool preview(const ast::expr& e)
      {
        return e.type() == types::type::Function;
      }

      bool preview(const ast::id_expr& id)
      {
        auto&& decl = id.declaration();
        if (decl.what() == ast::node::FuncDecl)
          m_func = static_cast<const ast::func_decl*>(&decl);
        return false;
      }

    private:
      const ast::func_decl* m_func{};
    };

    class decl_extractor : public ast::visitor<decl_extractor>
    {
    public:
      using base = ast::visitor<decl_extractor>;

    public:
      const ast::decl* operator()(const ast::node& node)
      {
        if (!node.is_valid())
          return {};

        base::operator()(&node);
        return m_decl;
      }

      bool preview(const ast::expr& e)
      {
        if (e.what() == ast::node::ParenExpr)
          return true;

        return e.what() == ast::node::IdExpr;
      }

      bool preview(const ast::id_expr& id)
      {
        m_decl = &id.declaration();
        return false;
      }

    private:
      const ast::decl* m_decl{};
    };
  }

  constexpr bool is_logical(eval::operation op) noexcept
  {
    return op == eval::operation::LogOr || op == eval::operation::LogAnd;
  }

  const bool is_relational(eval::operation op) noexcept
  {
    using enum eval::operation;
    switch (op)
    {
    case Equal:
    case NEqual:
    case Less:
    case LessEq:
    case Greater:
    case GreaterEq:
      return true;
    }
    return false;
  }
}

namespace toy
{
  sema::sema()
  {
    m_global = &m_symtab.make_scope(nullptr);
  }

  bool sema::check_type_compatibility(types::type t1, types::type t2) noexcept
  {
    return types::calc_ct(t1, t2) != types::type::Error;
  }

  types::type sema::extract_type(const ast::node& node) noexcept
  {
    return type_extractor{}(node);
  }

  types::type sema::literal_type(const token& lit) noexcept
  {
    using enum types::type;
    switch (lit.what())
    {
    case token::IntNumber:
      return Int;
    
    case token::FloatNumber:
      return Float;
    
    case token::KwTrue:
    case token::KwFalse:
      return Bool;

    default:
      return Error;
    }
  }

  types::type sema::keyword_type(const token& lit) noexcept
  {
    using enum types::type;
    switch (lit.what())
    {
    case token::KwInt:
      return Int;

    case token::KwFloat:
      return Float;

    case token::KwBool:
      return Bool;

    case token::KwVoid:
      return Void;

    default:
      return Error;
    }
  }

  unary_cast sema::unary_type(const ast::node& operand, eval::operation op) noexcept
  {
    const auto operandType = extract_type(operand);
    if (op == eval::operation::UnaryNot)
      return {
               .m_exprType = types::type::Bool,
               .m_opType = operandType,
               .m_opToType = types::type::Bool
             };

    const auto resType = types::calc_ct(operandType, operandType);
    return {
             .m_exprType = resType,
             .m_opType = operandType,
             .m_opToType = resType
           };
  }

  binary_cast sema::binary_type(const ast::node& lhs, const ast::node& rhs, eval::operation op) noexcept
  {
    const auto lt = extract_type(lhs);
    const auto rt = extract_type(rhs);
    if(is_logical(op))
      return {
               .m_exprType  = types::type::Bool,
               .m_lhsType   = lt,
               .m_lhsToType = types::type::Bool,
               .m_rhsType   = rt,
               .m_rhsToType = types::type::Bool
             };

    const auto common = types::calc_ct(lt, rt);
    const auto resType = is_relational(op) ? types::type::Bool : common;
    return {
             .m_exprType = resType,
             .m_lhsType = lt,
             .m_lhsToType = common,
             .m_rhsType = rt,
             .m_rhsToType = common
           };
  }

  const ast::func_decl* sema::extract_function(const ast::node& node)
  {
    return func_extractor{}(node);
  }

  const ast::decl* sema::extract_decl(const ast::node& node)
  {
    return decl_extractor{}(node);
  }

  void sema::enter_new_scope()
  {
    if (!m_cur)
    {
      m_cur = m_global;
      return;
    }

    m_cur = &m_symtab.make_scope(m_cur);
  }

  void sema::exit_scope()
  {
    if (!m_cur)
      return;

    m_cur = m_cur->m_parent;
  }

  void sema::begin_function(ast::func_decl& fd)
  {
    m_curFn = &fd;
  }

  void sema::end_function()
  {
    if (m_curFn && m_curFn->return_type() == types::type::Error)
    {
      m_curFn->specify_ret(types::type::Void);
    }
    m_curFn = {};
  }

  void sema::record(ast::decl& decl)
  {
    if (!m_cur)
      return;

    m_symtab.record(decl, *m_cur);
  }

  ast::decl* sema::lookup(symtab::name_type name)
  {
    if (!m_cur)
      m_cur = m_global;

    return m_symtab.lookup(name, *m_cur, lookup_mode::Unscoped);
  }

  ast::decl* sema::scoped_lookup(symtab::name_type name)
  {
    if (!m_cur)
      m_cur = m_global;

    return m_symtab.lookup(name, *m_cur, lookup_mode::Scoped);
  }

  unary_cast sema::handle_return(const ast::node* retExpr)
  {
    if (!m_curFn)
      return {};

    auto retT = m_curFn->return_type();
    const auto exprT = retExpr ? extract_type(*retExpr) : types::type::Void;
    if (retT == types::type::Error)
    {
      m_curFn->specify_ret(exprT);
      retT = exprT;
    }

    if (exprT == types::type::Void)
    {
      return {
             .m_exprType = exprT,
             .m_opType = exprT,
             .m_opToType = retT
      };
    }
    
    const auto resType = types::calc_ct(retT, exprT);
    return {
             .m_exprType = resType,
             .m_opType   = exprT,
             .m_opToType = retT
    };
  }
}