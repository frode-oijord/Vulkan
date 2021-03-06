
#include <Scheme.h>
#include <iostream>
#include <string>


scm::env_ptr env = scm::global_env();


std::string repl(std::string input)
{
	std::any exp = scm::read(input.begin(), input.end());
	exp = scm::eval(exp, env);
	std::stringstream ss;
	scm::print(exp, ss);
	return ss.str();
}

#define GREEN(__text__) "\033[1;32m" + std::string(__text__) + "\033[0m"
#define RED(__text__) "\033[1;31m" + std::string(__text__) + "\033[0m"

#define TEST(__exp__, __ev__)												\
	std::string __v__ = repl(__exp__);										\
	std::cout << __exp__ << " => " << __v__ << " ";							\
	bool passed = (__ev__ == __v__);										\
	std::cout << (passed ? GREEN("(Pass)") : RED("(Fail)")) << std::endl;	\
	return passed;

typedef std::function<bool()> test_case;

std::vector<test_case> tests
{
	[] { TEST("(quote ())", "()"); },
	[] { TEST("(begin (define a 1) (+ 1 2 3))", "6"); },
	[] { TEST("a", "1"); },
	[] { TEST("(quote (testing 1 (2) -3.14e+159))", "(testing 1 (2) -3.14e+159)"); },
	[] { TEST("(+ 2 2)", "4"); },
	[] { TEST("(+ (* 2 100) (* 1 10))", "210"); },
	[] { TEST("(if (> 6 5) (+ 1 1) (+ 2 2))", "2"); },
	[] { TEST("(if (< 6 5) (+ 1 1) (+ 2 2))", "4"); },
	[] { TEST("(define x 3)", "3"); },
	[] { TEST("x", "3"); },
	[] { TEST("(+ x x)", "6"); },
	[] { TEST("((lambda (x) (+ x x)) 5)", "10"); },
	[] { TEST("(define twice (lambda (x) (* 2 x)))", "struct scm::Function"); },
	[] { TEST("(twice 5)", "10"); },
	[] { TEST("(define compose (lambda (f g) (lambda (x) (f (g x)))))", "struct scm::Function"); },
	[] { TEST("((compose list twice) 5)", "(10)"); },
	[] { TEST("(define repeat (lambda (f) (compose f f)))", "struct scm::Function"); },
	[] { TEST("((repeat twice) 5)", "20"); },
	[] { TEST("((repeat (repeat twice)) 5)", "80"); },
	[] { TEST("(define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1))))))", "struct scm::Function"); },
	[] { TEST("(fact 3)", "6"); },
	[] { TEST("(fact 50)", "3.04141e+64"); },
	[] { TEST("(define abs (lambda (n) ((if (> n 0) + -) 0 n)))", "struct scm::Function"); },
	[] { TEST("(list (abs -3) (abs 0) (abs 3))", "(3 0 3)"); },
	[] {
		std::string input = R"(
(define combine (lambda (f)
	(lambda (x y)
		(if (null ? x) (quote ())
			(f (list (car x) (car y))
				((combine f) (cdr x) (cdr y)))))))
			)";
		TEST(input, "struct scm::Function");
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
