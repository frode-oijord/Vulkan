
#include <Scheme.h>
#include <iostream>
#include <string>


Scheme scheme;

struct test_case {
	std::string name;
	std::function<bool()> run;
};


template <typename T>
bool ASSERT_EQUAL(std::any exp, T value)
{
	std::cout << "= " << value << ": ";
	return (exp.type() == typeid(T)) && (std::any_cast<T>(exp) == value);
}


std::vector<test_case> tests{
	{ "(+ 1 1)", [] {
		return ASSERT_EQUAL(scheme.eval("(+ 1 1)"), 2.0);
	}}, 
	{ "(define a 2)", [] {
		return ASSERT_EQUAL(scheme.eval("(define a 2)"), 2.0);
	}}, 
	{ "a", [] {
		return ASSERT_EQUAL(scheme.eval("a"), 2.0);
	}},
};


int main(int, char* [])
{
	std::vector<bool> results(tests.size());
	std::transform(tests.begin(), tests.end(), results.begin(), [](test_case& test) {
		std::cout << test.name << " ";
		bool success = test.run();
		std::cout << (success ? "Passed" : "Failed") << std::endl;
		return success;
		});

	std::cout << std::endl << results.size() << " tests executed. " << std::endl;
	std::cout << std::count(results.begin(), results.end(), true) << " tests succeeded. " << std::endl;
	std::cout << std::count(results.begin(), results.end(), false) << " tests failed. " << std::endl;

	return 1;
}
