
#include <r2.h>
#include <iostream>
#include <string>
#include <functional>


#define GREEN(__text__) "\033[1;32m" + std::string(__text__) + "\033[0m"
#define RED(__text__) "\033[1;31m" + std::string(__text__) + "\033[0m"

typedef std::function<bool()> test_case;

std::vector<test_case> tests
{
	[] {
		std::string result = to_string(e1 * e1);
		std::cout << "e1 * e1: " << result << std::endl;
		return result == "1";
	},
	[] {
		std::string result = to_string(e2 * e2);
		std::cout << "e2 * e2: " << result << std::endl;
		return result == "1";
	},
	[] {
		std::string result = to_string(e1 * e2);
		std::cout << "e1 * e2: " << result << std::endl;
		return result == "e12";
	},
	[] {
		std::string result = to_string(e2 * e1);
		std::cout << "e2 * e1: " << result << std::endl;
		return result == "-e12";
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
