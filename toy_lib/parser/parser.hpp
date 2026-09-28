#pragma once

#include "parser/lex.hpp"
#include "parser/ast/builder.hpp"

namespace toy
{
  class sema;

  template <typename F>
  concept list_handler = std::is_invocable_r_v<bool, F>;
  
  class parser final
  {
  public:
    using input_t = lex::value_type;

  public:
    parser() = delete;
    ~parser() = default;

    parser(const parser&) = delete;
    parser& operator=(const parser&) = delete;
    parser(parser&&) = delete;
    parser& operator=(parser&&) = delete;

    parser(ast::builder& builder, sema& s) noexcept;

  public:
    void operator()(input_t input);

  private:
    bool good();

    void program();

    void error(const token& at, ast::builder::error_msg msg);

    void list(list_handler auto handler, tok_kind breakOn = token::Eof);

    void declaration();
    void var_decl();
    void func_decl();
    void param_decl();
    void param_list();

    void statement(bool needNewScope);
    void compound_statement();
    void return_statement();

    void condition();
    void if_statement();
    void while_statement();

    void expr();
    void add_expr();
    void mul_expr();

    void assign_expr();
    void binary_expr(std::size_t precIdx = {});
    void unary_expr();
    void postfix_expr();
    void primary_expr();
    void paren_expr();
    void literal_expr();
    void id_expr();

  private:
    lex m_lexer;
    ast::builder* m_builder;
    sema* m_sema;
  };
}