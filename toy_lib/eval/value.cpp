#include "eval/value.hpp"

namespace toy::eval
{
  namespace
  {
    constexpr auto to_int(bool_type b) noexcept
    {
      return static_cast<int_type>(b);
    }

    auto equal(float_type lhs, float_type rhs) noexcept
    {
      constexpr auto max_diff = std::numeric_limits<float_type>::epsilon();
      return std::abs(lhs - rhs) <= max_diff;
    }

    auto unary_neg(valid_value auto op) noexcept
    {
      return value{ -op };
    }
    auto unary_neg(bool_type op) noexcept
    {
      return unary_neg(to_int(op));
    }

    template <valid_value T>
    auto binary_add(T lhs, T rhs) noexcept
    {
      return value{ lhs + rhs };
    }
    auto binary_add(bool_type lhs, bool_type rhs) noexcept
    {
      return binary_add(to_int(lhs), to_int(rhs));
    }

    template <valid_value T>
    auto binary_sub(T lhs, T rhs) noexcept
    {
      return value{ lhs - rhs };
    }
    auto binary_sub(bool_type lhs, bool_type rhs) noexcept
    {
      return binary_sub(to_int(lhs), to_int(rhs));
    }

    template <valid_value T>
    auto binary_mul(T lhs, T rhs) noexcept
    {
      return value{ lhs * rhs };
    }
    auto binary_mul(bool_type lhs, bool_type rhs) noexcept
    {
      return binary_mul(to_int(lhs), to_int(rhs));
    }

    template <valid_value T>
    auto binary_div(T lhs, T rhs) noexcept
    {
      return value{ lhs / rhs };
    }
    auto binary_div(bool_type lhs, bool_type rhs) noexcept
    {
      return binary_div(to_int(lhs), to_int(rhs));
    }

    template <valid_value T>
    auto binary_eq(T lhs, T rhs) noexcept
    {
      return value{ lhs == rhs };
    }
    auto binary_eq(float_type lhs, float_type rhs) noexcept
    {
      return value{ equal(lhs, rhs) };
    }

    template <valid_value T>
    auto binary_lt(T lhs, T rhs) noexcept
    {
      return value{ lhs < rhs };
    }
    auto binary_lt(bool_type lhs, bool_type rhs) noexcept
    {
      return binary_lt(to_int(lhs), to_int(rhs));
    }

    template <valid_value T>
    auto binary_gt(T lhs, T rhs) noexcept
    {
      return value{ lhs > rhs };
    }
    auto binary_gt(bool_type lhs, bool_type rhs) noexcept
    {
      return binary_gt(to_int(lhs), to_int(rhs));
    }

    template <valid_value T>
    auto binary_lte(T lhs, T rhs) noexcept
    {
      return value{ lhs <= rhs };
    }
    auto binary_lte(float_type lhs, float_type rhs) noexcept
    {
      return value{ lhs < rhs || equal(lhs, rhs) };
    }
    auto binary_lte(bool_type lhs, bool_type rhs) noexcept
    {
      return binary_lte(to_int(lhs), to_int(rhs));
    }

    template <valid_value T>
    auto binary_gte(T lhs, T rhs) noexcept
    {
      return value{ lhs >= rhs };
    }
    auto binary_gte(float_type lhs, float_type rhs) noexcept
    {
      return value{ lhs > rhs || equal(lhs, rhs) };
    }
    auto binary_gte(bool_type lhs, bool_type rhs) noexcept
    {
      return binary_gte(to_int(lhs), to_int(rhs));
    }
  }
}

namespace toy::eval
{
  void value::reset() noexcept
  {
    m_underlying = invalid_type{};
  }

  value value::unary(operation op) const noexcept
  {
    auto vis = visitor
    {
      [&](auto val) noexcept
      {
        if constexpr (std::same_as<decltype(val), invalid_type>)
          return value{};
        else
        {
          using enum operation;
          switch (op)
          {
          case UnaryPos: return value{ val };
          case UnaryNeg: return unary_neg(val);
          case UnaryNot: return value{ !static_cast<bool_type>(val) };
          }
          return value{};
        }
      }
    };

    return std::visit(vis, m_underlying);
  }

  value value::binary(operation op, value rhs) const noexcept
  {
    auto vis = visitor
    {
      [&](auto l, auto r) noexcept
      {
        using lt = decltype(l);
        using rt = decltype(r);
        if constexpr (std::same_as<lt, invalid_type> ||
          std::same_as<rt, invalid_type> ||
          !std::same_as<lt, rt>)
          return value{};
        else
        {
          using enum operation;
          switch (op)
          {
          case Addition:       return binary_add(l, r);
          case Subtraction:    return binary_sub(l, r);
          case Multiplication: return binary_mul(l, r);
          case Division:       return binary_div(l, r);
          case Equal:          return binary_eq(l, r);
          case NEqual:         return binary_eq(l, r).unary(UnaryNot);
          case Less:           return binary_lt(l, r);
          case Greater:        return binary_gt(l, r);
          case LessEq:         return binary_lte(l, r);
          case GreaterEq:      return binary_gte(l, r);
          }
          return value{};
        }
      }
    };
    return std::visit(vis, m_underlying, rhs.m_underlying);
  }

  bool_type value::to_bool() const noexcept
  {
    return std::visit(
      visitor
      {
        [](invalid_type) noexcept
        {
          return false;
        },
        [](auto val) noexcept
        {
          return static_cast<bool_type>(val);
        }
      },
    m_underlying);
  }

  value value::cast_to(types::type t) const noexcept
  {
    auto vis = visitor
    {
      [](invalid_type) noexcept
      {
        return value{};
      },
      [&](auto val) noexcept
      {
        using enum types::type;
        switch (t)
        {
        case Int:   return value{ static_cast<int_type>(val) };
        case Float: return value{ static_cast<float_type>(val) };
        case Bool:  return value{ static_cast<bool_type>(val) };
        }
        return value{};
      }
    };
    return std::visit(vis, m_underlying);
  }

  const value::value_type& value::raw() const noexcept
  {
    return m_underlying;
  }
}