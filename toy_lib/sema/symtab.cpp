#include "sema/symtab.hpp"
#include "parser/ast/decl.hpp"

namespace toy
{
  symtab::scope& symtab::make_scope(scope* parent)
  {
    return m_scopes.emplace_front(parent);
  }

  void symtab::record(ast::decl& decl, scope& owner)
  {
    auto name = decl.name().value();
    auto&& scopeMap = m_symbols.try_emplace(name, scope_map{}).first->second;
    scopeMap.try_emplace(&owner, &decl);
  }

  ast::decl* symtab::lookup(name_type name, scope& start, lookup_mode mode)
  {
    auto foundName = m_symbols.find(name);
    if (foundName == m_symbols.end())
      return {};

    auto curScope = &start;
    auto&& scopeMap = foundName->second;
    while (curScope)
    {
      auto sym = scopeMap.find(curScope);
      if (sym != scopeMap.end())
        return sym->second;

      if (mode == lookup_mode::Scoped)
        break;

      curScope = curScope->m_parent;
    }

    return {};
  }
}