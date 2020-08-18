#include <boost/fusion/adapted.hpp>
#include <boost/spirit/home/x3.hpp>

#include <vector>
#include <string>
#include <variant>
#include <iostream>

namespace ast {
	struct pawn_move : public std::vector<char> {};
	struct pawn_capture : public std::vector<char> {};
	struct pawn_capture_ep : public std::vector<char> {};
	struct pawn_promotion : public std::vector<char> {};
	struct pawn_capture_promotion : public std::vector<char> {};
	struct piece_move : public std::vector<char> {};
	struct piece_capture : public std::vector<char> {};
	struct explicit_piece_move : public std::vector<char> {};

	typedef std::variant<
		int,
		pawn_move,
		pawn_capture,
		pawn_capture_ep,
		pawn_promotion,
		pawn_capture_promotion,
		piece_move,
		piece_capture,
		explicit_piece_move> value;

	struct {
		void operator()(int const& v) const {
			std::cout << "move " << v << ": ";
		}
		void operator()(pawn_move const& v) const {
			std::cout << "Pawn moved to " << std::string(v.begin(), v.end()) << std::endl;
		}
		void operator()(pawn_capture const& v) const {
			std::cout << "Pawn captured on " << std::string(v.begin(), v.end()) << std::endl;
		}
		void operator()(pawn_capture_ep const& v) const {
			std::cout << "Pawn captured on " << std::string(v.begin(), v.end()) << " with en passant" << std::endl;
		}
		void operator()(pawn_promotion const& v) const {
			std::cout << "Pawn promoted to " << std::string(v.begin(), v.end()) << std::endl;
		}
		void operator()(pawn_capture_promotion const& v) const {
			std::cout << "Pawn captured and promoted to " << std::string(v.begin(), v.end()) << std::endl;
		}
		void operator()(piece_move const& v) const {
			std::cout << "Piece move " << std::string(v.begin(), v.end()) << std::endl;
		}
		void operator()(piece_capture const& v) const {
			std::cout << "Piece captured " << std::string(v.begin(), v.end()) << std::endl;
		}
		void operator()(explicit_piece_move const& v) const {
			std::cout << "Piece moved explicitly " << std::string(v.begin(), v.end()) << std::endl;
		}
	} visitor;
}


namespace parser {
	namespace x3 = boost::spirit::x3;
	using x3::lit;
	using x3::int_;
	using x3::char_;
	using x3::lexeme;
	using x3::alnum;

	const auto file = char_('a', 'h');
	const auto rank = char_('1', '8');
	const auto square = file >> rank;
	const auto rank8 = char_('8');
	const auto takes = char_('x');
	const auto en_passant = ((char_('6') | char_('3')) >> lit("ep"));
	const auto officer = (char_('B') | char_('N') | char_('Q') | char_('R'));
	const auto king = char_('K');

	x3::rule<struct move_num_class, int> move_num = "move_num";
	x3::rule<struct pawn_move_class, ast::pawn_move> pawn_move = "pawn_move";
	x3::rule<struct pawn_capture_class, ast::pawn_capture> pawn_capture = "pawn_capture";
	x3::rule<struct pawn_capture_ep_class, ast::pawn_capture_ep> pawn_capture_ep = "pawn_capture_ep";
	x3::rule<struct pawn_promotion_class, ast::pawn_promotion> pawn_promotion = "pawn_promotion";
	x3::rule<struct pawn_capture_promotion_class, ast::pawn_capture_promotion> pawn_capture_promotion = "pawn_capture_promotion";
	x3::rule<struct piece_move_class, ast::piece_move> piece_move = "piece_move";
	x3::rule<struct piece_capture_class, ast::piece_capture> piece_capture = "piece_capture";
	x3::rule<struct explicit_piece_move_class, ast::explicit_piece_move> explicit_piece_move = "explicit_piece_move";
	x3::rule<struct value_class, ast::value> value = "value";
	x3::rule<struct game_class, std::vector<ast::value>> game = "game";

	const auto move_num_def = int_ >> lit(".");
	const auto pawn_move_def = square;
	const auto pawn_capture_def = file >> takes >> square;
	const auto pawn_capture_ep_def = file >> takes >> file >> en_passant;
	const auto pawn_promotion_def = file >> rank8 >> officer;
	const auto pawn_capture_promotion_def = file >> takes >> rank8 >> officer;
	const auto piece_move_def = (king | officer) >> file >> rank;
	const auto piece_capture_def = (king | officer) >> takes >> file >> rank;
	const auto explicit_piece_move_def = officer >> (file | rank) >> file >> rank;
	const auto game_def = value % x3::space;

	const auto value_def
		= move_num
		| pawn_move
		| pawn_capture
		| pawn_capture_ep
		| pawn_promotion
		| pawn_capture_promotion
		| piece_move
		| piece_capture
		| explicit_piece_move;

	BOOST_SPIRIT_DEFINE(value, move_num, pawn_move, pawn_capture, pawn_capture_ep, pawn_promotion, pawn_capture_promotion, piece_move, piece_capture, explicit_piece_move, game);
	
	const auto entry_point = game;
}

bool read(std::string game)
{
	std::vector<ast::value> val;
	bool ok = parse(game.begin(), game.end(), parser::entry_point, val);

	for (auto v : val) {
		std::visit(ast::visitor, v);
	}
	return ok;
}
