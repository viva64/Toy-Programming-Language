#include "parser/parser.hpp"
#include "sema/sema.hpp"
#include "parser/ast/visitor.hpp"
#include "eval/evaluator.hpp"

// The path to the source file is hard-coded here for simplicity
// Change before running
constexpr std::string_view filePath{ "<path>/input.txt" };

class ast_printer :
  public toy::ast::visitor<ast_printer>
{
public:
  using base = toy::ast::visitor<ast_printer>;
  using child_count = std::size_t;
  using child_tracker = std::vector<child_count>;
  using size_type = child_tracker::size_type;

public:
  ast_printer() = default;
  ~ast_printer() = default;

  ast_printer(const ast_printer&) = delete;
  ast_printer& operator=(const ast_printer&) = delete;
  ast_printer(ast_printer&&) = delete;
  ast_printer& operator=(ast_printer&&) = delete;

  void operator()(const toy::ast::node* node)
  {
    m_indetations.clear();
    base::operator()(node);
  }

public:
  bool preview(const toy::ast::error& err)
  {
    indent();
    std::cout << "error: '" << err.message() << '\'';
    if(auto at = err.at().value(); !at.empty())
      std::cout << " at " << err.at().value();
    endl();
    return true;
  }

  bool preview(const toy::ast::var_decl& decl)
  {
    using namespace std::literals;
    indent();
    print_sym("variable", decl);
    print_type(decl.type());
    endl();
    m_indetations.push_back(1u);
    return true;
  }

  bool preview(const toy::ast::func_decl& decl)
  {
    using namespace std::literals;
    indent();
    print_sym("function", decl);
    std::cout << " returns";
    print_type(decl.return_type());
    endl();
    m_indetations.push_back(2u);
    return true;
  }

  bool preview(const toy::ast::param_decl& decl)
  {
    using namespace std::literals;
    indent();
    print_sym("param", decl);
    print_type(decl.type());
    endl();
    return true;
  }

  bool preview(const toy::ast::lit_expr& expr)
  {
    indent();
    std::cout << "literal '" << expr.value().value() << '\'';
    print_type(expr.type());
    endl();
    return true;
  }

  bool preview(const toy::ast::id_expr& expr)
  {
    using namespace std::literals;
    indent();
    print_sym("id", expr.declaration());
    print_type(expr.type());
    endl();
    return true;
  }

  bool preview(const toy::ast::paren_expr& p)
  {
    indent();
    node_designator(p, "paren");
    print_type(p.type());
    endl();
    m_indetations.push_back(1u);
    return true;
  }

  bool preview(const toy::ast::unary_expr& expr)
  {
    indent();
    std::cout << "unary '";
    switch (expr.op())
    {
    case toy::eval::operation::UnaryPos: std::cout << '+'; break;
    case toy::eval::operation::UnaryNeg: std::cout << '-'; break;
    case toy::eval::operation::UnaryNot: std::cout << '!'; break;

    default: std::cout << "unknown"; break;
    }

    std::cout << '\'';
    error_mark(expr);
    print_type(expr.type());
    endl();
    m_indetations.push_back(1u);
    return true;
  }

  bool preview(const toy::ast::binary_expr& expr)
  {
    indent();
    std::cout << "binary '";
    switch (expr.op())
    {
    case toy::eval::operation::Addition:       std::cout << '+';  break;
    case toy::eval::operation::Subtraction:    std::cout << '-';  break;
    case toy::eval::operation::Multiplication: std::cout << '*';  break;
    case toy::eval::operation::Division:       std::cout << '/';  break;
    case toy::eval::operation::Equal:          std::cout << "=="; break;
    case toy::eval::operation::NEqual:         std::cout << "!="; break;
    case toy::eval::operation::Less:           std::cout << '<';  break;
    case toy::eval::operation::Greater:        std::cout << '>';  break;
    case toy::eval::operation::LessEq:         std::cout << "<="; break;
    case toy::eval::operation::GreaterEq:      std::cout << ">="; break;
    case toy::eval::operation::LogAnd:         std::cout << "&&"; break;
    case toy::eval::operation::LogOr:          std::cout << "||"; break;

    default: std::cout << "unknown"; break;
    }

    std::cout << '\'';
    error_mark(expr);
    print_type(expr.type());
    endl();
    m_indetations.push_back(2u);
    return true;
  }

  bool preview(const toy::ast::assign_expr& expr)
  {
    indent();
    std::cout << "assign '";
    switch (expr.op())
    {
    case toy::eval::operation::Assign:       std::cout << '='; break;

    default: std::cout << "unknown"; break;
    }

    std::cout << '\'';
    error_mark(expr);
    print_type(expr.type());
    endl();
    m_indetations.push_back(2u);
    return true;
  }

  bool preview(const toy::ast::call_expr& c)
  {
    indent();
    node_designator(c, "call");
    print_type(c.type());
    endl();
    m_indetations.push_back(2u);
    return true;
  }

  bool preview(const toy::ast::list& list)
  {
    indent();
    node_designator(list, "list");
    endl();
    m_indetations.push_back(list.chidlren().size());
    return true;
  }

  bool preview(const toy::ast::ret_stmt& ret)
  {
    indent();
    if (!ret.ret_expr())
    {
      node_designator(ret, "return <empty>");
    }
    else
    {
      node_designator(ret, "return");
      m_indetations.push_back(1u);
    }

    endl();
    return true;
  }

  bool preview(const toy::ast::if_stmt& is)
  {
    indent();
    node_designator(is, "if");
    endl();
    const auto childCount = is.else_body() ? 3u : 2u;
    m_indetations.push_back(childCount);
    return true;
  }

  bool preview(const toy::ast::while_stmt& ws)
  {
    indent();
    node_designator(ws, "while");
    endl();
    m_indetations.push_back(2u);
    return true;
  }

  bool preview(const toy::ast::impl_cast& ic)
  {
    indent();
    node_designator(ic, "implicit-cast");
    print_type(ic.type());
    endl();
    m_indetations.push_back(1u);
    return true;
  }

private:
  void error_mark(const toy::ast::node& node)
  {
    if (!node.is_valid())
    {
      std::cout << " has-errors";
    }
  }

  void node_designator(const toy::ast::node& node, std::string_view name)
  {
    std::cout << name;
    error_mark(node);
  }

  void endl()
  {
    std::cout << '\n';
  }

  void print_type(toy::types::type t)
  {
    std::cout << ' ';
    using enum toy::types::type;
    switch (t)
    {
    case Void:     std::cout << "void"; break;
    case Bool:     std::cout << "bool"; break;
    case Int:      std::cout << "int"; break;
    case Float:    std::cout << "float"; break;
    case Function: std::cout << "func"; break;

    default: std::cout << "error-type"; break;
    }
    std::cout << ' ';
  }

  void indent()
  {
    while (!m_indetations.empty())
    {
      if (m_indetations.back())
        break;

      m_indetations.pop_back();
    }

    if (m_indetations.empty())
      return;

    auto&& last = m_indetations.back();
    --last;
    for (auto&& cur : m_indetations)
    {
      if (&cur != &last)
      {
        print_parent(cur);
        continue;
      }

      print_child(last);
    }
  }

  void print_parent(child_count count)
  {
    using namespace std::literals;
    static constexpr auto childIndent = "| "sv;
    static constexpr auto blankIndent = "  "sv;

    if (!count)
      std::cout << blankIndent;
    else
      std::cout << childIndent;
  }

  void print_child(child_count count)
  {
    using namespace std::literals;
    static constexpr auto prefix = "|-"sv;
    static constexpr auto lastPrefix = "`-"sv;

    if (!count)
      std::cout << lastPrefix;
    else
      std::cout << prefix;
  }

  void print_sym(std::string_view prefix, const toy::ast::decl& decl)
  {
    using namespace std::literals;
    std::cout << prefix << " '" << decl.name().value() << "' [";
    std::print(std::cout, "{:X}"sv, reinterpret_cast<std::size_t>(&decl));
    std::cout << ']';
    error_mark(decl);
  }

private:
  child_tracker m_indetations;
};

auto read_file() noexcept
{
  std::string buffer;
  std::ifstream in{ filePath.data() };

  in.seekg(0, std::ios::end);
  buffer.reserve(in.tellg());
  in.seekg(0, std::ios::beg);

  using it = std::istreambuf_iterator<std::string::value_type>;
  buffer.assign(it{ in }, it{});
  return buffer;
}

void echo(std::string_view input)
{
  std::cout << "input:\n" << input << "\n\n";
}

void test_eval(std::string_view input)
{
  toy::sema sema;
  toy::ast::builder builder;
  toy::parser parser{ builder, sema };
  toy::evaluator ev{ sema };
  parser(input);
  //ast_printer{}(builder.root());
  auto res = ev(builder.root());
  std::cout << "Evaluator returned: ";
  toy::eval::visitor vis
  {
    [](auto)
    {
      std::cout << "error";
    },
    [](toy::eval::valid_value auto v)
    {
      std::cout << v;
    }
  };
  std::visit(vis, res.raw());
  std::cout << '\n';
}

void test_lex(std::string_view input)
{
  echo(input);
  toy::lex lex;
  lex(input);
  for (;;)
  {
    auto tok = lex.next();
    switch (tok.what())
    {
    case toy::token::Eof:
      std::cout << "eof\n"; return;

    case toy::token::IntNumber:
      std::cout << "int"; break;
    case toy::token::FloatNumber:
      std::cout << "float"; break;

    case toy::token::Identifier:
      std::cout << "identifier"; break;

    case toy::token::KwVar:
    case toy::token::KwFn:
    case toy::token::KwInt:
    case toy::token::KwFloat:
    case toy::token::KwBool:
    case toy::token::KwTrue:
    case toy::token::KwFalse:
    case toy::token::KwRet:
      std::cout << "keyword"; break;

    case toy::token::Plus:
    case toy::token::Minus:
    case toy::token::Asterisk:
    case toy::token::Slash:
    case toy::token::EqSign:
    case toy::token::Eq:
    case toy::token::Less:
    case toy::token::LessEq:
    case toy::token::Greater:
    case toy::token::GreaterEq:
    case toy::token::LAnd:
    case toy::token::LOr:
    case toy::token::NotEq:
    case toy::token::Exclamation:
      std::cout << "operator"; break;

    case toy::token::ParenOpen:
    case toy::token::ParenClose:
      std::cout << "paren"; break;

    case toy::token::CurlyOpen:
    case toy::token::CurlyClose:
      std::cout << "curly"; break;

    case toy::token::Semicolon:
      std::cout << "semi"; break;

    case toy::token::Comma:
      std::cout << "comma"; break;

    default:
      std::cout << "error"; break;
    }

    std::cout << ": '" << tok.value() << "'\n";
  }
}

void test_parser(std::string_view input)
{
  toy::sema sema;
  toy::ast::builder builder;
  toy::parser parser{ builder, sema };
  parser(input);
  ast_printer{}(builder.root());
}

int main()
{
  //test_lex("== ! != && || < > <= >= +-*/= 1.2s _id1 {};,() true false 1 1.0 var fn int float bool");
  auto buffer = read_file();
  test_parser(buffer);
  return 0;
}