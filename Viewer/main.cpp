#include <Window.h>
#include <Innovator/ScmEnv.h>
#include <Innovator/Factory.h>
#include <Innovator/Nodes.h>

#include <iostream>
#include <vector>
#include <string>

int main(int argc, char* argv[])
{
	try {
		VulkanImageFactory::Register<GliTextureImage>();
		//VulkanImageFactory::Register<DebugTextureImage>();
		scm::env_ptr global_env = scm::global_env();
		global_env->outer = innovator_env();

		if (argc > 1) {
			std::string input = argv[1];

			std::ifstream stream(input, std::ios::in);
			if (stream.is_open()) {
				const std::string code{
					std::istreambuf_iterator<char>(stream),
					std::istreambuf_iterator<char>()
				};
				std::any exp = scm::read(code.begin(), code.end());
				exp = scm::eval(exp, global_env);
				auto window = std::any_cast<std::shared_ptr<VulkanWindow>>(exp);
				return window->show();
			}
			else {
				std::any exp = scm::read(input.begin(), input.end());
				exp = scm::eval(exp, global_env);
				scm::print(exp);
				std::cout << std::endl;
			}
			return 1;
		}

		std::cout << "Innovator Scheme REPL" << std::endl;

		while (true) {
			std::string input;
			std::cout << "> ";
			std::getline(std::cin, input);

			std::any exp = scm::read(input.begin(), input.end());
			exp = scm::eval(exp, global_env);
			scm::print(exp);
			std::cout << std::endl;
		}
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	return 1;
}
