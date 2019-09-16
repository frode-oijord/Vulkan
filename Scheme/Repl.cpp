
#include <Scheme.h>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>

#include <string>
#include <chrono>
#include <iostream>

#include <string>
#include <complex>


using namespace scm;

namespace ast {
  class symbol : public std::string {};
}


namespace client
{
  namespace x3 = boost::spirit::x3;
  namespace ascii = boost::spirit::x3::ascii;

  using x3::double_;
  using x3::phrase_parse;
  using x3::_attr;
  using x3::lexeme;
  using x3::alnum;
  using ascii::space;
  using ascii::char_;

  x3::rule<class number, double> const number = "number";
  auto const number_def = double_;
  BOOST_SPIRIT_DEFINE(number);

  x3::rule<class string, std::string> const string = "string";
  auto const string_def = lexeme['"' >> +(char_ - '"') >> '"'];
  BOOST_SPIRIT_DEFINE(string);

  x3::rule<class symbol, std::string> const symbol = "symbol";
  auto const symbol_def = +alnum;
  BOOST_SPIRIT_DEFINE(symbol);


  template <typename Iterator>
  bool parse_numbers(Iterator first, Iterator last, std::vector<std::any>& v)
  {
    bool r = phrase_parse(first, last,

      //  Begin grammar
      (
        '(' >> (number | string | symbol) % ',' >> ')'
      )
      ,
      //  End grammar

      space, v);

    if (first != last) // fail if we did not get a full match
      return false;
    return r;
  }
}


int main(int, char **)
{
  std::cout << "Innovator Scheme REPL" << std::endl;
  env_ptr env = scm::global_env();

  while (true) {
    try {
      std::cout << "> ";
      std::string input;
      std::getline(std::cin, input);
      auto start = std::chrono::high_resolution_clock::now();

      std::vector<std::any> v;
      if (client::parse_numbers(input.begin(), input.end(), v))
      {
        std::cout << "-------------------------\n";
        std::cout << "Parsing succeeded\n";
        std::cout << "got: " << std::endl;
        for (auto & n : v) {
          if (n.type() == typeid(double)) {
            std::cout << std::any_cast<double>(n) << " ";
          }
          if (n.type() == typeid(std::string)) {
            std::cout << std::any_cast<std::string>(n) << " ";
          }
          if (n.type() == typeid(ast::symbol)) {
            std::cout << std::any_cast<ast::symbol>(n) << " ";
          }
        }
      }
      else
      {
        std::cout << "-------------------------\n";
        std::cout << "Parsing failed\n";
        std::cout << "-------------------------\n";
      }

      //std::any exp = scm::read(input);
      //auto result = scm::eval(exp, env);

      auto finish = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> elapsed = finish - start;
      //scm::print(result);
      std::cout << std::endl;
      std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;
    }
    catch (std::exception & e) {
      std::cerr << e.what() << std::endl;
    }
  }
  return 1;
}
