#include "parser/lex.hpp"

namespace toy
{
  template <typename It>
  constexpr auto is_in_range(lex::char_t c, It beg, It end) noexcept
  {
    return std::find(beg, end, c) != end;
  }

  constexpr auto is_blank(lex::char_t c) noexcept
  {
    constexpr std::array blanks{ ' ', '\n', '\t', '\f', '\v', '\r', '\0' };
    return is_in_range(c, blanks.begin(), blanks.end());
  }

  constexpr auto is_operator(lex::char_t c) noexcept
  {
    constexpr std::array ops{ '+', '-', '*', '/', '=', '<', '>', '&', '|', '!' };
    return is_in_range(c, ops.begin(), ops.end());
  }

  constexpr auto is_digit(lex::char_t c) noexcept
  {
    return c >= '0' && c <= '9';
  }
  constexpr auto is_id_start(lex::char_t c) noexcept
  {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
  }
  constexpr auto is_id_char(lex::char_t c) noexcept
  {
    return is_id_start(c) || is_digit(c);
  }

  constexpr auto is_paren_open(lex::char_t c) noexcept
  {
    return c == '(';
  }
  constexpr auto is_paren_close(lex::char_t c) noexcept
  {
    return c == ')';
  }
  constexpr auto is_paren(lex::char_t c) noexcept
  {
    return is_paren_open(c) || is_paren_close(c);
  }
  constexpr auto is_curly_open(lex::char_t c) noexcept
  {
    return c == '{';
  }
  constexpr auto is_curly_close(lex::char_t c) noexcept
  {
    return c == '}';
  }
  constexpr auto is_curly(lex::char_t c) noexcept
  {
    return is_curly_open(c) || is_curly_close(c);
  }
  constexpr auto is_semi(lex::char_t c) noexcept
  {
    return c == ';';
  }
  constexpr auto is_comma(lex::char_t c) noexcept
  {
    return c == ',';
  }
  constexpr auto is_dot(lex::char_t c) noexcept
  {
    return c == '.';
  }

  constexpr auto is_separator(lex::char_t c) noexcept
  {
    return 
      is_paren(c) ||
      is_curly(c) ||
      is_semi(c)  ||
      is_blank(c) ||
      is_comma(c) ||
      is_dot(c)   ||
      is_operator(c);
  }

  static auto lookup_keyword(token::value_type name) noexcept
  {
    using namespace std::literals;
    using kw_map = std::unordered_map<token::value_type, tok_kind>;
    static kw_map keywords
    {
      { "var"sv,    token::KwVar },
      { "fn"sv,     token::KwFn },
      { "int"sv,    token::KwInt },
      { "float"sv,  token::KwFloat },
      { "bool"sv,   token::KwBool },
      { "void"sv,   token::KwVoid },
      { "true"sv,   token::KwTrue },
      { "false"sv,  token::KwFalse },
      { "return"sv, token::KwRet },
      { "if"sv,     token::KwIf },
      { "else"sv,   token::KwElse },
      { "while"sv,  token::KwWhile },
    };

    auto found = keywords.find(name);
    return found != keywords.end() ? found->second : token::Identifier;
  }
}

namespace toy
{
  lex::lex() noexcept :
    m_start{ m_buffer.end() },
    m_end{ m_buffer.end() }
  { }

  void lex::operator()(value_type input) noexcept
  {
    m_buffer = input;
    m_start = m_buffer.begin();
    m_end = m_start;
    m_preview.reset();
    skip_spaces();
  }

  token lex::next() noexcept
  {
    auto tok = peek();
    m_preview.reset();
    return tok;
  }

  const token& lex::peek() noexcept
  {
    if (m_preview)
      return *m_preview;

    if (!good())
      return consume(token::Eof);

    const auto next = peek_char();

    if (is_id_start(next))
      return identifier();

    if (is_digit(next))
      return number();

    if (is_operator(next))
      return op();

    if (is_separator(next))
      return punctuation();

    return consume(token::Error);
  }

  bool lex::good() const noexcept
  {
    return m_end != m_buffer.end();
  }

  lex::char_t lex::peek_char() const noexcept
  {
    return good() ? *m_end : char_t{};
  }

  void lex::advance() noexcept
  {
    if (good())
      ++m_end;
  }

  lex::value_type lex::read_str() const noexcept
  {
    return { m_start, m_end };
  }

  void lex::skip_spaces() noexcept
  {
    while (good() && is_blank(peek_char()))
      advance();

    m_start = m_end;
  }

  const token& lex::consume(tok_kind kind) noexcept
  {
    if (kind == token::Error)
    {
      while (!is_separator(peek_char()))
        advance();
    }

    auto tokStr = read_str();
    if (kind == token::Identifier)
      kind = lookup_keyword(tokStr);

    auto val = kind != token::Eof ? tokStr : value_type{};
    skip_spaces();
    m_preview.emplace(val, kind);
    return *m_preview;
  }

  const token& lex::punctuation() noexcept
  {
    const auto next = peek_char();
    advance();
    auto res = token::Error;
    switch (next)
    {
    case '(': res = token::ParenOpen;  break;
    case ')': res = token::ParenClose; break;
    case '{': res = token::CurlyOpen;  break;
    case '}': res = token::CurlyClose; break;
    case ',': res = token::Comma;      break;
    case ';': res = token::Semicolon;  break;
    }

    return consume(res);
  }

  //
  // add-op:
  //  + -
  // 
  // mul_op:
  //  * /
  // 
  // unary-op:
  //  + - !
  // 
  // logical-op:
  //  || &&
  //
  // eq-op:
  //  == !=
  //
  // rel-op :
  //  < > <= >=
  //
  const token& lex::op() noexcept
  {
    const auto next = peek_char();
    advance();
    auto res = token::Error;
    switch (next)
    {
    case '+': res = token::Plus;                                        break;
    case '-': res = token::Minus;                                       break;
    case '*': res = token::Asterisk;                                    break;
    case '/': res = token::Slash;                                       break;
    case '=': res = op_continue('=', token::Eq, token::EqSign);         break;
    case '!': res = op_continue('=', token::NotEq, token::Exclamation); break;
    case '<': res = op_continue('=', token::LessEq, token::Less);       break;
    case '>': res = op_continue('=', token::GreaterEq, token::Greater); break;
    case '&': res = op_continue('&', token::LAnd, token::Error);        break;
    case '|': res = op_continue('|', token::LOr, token::Error);         break;
    }

    return consume(res);
  }

  tok_kind lex::op_continue(char_t next, tok_kind onTrue, tok_kind onFalse) noexcept
  {
    const auto nextChar = peek_char();
    if (nextChar != next)
      return onFalse;

    advance();
    return onTrue;
  }

  //
  // number:
  //  digit-sequence
  //
  // digit-sequence:
  //  digit
  //  digit-sequence digit
  //
  const token& lex::number() noexcept
  {
    auto res = token::IntNumber;
    while (good())
    {
      const auto next = peek_char();
      if (is_digit(next))
      {
        advance();
        continue;
      }

      if (is_dot(next) && res == token::IntNumber)
      {
        res = token::FloatNumber;
        advance();
        continue;
      }

      if (is_separator(next))
        break;

      res = token::Error;
      break;
    }

    return consume(res);
  }

  //
  // identifier:
  //  identifier-start
  //  identifier-start id-sequence
  // 
  // id-sequence:
  //  identifier-char id-sequence
  //  identifier-char
  //
  // identifier-start:
  //  [a - z, A - Z] _
  //
  // identifier - char :
  //  [a - z, A - Z] _[0 - 9]
  //
  const token& lex::identifier() noexcept
  {
    auto res = token::Identifier;
    while (good())
    {
      const auto next = peek_char();
      if (is_separator(next))
        break;

      advance();
      if (is_id_char(next))
      {
        continue;
      }

      res = token::Error;
      break;
    }

    return consume(res);
  }
}