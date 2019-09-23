
#include <Scheme.h>

#include <iostream>
#include <string>

int main(int, char**)
{
  std::cout << "Innovator Scheme REPL" << std::endl;
  scm::env_ptr env = scm::global_env();

  while (true) {
    try {
      std::cout << "> ";
      std::string input;
      std::getline(std::cin, input);

      std::any exp = scm::read(input.begin(), input.end());
      exp = scm::eval(exp, env);
      scm::print(exp); std::cout << std::endl;
    }
    catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }
  return 1;
}
