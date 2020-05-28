#include <Window.h>
#include <Innovator/File.h>
#include <Innovator/Factory.h>
#include <Innovator/Nodes.h>

#include <iostream>
#include <vector>


int main(int argc, char *argv[])
{
  try {
    VulkanImageFactory::Register<GliTextureImage>();
	//auto test = eval_file("world/world.scm");

    //VulkanImageFactory::Register<DebugTextureImage>();
    //auto scene = eval_file("sparse3d/sparse3d.scm");
    auto scene = eval_file("crate/crate.scm");

    auto window = std::any_cast<std::shared_ptr<VulkanWindow>>(scene);
    return window->show();
  }
  catch (std::exception & e) {
    std::cerr << std::string("caught exception in main(): ") + typeid(e).name() << std::endl;
    std::cerr << std::string("message: ") + e.what() << std::endl;
  }
  return 1;
}
