#include "parser/parser.hpp"
#include "sema/sema.hpp"

namespace toy
{
  constexpr auto unary_op(tok_kind tk) noexcept
  {
    switch (tk)
    {
    case tok_kind::Plus:        return eval::operation::UnaryPos;
    case tok_kind::Minus:       return eval::operation::UnaryNeg;
    case tok_kind::Exclamation: return eval::operation::UnaryNot;
    
    default: return eval::operation::Unknown;
    }
  }

  constexpr auto binary_op(tok_kind tk) noexcept
  {
    switch (tk)
    {
    case tok_kind::Plus:      return eval::operation::Addition;
    case tok_kind::Minus:     return eval::operation::Subtraction;
    case tok_kind::Asterisk:  return eval::operation::Multiplication;
    case tok_kind::Slash:     return eval::operation::Division;
    case tok_kind::EqSign:    return eval::operation::Assign;
    case tok_kind::Eq:        return eval::operation::Equal;
    case tok_kind::NotEq:     return eval::operation::NEqual;
    case tok_kind::Less:      return eval::operation::Less;
    case tok_kind::Greater:   return eval::operation::Greater;
    case tok_kind::LessEq:    return eval::operation::LessEq;
    case tok_kind::GreaterEq: return eval::operation::GreaterEq;
    case tok_kind::LAnd:      return eval::operation::LogAnd;
    case tok_kind::LOr:       return eval::operation::LogOr;

    default: return eval::operation::Unknown;
    }
  }

  constexpr auto is_assign(eval::operation op) noexcept
  {
    return op == eval::operation::Assign;
  }

  constexpr auto is_type_name(tok_kind tk) noexcept
  {
    return tk == token::KwInt   ||
           tk == token::KwFloat ||
           tk == token::KwBool;
  }

  constexpr auto is_ret_type(tok_kind tk) noexcept
  {
    return is_type_name(tk) || tk == token::KwVoid;
  }

  constexpr auto is_literal(tok_kind tk) noexcept
  {
    return tk == token::IntNumber   ||
           tk == token::FloatNumber ||
           tk == token::KwFalse     ||
           tk == token::KwTrue;
  }

  enum class precedence : std::uint8_t
  {
    Additive,
    Multiplicative,
    Relational,
    Equality,
    LogicalAnd,
    LogicalOr
  };

  constexpr auto match_precedence(tok_kind tk, precedence pr) noexcept
  {
    switch (pr)
    {
    case precedence::Additive:
      return tk == token::Plus || tk == token::Minus;

    case precedence::Multiplicative:
      return tk == token::Asterisk || tk == token::Slash;

    case precedence::Relational:
      return tk == token::Less    ||
             tk == token::LessEq  ||
             tk == token::Greater ||
             tk == token::GreaterEq;

    case precedence::Equality:
      return tk == token::Eq || tk == token::NotEq;

    case precedence::LogicalAnd:
      return tk == token::LAnd;

    case precedence::LogicalOr:
      return tk == token::LOr;
    }

    return false;
  }
}

namespace toy
{
  parser::parser(ast::builder& builder, sema& s) noexcept :
    m_builder{ &builder },
    m_sema{ &s }
  { }

  void parser::operator()(input_t input)
  {
    m_lexer(input);
    program();
  }

  bool parser::good()
  {
    return m_lexer.peek().what() != token::Eof;
  }

  void parser::error(const token& at, ast::builder::error_msg msg)
  {
    m_builder->make_error(at, msg);
  }

  //
  // program:
  //  decl-list
  // 
  // decl-list:
  //  declaration
  //  decl-list declaration
  //
  void parser::program()
  {
    m_sema->enter_new_scope();
    list([&]
      {
        if (m_lexer.peek().what() == token::Semicolon)
        {
          m_lexer.next();
          return false;
        }

        declaration();
        return true;
      });
    m_builder->clear_state();
    m_sema->exit_scope();
  }

  void parser::list(list_handler auto handler, tok_kind breakOn)
  {
    ast::list::size_type count{};
    while (good())
    {
      if (m_lexer.peek().what() == breakOn)
        break;

      if(handler())
        ++count;

      if (auto last = m_builder->last(); last && last->what() == ast::node::Error)
        ++count;
    }

    m_builder->make_list(count);
  }

  //
  // declaration:
  //  ';'
  //  var-decl ';'
  //  func-decl
  //
  void parser::declaration()
  {
    auto&& next = m_lexer.peek();
    
    if (const auto nextKind = next.what(); nextKind == token::KwVar)
    {
      var_decl();
      if (auto&& semi = m_lexer.peek(); semi.what() == token::Semicolon)
      {
        m_lexer.next();
      }
      else
      {
        error(semi, "Expected ;");
      }
    }
    else if (nextKind == token::KwFn)
    {
      func_decl();
    }
    else
    {
      error(next, "Unexpected token");
      m_lexer.next();
    }
  }

  //
  // var-decl:
  //  'var' name '=' expr
  //
  void parser::var_decl()
  {
    // KwVar
    m_lexer.next();
    auto name = m_lexer.next();
    auto errCount{ 0u };
    if (name.what() != token::Identifier)
    {
      error(name, "Expected identifier");
      ++errCount;
    }
    else if (m_sema->scoped_lookup(name.value()))
    {
      error(name, "Name redefinition");
      ++errCount;
    }

    auto eq = m_lexer.next();
    if (eq.what() != token::EqSign)
    {
      error(name, "Expected =");
      ++errCount;
    }

    expr();
    if (errCount == 0u)
    {
      auto var = m_builder->make_var(name);
      m_sema->record(*var);
    }
    else
    {
      m_builder->make_list(errCount + 1);
    }
  }

  //
  // func-decl:
  //  'fn' name func-params compound-statement
  // 
  // func-params:
  //  '(' param-list ')'
  //
  void parser::func_decl()
  {
    // KwFn
    m_lexer.next();
    auto name = m_lexer.next();
    auto errCount{ 0u };
    if (name.what() != token::Identifier)
    {
      error(name, "Expected identifier");
      ++errCount;
    }
    else if (m_sema->scoped_lookup(name.value()))
    {
      error(name, "Name redefinition");
      ++errCount;
    }

    auto decl = m_builder->prepare_func(name);
    if (errCount == 0u)
    {
      m_sema->record(*decl);
      m_sema->enter_new_scope();
      m_sema->begin_function(*decl);
    }

    param_list();
    m_builder->attach_params(*decl);

    if (auto&& retType = m_lexer.peek(); is_ret_type(retType.what()))
    {
      const auto rt = sema::keyword_type(retType);
      decl->specify_ret(rt);
      m_lexer.next();
    }
    compound_statement();
    m_builder->complete_func(*decl);

    if (errCount == 0u)
    {
      m_sema->end_function();
      m_sema->exit_scope();
    }
  }

  //
  // param-list:
  //  ''
  //  param-decl
  //  param-list ',' param-decl
  //
  void parser::param_list()
  {
    bool err{};
    if (auto&& op = m_lexer.peek(); op.what() != token::ParenOpen)
    {
      err = true;
      error(op, "Expected (");
    }

    if (!err)
    {
      // (
      m_lexer.next();
    }

    list([&]
      {
        param_decl();
        if (auto&& comma = m_lexer.peek(); comma.what() != token::Comma)
        {
          if (comma.what() != token::ParenClose)
            error(comma, "Expected ,");
        }
        else
        {
          // ,
          m_lexer.next();
        }
        return true;
      },
      token::ParenClose);

    if (err)
      m_builder->make_list(2u);

    // )
    m_lexer.next();
  }

  //
  // param-decl:
  //  type-name name
  // 
  // type-name:
  //  'int' 'float' 'bool'
  //
  void parser::param_decl()
  {
    auto typeName = m_lexer.next();
    auto errCount{ 0u };
    if (!is_type_name(typeName.what()))
    {
      error(typeName, "Expected type name");
      ++errCount;
    }

    auto name = m_lexer.next();
    if (name.what() != token::Identifier)
    {
      error(name, "Expected identifier");
      ++errCount;
    }
    else if (m_sema->scoped_lookup(name.value()))
    {
      error(name, "Parameter redefinition");
      ++errCount;
    }

    if (errCount == 0u)
    {
      auto param = m_builder->make_param(typeName, name);
      m_sema->record(*param);
    }
    else
    {
      m_builder->make_list(errCount);
    }
  }

  //
  // compound-statement:
  //  '{' statement-list '}'
  //
  // statement-list :
  //  ''
  //  statement
  //  statement-list statement
  //
  void parser::compound_statement()
  {
    bool err{};
    if (auto&& curly = m_lexer.peek(); curly.what() != token::CurlyOpen)
    {
      error(curly, "Expected {");
      err = true;
    }

    // {
    m_lexer.next();
    list([&]
      {
        if (m_lexer.peek().what() == token::Semicolon)
          return false;

        statement(true);
        return true;
      },
      token::CurlyClose);

    if (err)
      m_builder->make_list(2u);

    // }
    m_lexer.next();
  }

  //
  // return-statement:
  //  'return' ''
  //  'return' expr
  //
  void parser::return_statement()
  {
    // return
    auto retKw = m_lexer.next();
    if (m_lexer.peek().what() == token::Semicolon)
    {
      m_builder->make_empty_ret(retKw, *m_sema);
      return;
    }

    expr();
    m_builder->make_ret(retKw, *m_sema);
  }

  //
  // condition:
  //  '(' expr ')'
  //
  void parser::condition()
  {
    auto errCount{ 0u };
    if (auto&& op = m_lexer.peek(); op.what() != token::ParenOpen)
    {
      error(op, "Expected (");
      ++errCount;
    }
    else
    {
      // (
      m_lexer.next();
    }

    expr();

    if (auto&& cp = m_lexer.peek(); cp.what() != token::ParenClose)
    {
      error(cp, "Expected )");
      ++errCount;
    }
    else
    {
      // (
      m_lexer.next();
    }

    if (errCount != 0)
      m_builder->make_list(errCount + 1);
  }

  //
  // if-statement:
  //  'if' condition statement
  //  else-clause
  //
  // else-clause:
  //  ''
  //  'else' statement
  //
  void parser::if_statement()
  {
    // if
    m_lexer.next();
    m_sema->enter_new_scope();
    condition();
    statement(false);
    bool hasElse{};
    if (m_lexer.peek().what() == token::KwElse)
    {
      // else
      m_lexer.next();
      statement(false);
      hasElse = true;
    }
    m_builder->make_if(hasElse);
    m_sema->exit_scope();
  }

  //
  // while-statement:
  // 'while' condition statement
  //
  void parser::while_statement()
  {
    // while
    m_lexer.next();
    m_sema->enter_new_scope();
    condition();
    statement(false);
    m_builder->make_while();
    m_sema->exit_scope();
  }

  // statement :
  //  ';'
  //  var-decl ';'
  //  expr ';'
  //  compound-statement
  //  return-statement ';'
  //  if-statement
  //  while-statement
  //
  void parser::statement(bool needNewScope)
  {
    auto needSemi = false;
    if (const auto nextKind = m_lexer.peek().what(); nextKind == token::CurlyOpen)
    {
      if(needNewScope)
        m_sema->enter_new_scope();

      compound_statement();

      if(needNewScope)
        m_sema->exit_scope();
    }
    else if(nextKind == token::KwVar)
    {
      var_decl();
      needSemi = true;
    }
    else if (nextKind == token::KwRet)
    {
      return_statement();
      needSemi = true;
    }
    else if (nextKind == token::KwIf)
    {
      if_statement();
    }
    else if (nextKind == token::KwWhile)
    {
      while_statement();
    }
    else
    {
      expr();
      needSemi = true;
    }

    if (auto&& semi = m_lexer.peek(); semi.what() == token::Semicolon)
    {
      m_lexer.next();
    }
    else if(needSemi)
    {
      error(semi, "Expected ;");
    }
  }

  //
  // expr:
  //  assign-expr
  //
  void parser::expr()
  {
    assign_expr();
  }

  //
  // assign-expr:
  //  id-expr '=' expr
  //
  void parser::assign_expr()
  {
    auto&& firstToken = m_lexer.peek();
    binary_expr();
    auto id = m_builder->last();
    const auto isId = id && id->what() == ast::node::IdExpr;
    const auto op = binary_op(m_lexer.peek().what());
    if (!is_assign(op))
      return;

    if (!isId)
    {
      error(firstToken, "Expected an identifier");
      m_builder->make_list(2u);
    }

    // =
    m_lexer.next();
    expr();
    m_builder->make_assign(op);
  }

  //
  // additive-expr:
  //  multiplicative-expr
  //  additive-expr add-op multiplicative-expr
  //
  // multiplicative-expr:
  //  unary-expr
  //  multiplicative-expr mul-op unary-expr
  //
  void parser::binary_expr(std::size_t precIdx)
  {
    static constexpr std::array precOrder
    {
      precedence::LogicalOr,
      precedence::LogicalAnd,
      precedence::Equality,
      precedence::Relational,
      precedence::Additive,
      precedence::Multiplicative
    };

    if (precIdx == precOrder.size())
    {
      unary_expr();
      return;
    }

    const auto nextIdx = precIdx + 1;
    binary_expr(nextIdx);

    while(good())
    {
      const auto opKind = m_lexer.peek().what();
      if (!match_precedence(opKind, precOrder[precIdx]))
        break;

      m_lexer.next();
      binary_expr(nextIdx);
      m_builder->make_binary(binary_op(opKind));
    }
  }

  //
  // unary-expr:
  //  postfix-expr
  //  unary-op unary-expr
  //
  void parser::unary_expr()
  {
    auto&& next = m_lexer.peek();
    const auto unaryOp = unary_op(next.what());
    if (unaryOp == eval::operation::Unknown)
    {
      postfix_expr();
      return;
    }

    m_lexer.next();
    unary_expr();
    m_builder->make_unary(unaryOp);
  }

  //
  // postfix-expr:
  //  primary-expr
  //  call-expr
  //
  // call-expr:
  //  postfix-expr args
  // 
  // args:
  //  '(' expr-list ')'
  //
  void parser::postfix_expr()
  {
    primary_expr();
    while (good())
    {
      if (m_lexer.peek().what() != token::ParenOpen)
        break;

      // (
      m_lexer.next();
      list([&]
        {
          expr();
          if (auto&& comma = m_lexer.peek(); comma.what() != token::Comma)
          {
            if (comma.what() != token::ParenClose)
              error(comma, "Expected ,");
          }
          else
          {
            m_lexer.next();
          }
          return true;
        },
        token::ParenClose);

      // )
      m_lexer.next();
      m_builder->make_call();
    }
  }

  //
  // primary-expr:
  //  paren-expr
  //  literal-expr
  //
  void parser::primary_expr()
  {
    auto&& next = m_lexer.peek();
    const auto kind = next.what();
    if (kind == token::ParenOpen)
    {
      paren_expr();
    }
    else if (is_literal(kind))
    {
      literal_expr();
    }
    else if (kind == token::Identifier)
    {
      id_expr();
    }
    else
    {
      auto errPos = m_lexer.next();
      error(errPos, "Unexpected token");
    }
  }

  //
  // paren-expr:
  //  ( expr )
  //
  void parser::paren_expr()
  {
    // (
    m_lexer.next();
    expr();
    if (auto&& closeParen = m_lexer.peek(); closeParen.what() != token::ParenClose)
    {
      error(closeParen, "Expected )");
      m_builder->make_list(2u);
    }

    // )
    m_lexer.next();
    m_builder->make_paren();
  }

  //
  // literal-expr:
  //  int-number
  //  float-number
  //  'true'
  //  'false'
  //
  void parser::literal_expr()
  {
    auto num = m_lexer.next();
    m_builder->make_literal(num);
  }

  //
  // id-expr:
  //  identifier
  //
  void parser::id_expr()
  {
    auto name = m_lexer.next();
    auto foundVar = m_sema->lookup(name.value());
    if (!foundVar)
    {
      error(name, "Undeclared identifier");
      return;
    }
    m_builder->make_id(*foundVar);
  }
}