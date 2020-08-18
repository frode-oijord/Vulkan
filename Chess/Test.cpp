
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
		return read("a4");
	},
	[] { // pawn capture
		return read("axb3");
	},
	[] { // pawn capture en passant
		return read("axb6ep");
	},
	[] { // pawn promotion
		return read("c8Q");
	},
	[] { // piece move
		return read("Na3");
	},
	[] { // piece move
		return read("Qh5");
	},
	[] { // piece capture
		return read("Qxh5");
	},
	[] { // piece capture
		return read("Nxa3");
	},
	[] { // explicit piece move
		return read("Nba3");
	},
	[] { // explicit piece move
		return read("N4a3");
	},
	[] { // game
		return read("1. e4 e5 2. Nf3 Nc6 3. Bb5 a6");
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
