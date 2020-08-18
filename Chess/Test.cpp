
#include <Chess.h>

#include <iostream>
#include <string>
#include <vector>
#include <functional>


#define GREEN(__text__) "\033[1;32m" + std::string(__text__) + "\033[0m"
#define RED(__text__) "\033[1;31m" + std::string(__text__) + "\033[0m"

typedef std::function<bool()> test_case;

std::vector<test_case> tests
{
	[] { // pawn move
			std::string move("a4");
			ast::move value;
			bool passed = parse(move.begin(), move.end(), parser::entry_point, value);
			std::visit(ast::visitor, value);
			return passed;
	},
	[] { // pawn capture
			std::string move("axb3");
			ast::move value;
			bool passed = parse(move.begin(), move.end(), parser::entry_point, value);
			std::visit(ast::visitor, value);
			return passed;
	},
	[] { // pawn capture en passant
			std::string move("axb6ep");
			ast::move value;
			bool passed = parse(move.begin(), move.end(), parser::entry_point, value);
			std::visit(ast::visitor, value);
			return passed;
	},
	[] { // pawn promotion
			std::string move("c8Q");
			ast::move value;
			bool passed = parse(move.begin(), move.end(), parser::entry_point, value);
			std::visit(ast::visitor, value);
			return passed;
	},
	[] { // piece move
			std::string move("Na3");
			ast::move value;
			bool passed = parse(move.begin(), move.end(), parser::entry_point, value);
			std::visit(ast::visitor, value);
			return passed;
	},
	[] { // piece move
			std::string move("Qh5");
			ast::move value;
			bool passed = parse(move.begin(), move.end(), parser::entry_point, value);
			std::visit(ast::visitor, value);
			return passed;
	},
	[] { // piece capture
			std::string move("Qxh5");
			ast::move value;
			bool passed = parse(move.begin(), move.end(), parser::entry_point, value);
			std::visit(ast::visitor, value);
			return passed;
	},
	[] { // piece capture
			std::string move("Nxa3");
			ast::move value;
			bool passed = parse(move.begin(), move.end(), parser::entry_point, value);
			std::visit(ast::visitor, value);
			return passed;
	},
	[] { // explicit piece move
			std::string move("Nba3");
			ast::move value;
			bool passed = parse(move.begin(), move.end(), parser::entry_point, value);
			std::visit(ast::visitor, value);
			return passed;
	},
	[] { // explicit piece move
			std::string move("N4a3");
			ast::move value;
			bool passed = parse(move.begin(), move.end(), parser::entry_point, value);
			std::visit(ast::visitor, value);
			return passed;
	},

};


int main(int, char* [])
{
	std::vector<bool> results;

	for (auto test : tests) {
		results.push_back(test());
	}

	size_t n_exec = results.size();
	size_t n_fail = std::count(results.begin(), results.end(), false);
	size_t n_pass = std::count(results.begin(), results.end(), true);

	std::cout << std::endl << GREEN(std::to_string(n_exec) + " tests executed. ");

	if (n_fail > 0) {
		std::cout << RED(std::to_string(n_fail) + " tests failed. ") << std::endl;
	}
	else {
		std::cout << GREEN("All tests passed.") << std::endl;
	}

	return 1;
}
