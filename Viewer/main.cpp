#include <Window.h>
#include <Innovator/ScmEnv.h>
#include <Innovator/Factory.h>
#include <Innovator/Nodes.h>

#include <boost/program_options.hpp>

#include <iostream>
#include <vector>
#include <string>

namespace bpo = boost::program_options;

static void repl(std::string input, scm::env_ptr env)
{
	scm::print(scm::eval(scm::read(input.begin(), input.end()), env), std::cout);
	std::cout << std::endl;
}

int main(int argc, char* argv[])
{
	bpo::options_description options("Allowed options");
	bpo::variables_map vm;

	options.add_options()
		("help", "produce help message")
		("exec,c", bpo::value<std::string>(), "execute")
		("file,f", bpo::value<std::string>(), "input file");

	bpo::positional_options_description positional_options;
	positional_options.add("file", -1);

	bpo::store(bpo::command_line_parser(argc, argv).options(options).positional(positional_options).run(), vm);
	bpo::notify(vm);

	try {
		//VulkanImageFactory::Register<GliTextureImage>();
		VulkanImageFactory::Register<DebugTextureImageBricked>();
		scm::env_ptr global_env = scm::global_env();
		global_env->outer = innovator_env();

		if (vm.count("exec")) {
			std::string code = vm["exec"].as<std::string>();
			repl(code, global_env);
			return EXIT_SUCCESS;
		}

		if (vm.count("file")) {
			std::string file = vm["file"].as<std::string>();
			std::ifstream stream(file, std::ios::in);
			const std::string code{
			  std::istreambuf_iterator<char>(stream),
			  std::istreambuf_iterator<char>()
			};
			repl(code, global_env);
			return EXIT_SUCCESS;
		}

		std::cout << "Innovator Scheme REPL" << std::endl;
		while (true) {
			std::string input;
			std::cout << "> ";
			std::getline(std::cin, input);

			repl(input, global_env);
		}
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
