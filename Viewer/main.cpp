#include <Window.h>
#include <Innovator/File.h>
#include <Innovator/Factory.h>
#include <Innovator/Nodes.h>

#include <iostream>
#include <vector>


int main(int argc, char *argv[])
{
  try {
    VulkanImageFactory::Register<DebugTextureImage>();

		auto test = eval_file("crate/crate3.scm");
		auto window = std::any_cast<std::shared_ptr<VulkanWindow>>(test);
    return window->show();
  }
  catch (std::exception & e) {
    std::cerr << std::string("caught exception in main(): ") + typeid(e).name() << std::endl;
    std::cerr << std::string("message: ") + e.what() << std::endl;
  }
  return 1;
}
