
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


#define ASSERT(__exp__, __ev__)											\
	std::string __v__ = repl(__exp__);									\
	std::cout << __exp__ << " => " << __v__ << " ";						\
	bool passed = (__ev__ == __v__);									\
	std::cout << (passed ? "(Pass)" : "(Fail)") << std::endl;			\
	return passed;


std::vector<std::function<bool()>> tests
{
	[] { ASSERT("(+ 2 2)", "4"); },
	[] { ASSERT("(+ (* 2 100) (* 1 10))", "210"); },
	[] { ASSERT("(if (> 6 5) (+ 1 1) (+ 2 2))", "2"); },
	[] { ASSERT("(if (< 6 5) (+ 1 1) (+ 2 2))", "4"); },
	[] { ASSERT("(define x 3)", "3"); },
	[] { ASSERT("x", "3"); },
	[] { ASSERT("(+ x x)", "6"); },
	[] { ASSERT("((lambda (x) (+ x x)) 5)", "10"); },
	[] { ASSERT("(define twice (lambda (x) (* 2 x)))", "struct scm::Function"); },
	[] { ASSERT("(twice 5)", "10"); },
	[] { ASSERT("(define compose (lambda (f g) (lambda (x) (f (g x)))))", "struct scm::Function"); },
	[] { ASSERT("((compose list twice) 5)", "(10)"); },
	[] { ASSERT("(define repeat (lambda (f) (compose f f)))", "struct scm::Function"); },
	[] { ASSERT("((repeat twice) 5)", "20"); },
	[] { ASSERT("((repeat (repeat twice)) 5)", "80"); },
	[] { ASSERT("(define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1))))))", "struct scm::Function"); },
	[] { ASSERT("(fact 3)", "6"); },
	[] { ASSERT("(fact 50)", "3.04141e+64"); },
	[] { ASSERT("(define abs (lambda (n) ((if (> n 0) + -) 0 n)))", "struct scm::Function"); },
	[] { ASSERT("(list (abs -3) (abs 0) (abs 3))", "(3 0 3)"); },
	[] { 
		std::string input = R"(
			(define combine (lambda (f)
				(lambda (x y)
					(if (null ? x) (quote ())
						(f (list (car x) (car y))
							((combine f) (cdr x) (cdr y)))))))
						)";
		ASSERT(input, "struct scm::Function");
	},

};


int main(int, char* [])
{
	std::vector<bool> results(tests.size());
	std::transform(tests.begin(), tests.end(), results.begin(), [](std::function<bool()> test) {
		return test();
	});

	size_t n_exec = results.size();
	size_t n_fail = std::count(results.begin(), results.end(), false);
	size_t n_pass = std::count(results.begin(), results.end(), true);

	std::cout << std::endl << n_exec << " tests executed. ";
	std::cout << "(" << n_pass << " tests passed and ";
	std::cout << n_fail << " tests failed) " << std::endl;

	return 1;
}
