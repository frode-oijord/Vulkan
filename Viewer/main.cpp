#include <Window.h>
#include <Innovator/File.h>
#include <Innovator/Nodes.h>
#include <Innovator/Factory.h>

#include <iostream>
#include <vector>

int main(int argc, char *argv[])
{
  try {
    VulkanImageFactory::Register<GliTextureImage>();

    std::vector<const char *> instance_layers{
#ifdef DEBUG
      "VK_LAYER_LUNARG_standard_validation",
#endif
    };

    std::vector<const char *> instance_extensions{
      VK_KHR_SURFACE_EXTENSION_NAME,
      VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#ifdef DEBUG
      VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
    };

    auto vulkan = std::make_shared<VulkanInstance>(
      "Innovator",
      instance_layers,
      instance_extensions);

#ifdef DEBUG
    auto debugcb = std::make_unique<VulkanDebugCallback>(
      vulkan,
      VK_DEBUG_REPORT_WARNING_BIT_EXT |
      VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
      VK_DEBUG_REPORT_ERROR_BIT_EXT);
#endif

	std::vector<const char *> device_layers{
#ifdef DEBUG
      "VK_LAYER_LUNARG_standard_validation",
#endif
    };

    std::vector<const char *> device_extensions{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkPhysicalDeviceFeatures device_features;
    ::memset(&device_features, VK_FALSE, sizeof(VkPhysicalDeviceFeatures));

    auto device = std::make_shared<VulkanDevice>(
      vulkan,
      device_features,
      device_layers,
      device_extensions);

    auto scene = std::make_shared<Group>();
    scene->children = {
      eval_file("crate/crate.scm")
    };

    VulkanWindow window(vulkan, device, scene);
    return window.show();
  }
  catch (std::exception & e) {
    std::cerr << std::string("caught exception in main(): ") + typeid(e).name() << std::endl;
  }
  return 1;
}
