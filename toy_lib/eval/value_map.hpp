#pragma once
#include "parser/ast/ast.hpp"
#include "eval/value.hpp"

namespace toy::eval
{
  class val_map final
  {
  public:
    using data = std::unordered_map<const ast::node*, value>;
    using val_opt = std::optional<value>;

  public:
    val_map() = delete;
    ~val_map() = default;

    val_map(const val_map&) = delete;
    val_map& operator=(const val_map&) = delete;
    val_map(val_map&&) = delete;
    val_map& operator=(val_map&&) = delete;

    explicit val_map(val_map* global) noexcept;

  public:
    void store(const ast::node* node, value val);

    val_opt load(const ast::node* node);

  private:
    data m_data;
    val_map* m_global;
  };
}