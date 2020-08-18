#include <boost/fusion/adapted.hpp>
#include <boost/spirit/home/x3.hpp>

#include <vector>
#include <string>
#include <variant>
#include <iostream>

namespace ast {
  struct ordinary_pawn_move : public std::string {};
  struct ordinary_pawn_capture : public std::string {};

  typedef std::variant<ordinary_pawn_move, ordinary_pawn_capture, std::string> move_t;

  struct move : move_t {
    using base_type = move_t;
    using base_type::variant;
  };

  struct {
    template <typename T>
    void operator()(T const& v) const { std::cout << typeid(T).name() << " " << v << std::endl; }
  } visitor;
}

namespace parser {
  namespace x3 = boost::spirit::x3;
  using x3::int_;
  using x3::char_;
  using x3::lexeme;
  using x3::double_;

  x3::rule<struct ordinary_pawn_move_class, ast::ordinary_pawn_move> ordinary_pawn_move = "ordinary_pawn_move";
  const auto ordinary_pawn_move_def = char_('a', 'h') >> char_('1', '8');
  BOOST_SPIRIT_DEFINE(ordinary_pawn_move);

  x3::rule<struct ordinary_pawn_capture_class, ast::ordinary_pawn_capture> ordinary_pawn_capture = "ordinary_pawn_capture";
  const auto ordinary_pawn_capture_def = char_('a', 'h') >> char_('x') >> char_('a', 'h') >> char_('1', '8');
  BOOST_SPIRIT_DEFINE(ordinary_pawn_capture);

  x3::rule<struct move_class, ast::move> move = "move";
  const auto move_def
    = ordinary_pawn_move
    | ordinary_pawn_capture;

  BOOST_SPIRIT_DEFINE(move);

  const auto entry_point = x3::skip(x3::space)[move];
}
