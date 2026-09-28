#pragma once
#include "parser/ast/visitor.hpp"
#include "sema/sema.hpp"
#include "eval/value.hpp"
#include "eval/value_map.hpp"

namespace toy
{
  class evaluator final : public ast::visitor<evaluator>
  {
  public:
    using base = ast::visitor<evaluator>;
    using eval_stack = std::vector<eval::value>;
    using call_stack = std::forward_list<eval::val_map>;

  public:
    evaluator() = delete;
    ~evaluator() = default;

    evaluator(const evaluator&) = delete;
    evaluator& operator=(const evaluator&) = delete;
    evaluator(evaluator&&) = delete;
    evaluator& operator=(evaluator&&) = delete;

    explicit evaluator(sema& s) noexcept;

    eval::value operator()(const ast::node* node);

  public:
    void visit(const ast::lit_expr& lit);

    void visit(const ast::unary_expr& unary);

    void visit(const ast::binary_expr& binary);

    bool preview(const ast::binary_expr& binary);

    bool preview(const ast::if_stmt& ifStmt);

    bool preview(const ast::while_stmt& whileStmt);

    bool preview(const ast::assign_expr& assign);

    void visit(const ast::var_decl& var);

    void visit(const ast::id_expr& id);

    void visit(const ast::ret_stmt& ret);

    void visit(const ast::impl_cast& cast);

    bool preview(const ast::call_expr& call);

    bool preview(const ast::func_decl& fd);

  private:
    eval::value extract();

    void push(eval::value val);

    void process(const ast::node& node);

    eval::val_map& value_map() noexcept;

  private:
    eval::val_map m_globalMap;
    eval::val_map* m_curFrame{};
    eval_stack m_stack;
    call_stack m_callStack;
    sema* m_sema;
    bool m_returning{};
  };
}