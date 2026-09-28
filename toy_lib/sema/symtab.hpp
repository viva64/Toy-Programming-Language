#pragma once

namespace toy::ast
{
  class decl;
}

namespace toy
{
  enum class lookup_mode : std::uint8_t
  {
    Scoped,
    Unscoped
  };

  class symtab final
  {
  public:
    struct scope
    {
      scope* m_parent{};
    };

    using name_type = std::string_view;
    using scope_list = std::forward_list<scope>;
    using scope_map = std::unordered_map<scope*, ast::decl*>;
    using symbol_map = std::unordered_map<name_type, scope_map>;

  public:
    symtab() = default;
    ~symtab() = default;

    symtab(const symtab&) = delete;
    symtab& operator=(const symtab&) = delete;
    symtab(symtab&&) = delete;
    symtab& operator=(symtab&&) = delete;

  public:
    scope& make_scope(scope* parent);

    void record(ast::decl& decl, scope& owner);

    ast::decl* lookup(name_type name, scope& start, lookup_mode mode);

  private:
    scope_list m_scopes;
    symbol_map m_symbols;
  };
}