#pragma once

namespace toy::types
{
  enum class type : std::uint8_t
  {
    Error,
    Void,
    Bool,
    Int,
    Float,
    Function
  };

  template <type T>
  struct ct_base
  {
    static constexpr auto value = T;
  };

  template <type T1, type T2>
  struct common_type : ct_base<type::Error> {};

  template <type T>
  struct common_type<T, T> : ct_base<T> {};

  template <>
  struct common_type<type::Function, type::Function> : ct_base<type::Error> {};

  template <>
  struct common_type<type::Int, type::Bool> : ct_base<type::Int> {};
  template <>
  struct common_type<type::Bool, type::Int> : ct_base<type::Int> {};

  template <>
  struct common_type<type::Int, type::Float> : ct_base<type::Float> {};
  template <>
  struct common_type<type::Float, type::Int> : ct_base<type::Float> {};

  template <>
  struct common_type<type::Bool, type::Float> : ct_base<type::Float> {};
  template <>
  struct common_type<type::Float, type::Bool> : ct_base<type::Float> {};

  template <>
  struct common_type<type::Bool, type::Bool> : ct_base<type::Int> {};

  template <type T1, type T2>
  constexpr auto calc_ct() noexcept
  {
    return common_type<T1, T2>::value;
  }

  template <type T>
  constexpr auto calc_ct(type other) noexcept
  {
    using enum type;
    switch (other)
    {
    case Void:     return calc_ct<T, Void>();
    case Bool:     return calc_ct<T, Bool>();
    case Int:      return calc_ct<T, Int>();
    case Float:    return calc_ct<T, Float>();
    case Function: return calc_ct<T, Function>();
    }

    return Error;
  }

  constexpr auto calc_ct(type t1, type t2) noexcept
  {
    using enum type;
    switch (t1)
    {
    case Void:     return calc_ct<Void>(t2);
    case Bool:     return calc_ct<Bool>(t2);
    case Int:      return calc_ct<Int>(t2);
    case Float:    return calc_ct<Float>(t2);
    case Function: return calc_ct<Function>(t2);
    }

    return Error;
  }
}