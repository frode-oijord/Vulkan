#include <boost/fusion/adapted.hpp>
#include <boost/spirit/home/x3.hpp>

#include <vector>
#include <string>
#include <variant>
#include <iostream>

namespace ast {
	struct pawn_move : public std::string {};
	struct pawn_capture : public std::string {};
	struct pawn_capture_ep : public std::string {};
	struct pawn_promotion : public std::string {};
	struct pawn_capture_promotion : public std::string {};
	struct piece_move : public std::string {};
	struct piece_capture : public std::string {};
	struct explicit_piece_move : public std::string {};

	typedef std::variant<
		pawn_move,
		pawn_capture,
		pawn_capture_ep,
		pawn_promotion,
		pawn_capture_promotion,
		piece_move,
		piece_capture,
		explicit_piece_move> move_t;

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
	using x3::lit;
	using x3::char_;
	using x3::lexeme;
	using x3::alnum;

	const auto file = char_('a', 'h');
	const auto rank = char_('1', '8');
	const auto square = file >> rank;
	const auto rank8 = char_('8');
	const auto takes = char_('x');
	const auto en_passant = (char_('6') | char_('3')) >> lit("ep");
	const auto officer = (char_('B') | char_('N') | char_('Q') | char_('R'));
	const auto king = char_('K');

	// pawn moves
	x3::rule<struct pawn_move_class, ast::pawn_move> pawn_move = "pawn_move";
	x3::rule<struct pawn_capture_class, ast::pawn_capture> pawn_capture = "pawn_capture";
	x3::rule<struct pawn_capture_ep_class, ast::pawn_capture_ep> pawn_capture_ep = "pawn_capture_ep";
	x3::rule<struct pawn_promotion_class, ast::pawn_promotion> pawn_promotion = "pawn_promotion";
	x3::rule<struct pawn_capture_promotion_class, ast::pawn_capture_promotion> pawn_capture_promotion = "pawn_capture_promotion";

	const auto pawn_move_def = lexeme[square >> !alnum];
	const auto pawn_capture_def = lexeme[file >> takes >> square >> !alnum];
	const auto pawn_capture_ep_def = lexeme[file >> takes >> file >> en_passant >> !alnum];
	const auto pawn_promotion_def = lexeme[file >> rank8 >> officer >> !alnum];
	const auto pawn_capture_promotion_def = lexeme[file >> takes >> rank8 >> officer >> !alnum];

	BOOST_SPIRIT_DEFINE(pawn_move, pawn_capture, pawn_capture_ep, pawn_promotion, pawn_capture_promotion);

	// piece moves
	x3::rule<struct piece_move_class, ast::piece_move> piece_move = "piece_move";
	x3::rule<struct piece_capture_class, ast::piece_capture> piece_capture = "piece_capture";
	x3::rule<struct explicit_piece_move_class, ast::explicit_piece_move> explicit_piece_move = "explicit_piece_move";

	const auto piece_move_def = lexeme[(king | officer) >> file >> rank >> !alnum];
	const auto piece_capture_def = lexeme[(king | officer) >> takes >> file >> rank >> !alnum];
	const auto explicit_piece_move_def = lexeme[officer >> (file | rank) >> file >> rank >> !alnum];

	BOOST_SPIRIT_DEFINE(piece_move, piece_capture, explicit_piece_move);

	x3::rule<struct move_class, ast::move> move = "move";
	const auto move_def
		= pawn_move
		| pawn_capture
		| pawn_capture_ep
		| pawn_promotion
		| pawn_capture_promotion
		| piece_move
		| piece_capture
		| explicit_piece_move;

	BOOST_SPIRIT_DEFINE(move);

	const auto entry_point = x3::skip(x3::space)[move];
}
