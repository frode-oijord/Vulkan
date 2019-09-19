
#include <Scheme.h>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/io.hpp>

#include <map>
#include <tuple>
#include <string>
#include <chrono>
#include <iostream>

#include <string>
#include <complex>

namespace fusion = boost::fusion;
namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;

using x3::lit;
using x3::alnum;
using x3::alpha;
using x3::lexeme;
using x3::double_;

using ascii::char_;
using ascii::string;

// AST definition
namespace client {
  namespace ast
  {
    struct expr_value : x3::variant<
      scm::Symbol,
      scm::Boolean,
      scm::Number,
      std::string,
      x3::forward_ast<struct expr>> {

      using base_type::base_type;
      using base_type::operator=;
    };

    struct expr {
      std::vector<expr_value> values;
    };

    struct scm_transformer {
      typedef std::any result_type;

      std::any operator()(expr const& ast) const
      {
        scm::List list;
        for (auto & value : ast.values) {
          list.push_back(boost::apply_visitor(scm_transformer(), value));
        }
        return list;
      }

      std::any operator()(std::string const& value) const {
        return value;
      }
      std::any operator()(scm::Number const& value) const {
        return value;
      }
      std::any operator()(scm::Symbol const& value) const {
        return value;
      }
      std::any operator()(scm::Boolean const& value) const {
        return value;
      }
    };
  }
}

namespace client {
  namespace parser {
    
    struct bool_table : x3::symbols<bool> {
      bool_table() {
        add("#t", true) ("#f", false);
      }
    } const boolean;

    x3::rule<class expr, ast::expr> expr = "expr";
    x3::rule<class symbol, scm::Symbol> const symbol = "symbol";
    x3::rule<class expr_value, ast::expr_value> expr_value = "expr_value";

    auto const number = double_;
    auto const symbol_def = lexeme[+(char_("A-Za-z") | char_("0-9") | char_('_') | char_('-') | char_("+*/%~&|^!=<>?"))];
    auto const quoted_string = lexeme['"' >> *(char_ - '"') >> '"'];
    auto const expr_value_def = number | boolean| quoted_string | symbol | expr;
    auto const expr_def = '(' >> *expr_value >> ')';

    BOOST_SPIRIT_DEFINE(expr, symbol, expr_value);
  }
}

// We need to tell fusion about our expr struct to make it a first-class fusion citizen
BOOST_FUSION_ADAPT_STRUCT(client::ast::expr, values)


int main(int, char **)
{
  std::cout << "Innovator Scheme REPL" << std::endl;

  scm::env_ptr env = scm::global_env();

  while (true) {
    try {
      std::cout << "> ";
      std::string input;
      std::getline(std::cin, input);
      using client::parser::expr; // Our grammar
      client::ast::expr ast; // Our tree

      using boost::spirit::x3::ascii::space;
      std::string::const_iterator iter = input.begin();
      std::string::const_iterator end = input.end();
      bool r = phrase_parse(iter, end, expr, space, ast);

      if (r && iter == end) {
        client::ast::scm_transformer t;
        std::any exp = t(ast);
        exp = scm::eval(scm::parse(exp), env);
        scm::print(exp); std::cout << std::endl;
      }
      else {
        std::string::const_iterator some = iter + 30;
        std::string context(iter, (some > end) ? end : some);
        std::cout << "-------------------------\n";
        std::cout << "Parsing failed\n";
        std::cout << "stopped at: \": " << context << "...\"\n";
        std::cout << "-------------------------\n";
      }
    }
    catch (std::exception & e) {
      std::cerr << e.what() << std::endl;
    }
  }
  return 1;
}
