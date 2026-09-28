#pragma once

#include "parser/token.hpp"

namespace toy
{
  class lex final
  {
  public:
    using value_type = token::value_type;
    using iter       = value_type::iterator;
    using char_t     = value_type::value_type;
    using token_opt  = std::optional<token>;

  public:
    lex() noexcept;
    ~lex() = default;

    lex(const lex&) = delete;
    lex& operator=(const lex&) = delete;
    lex(lex&&) = delete;
    lex& operator=(lex&&) = delete;

  public:
    void operator()(value_type input) noexcept;

    token next() noexcept;

    const token& peek() noexcept;

  private:
    bool good() const noexcept;

    char_t peek_char() const noexcept;

    void advance() noexcept;

    value_type read_str() const noexcept;

    void skip_spaces() noexcept;

    const token& consume(tok_kind kind) noexcept;

    const token& punctuation() noexcept;
    const token& op() noexcept;
    tok_kind op_continue(char_t next, tok_kind onTrue, tok_kind onFalse) noexcept;
    const token& number() noexcept;
    const token& identifier() noexcept;

  private:
    value_type m_buffer{};
    iter m_start;
    iter m_end;
    token_opt m_preview{};
  };
}