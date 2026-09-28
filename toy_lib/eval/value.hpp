#pragma once
#include "sema/types.hpp"

namespace toy::eval
{
  template <typename ...Bases>
  struct visitor : Bases...
  {
    using Bases::operator()...;
  };

  enum class operation : std::uint8_t
  {
    Unknown,
    UnaryNeg,
    UnaryPos,
    UnaryNot,
    Addition,
    Subtraction,
    Multiplication,
    Division,
    Assign,
    Equal,
    NEqual,
    Less,
    Greater,
    LessEq,
    GreaterEq,
    LogAnd,
    LogOr
  };

  struct invalid_type {};
  using int_type   = std::int64_t;
  using float_type = double;
  using bool_type  = bool;

  template <typename T>
  concept valid_value = 
    std::same_as<T, int_type>   ||
    std::same_as<T, float_type> ||
    std::same_as<T, bool_type>;

  class value final
  {
  public:
    using value_type = std::variant<invalid_type, int_type, float_type, bool_type>;

  public:
    value() = default;
    ~value() = default;
    value(const value&) = default;
    value& operator=(const value&) = default;
    value(value&&) = default;
    value& operator=(value&&) = default;

    explicit value(valid_value auto val) noexcept :
      m_underlying{ val }
    { }

    value& operator=(valid_value auto val) noexcept
    {
      m_underlying = val;
      return *this;
    }

    void reset() noexcept;

    value unary(operation op) const noexcept;

    value binary(operation op, value rhs) const noexcept;

    bool_type to_bool() const noexcept;

    value cast_to(types::type t) const noexcept;

    const value_type& raw() const noexcept;

  private:
    value_type m_underlying{};
  };
}