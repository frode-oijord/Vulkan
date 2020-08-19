#include <boost/fusion/adapted.hpp>
#include <boost/spirit/home/x3.hpp>

#include <vector>
#include <string>
#include <variant>
#include <iostream>


namespace ast {
	struct tag : std::string {};
	struct comment : std::string {};
	struct move {
		int move_number;
		std::string white;
		std::string black;
	};

	typedef std::variant<tag, comment, move> value;

	struct visitor {
		void operator()(move const& m) {
			std::cout << m.move_number << " " << m.white << " " << m.black << std::endl;
		}
		void operator()(tag const& t) {
			std::cout << t << std::endl;
		}
		void operator()(comment const& c) {
			std::cout << c << std::endl;
		}
	};
}

BOOST_FUSION_ADAPT_STRUCT(ast::move, move_number, white, black)

namespace move_parser {
	namespace x3 = boost::spirit::x3;
	using x3::int_;
	using x3::char_;
	using x3::alnum;
	using x3::lexeme;
	using x3::string;

	x3::rule<struct tag, ast::tag> tag = "tag";
	x3::rule<struct comment, ast::comment> comment = "comment";
	x3::rule<struct move, ast::move> move = "move";
	x3::rule<struct value, ast::value> value = "value";
	x3::rule<struct game, std::vector<ast::value>> game = "game";

	const auto tag_def = lexeme['[' >> +(char_ - ']') >> ']'];
	const auto comment_def = lexeme['{' >> +(char_ - '}') >> '}'];
	const auto move_def = lexeme[+int_ >> '.' >> ' ' >> +(char_ - ' ') >> ' ' >> +(char_ - ' ')];

	const auto value_def = tag | comment | move;
	const auto game_def = *value;

	BOOST_SPIRIT_DEFINE(tag, comment, move, value, game);

	const auto entrypoint = x3::skip(x3::space)[game];
}

bool read(std::string game)
{
	std::vector<ast::value> values;
	auto begin = game.begin();
	bool ok = parse(begin, game.end(), move_parser::entrypoint, values);

	if (!ok || begin != game.end()) {
		std::cerr << "parse failed: remaining input:\n" << std::string(begin, game.end());
	}

	ast::visitor visitor;
	for (auto v : values) {
		std::visit(visitor, v);
	}
	return ok;
}
