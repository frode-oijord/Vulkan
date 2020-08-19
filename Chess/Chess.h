#include <boost/fusion/adapted.hpp>
#include <boost/spirit/home/x3.hpp>

#include <vector>
#include <string>
#include <variant>
#include <iostream>


namespace chess {
	class move {
	public:
		move(std::string description) :
			description(std::move(description))
		{}
		std::string description;
	};
}


namespace ast {
	struct tag : std::string {};
	struct comment : std::string {};
	struct move : std::vector<char> {};
	struct pawn_move : move {};
	struct pawn_capture : move {};
	struct pawn_capture_ep : move {};
	struct pawn_promotion : move {};
	struct pawn_capture_promotion : move {};
	struct piece_move : move {};
	struct piece_capture : move {};
	struct explicit_piece_move : move {};
	struct castles_kingside : std::string {};
	struct castles_queenside : std::string {};

	typedef std::variant<
		int,
		tag,
		comment,
		pawn_move,
		pawn_capture,
		pawn_capture_ep,
		pawn_promotion,
		pawn_capture_promotion,
		piece_move,
		piece_capture,
		explicit_piece_move,
		castles_kingside,
		castles_queenside> value;

	struct visitor {
		void operator()(int const& move) {
		}
		void operator()(tag const& e) {
			std::cout << e << std::endl;
		}
		void operator()(comment const& e) {
			std::cout << "Comment: " << e << std::endl;
		}
		void operator()(castles_kingside const& v) {
			this->moves.emplace_back("O-O");
		}
		void operator()(castles_queenside const& v) {
			this->moves.emplace_back("O-O-O");
		}
		void operator()(pawn_move const& v) {
			this->moves.emplace_back(std::string(v.begin(), v.end()));
		}
		void operator()(pawn_capture const& v) {
			this->moves.emplace_back(std::string(v.begin(), v.end()));
		}
		void operator()(pawn_capture_ep const& v) {
			this->moves.emplace_back(std::string(v.begin(), v.end()));
		}
		void operator()(pawn_promotion const& v) {
			this->moves.emplace_back(std::string(v.begin(), v.end()));
		}
		void operator()(pawn_capture_promotion const& v) {
			this->moves.emplace_back(std::string(v.begin(), v.end()));
		}
		void operator()(piece_move const& v) {
			this->moves.emplace_back(std::string(v.begin(), v.end()));
		}
		void operator()(piece_capture const& v) {
			this->moves.emplace_back(std::string(v.begin(), v.end()));
		}
		void operator()(explicit_piece_move const& v) {
			this->moves.emplace_back(std::string(v.begin(), v.end()));
		}

		std::vector<chess::move> moves;
	};
}


namespace pgn_parser {
	namespace x3 = boost::spirit::x3;
	using x3::lit;
	using x3::int_;
	using x3::char_;
	using x3::lexeme;
	using x3::alnum;

	const auto file = char_("a-h");
	const auto rank = char_("1-8");
	const auto square = file >> rank;
	const auto rank8 = char_('8');
	const auto takes = char_('x');
	const auto en_passant = ((char_('6') | char_('3')) >> "ep");
	const auto officer = (char_('B') | char_('N') | char_('Q') | char_('R'));
	const auto king = char_('K');
	const auto quoted_string = lexeme['"' >> *(char_ - '"') >> '"'];

	x3::rule<struct tag, ast::tag> tag = "tag";
	x3::rule<struct comment_class, ast::comment> comment = "comment";

	x3::rule<struct move_num, int> move_num = "move_num";
	x3::rule<struct pawn_move, ast::pawn_move> pawn_move = "pawn_move";
	x3::rule<struct pawn_capture, ast::pawn_capture> pawn_capture = "pawn_capture";
	x3::rule<struct pawn_capture_ep, ast::pawn_capture_ep> pawn_capture_ep = "pawn_capture_ep";
	x3::rule<struct pawn_promotion, ast::pawn_promotion> pawn_promotion = "pawn_promotion";
	x3::rule<struct pawn_capture_promotion, ast::pawn_capture_promotion> pawn_capture_promotion = "pawn_capture_promotion";
	x3::rule<struct piece_move, ast::piece_move> piece_move = "piece_move";
	x3::rule<struct piece_capture, ast::piece_capture> piece_capture = "piece_capture";
	x3::rule<struct explicit_piece_move, ast::explicit_piece_move> explicit_piece_move = "explicit_piece_move";
	x3::rule<struct castles_kingside, ast::castles_kingside> castles_kingside = "castles_kingside";
	x3::rule<struct castles_queenside, ast::castles_queenside> castles_queenside = "castles_queenside";

	x3::rule<struct value, ast::value> value = "value";
	x3::rule<struct game, std::vector<ast::value>> game = "game";

	const auto tag_def = lexeme['[' >> +(char_ - ']') >> ']'];
	const auto comment_def = lexeme['{' >> +(char_ - '}') >> '}'];

	const auto move_num_def = int_ >> ".";
	const auto pawn_move_def = square >> -char_('+');
	const auto pawn_capture_def = file >> takes >> square >> -char_('+');
	const auto pawn_capture_ep_def = file >> takes >> file >> en_passant >> -char_('+');
	const auto pawn_promotion_def = file >> rank8 >> officer >> -char_('+');
	const auto pawn_capture_promotion_def = file >> takes >> rank8 >> officer >> -char_('+');
	const auto piece_move_def = (king | officer) >> square >> -char_('+');
	const auto piece_capture_def = (king | officer) >> takes >> square >> -char_('+');
	const auto explicit_piece_move_def = officer >> (file | rank) >> square >> -char_('+');
	const auto castles_kingside_def = +lit("O-O") >> -char_('+');
	const auto castles_queenside_def = +lit("O-O-O") >> -char_('+');

	const auto game_def = *value;

	const auto value_def
		= tag
		| comment
		| move_num
		| pawn_move
		| pawn_capture
		| pawn_capture_ep
		| pawn_promotion
		| pawn_capture_promotion
		| piece_move
		| piece_capture
		| explicit_piece_move
		| castles_kingside
		| castles_queenside;

	BOOST_SPIRIT_DEFINE(
		value,
		tag,
		comment,
		move_num,
		pawn_move,
		pawn_capture,
		pawn_capture_ep,
		pawn_promotion,
		pawn_capture_promotion,
		piece_move,
		piece_capture,
		explicit_piece_move,
		castles_kingside,
		castles_queenside,
		game);

	const auto entry_point = x3::skip(x3::space)[game];
}

bool read(std::string game)
{
	std::vector<ast::value> val;
	auto begin = game.begin();
	bool ok = parse(begin, game.end(), pgn_parser::entry_point, val);

	if (!ok) {
		std::cerr << "parse failed: remaining input:\n" << std::string(begin, game.end());
	}

	ast::visitor visitor;
	for (auto v : val) {
		std::visit(visitor, v);
	}

	for (auto m : visitor.moves) {
		std::cout << m.description << std::endl;
	}

	return ok;
}
