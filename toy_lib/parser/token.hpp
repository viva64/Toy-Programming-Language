#pragma once

namespace toy
{
  enum class tok_kind : std::uint8_t
  {
    Error,           // unknown
    Eof,             // eof

    Plus,        // +
    Minus,       // -
    Asterisk,    // *
    Slash,       // /
    EqSign,      // =
    Eq,          // ==
    Less,        // <
    Greater,     // >
    LessEq,      // <=
    GreaterEq,   // >=
    LAnd,        // &&
    LOr,         // ||
    NotEq,       // !=
    Exclamation, // !

    ParenOpen,   // (
    ParenClose,  // )
    Comma,       // ,
    Semicolon,   // ;
    CurlyOpen,   // {
    CurlyClose,  // }

    Identifier,  // a, b, _whatev8r
    IntNumber,   // 1234, 42
    FloatNumber, // 1.2
    KwTrue,      // true
    KwFalse,     // false

    KwInt,       // int
    KwFloat,     // float
    KwBool,      // bool

    KwVoid,      // void

    KwVar,      // var
    KwFn,       // fn

    KwRet,     // return
    KwIf,      // if
    KwElse,    // else
    KwWhile    // while
  };

  class token final
  {
  public:
    using value_type = std::string_view;
    using enum tok_kind;

  public:
    token() = delete;
    ~token() = default;

    token(const token&) = default;
    token& operator=(const token&) = default;
    token(token&&) = default;
    token& operator=(token&&) = default;

    token(value_type val, tok_kind tag) noexcept :
      m_value{ val },
      m_tag{ tag }
    {}

  public:
    value_type value() const noexcept
    {
      return m_value;
    }

    tok_kind what() const noexcept
    {
      return m_tag;
    }

  private:
    value_type m_value{};
    tok_kind m_tag{};
  };
}