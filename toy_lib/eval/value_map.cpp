#include "eval/value_map.hpp"

namespace toy::eval
{
  val_map::val_map(val_map* global) noexcept :
    m_global{ global }
  { }

  void val_map::store(const ast::node* node, value val)
  {
    m_data.insert_or_assign(node, val);
  }

  val_map::val_opt val_map::load(const ast::node* node)
  {
    auto item = m_data.find(node);
    if (item != m_data.end())
      return val_opt{ item->second };

    return m_global ? m_global->load(node) : val_opt{};
  }
}