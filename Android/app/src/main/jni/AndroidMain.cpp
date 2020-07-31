// Copyright 2016 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <android/log.h>
#include <android_native_app_glue.h>

#include <Innovator/ScmEnv.h>
#include <Innovator/VulkanSurface.h>

static std::shared_ptr<Separator> root;

void init(android_app* app)
{
  const std::string code = R"(
  (renderpass
    (renderpass-description
      (renderpass-attachment
        VK_FORMAT_R8G8B8A8_UNORM
        VK_SAMPLE_COUNT_1_BIT
        VK_ATTACHMENT_LOAD_OP_CLEAR
        VK_ATTACHMENT_STORE_OP_STORE
        VK_ATTACHMENT_LOAD_OP_DONT_CARE
        VK_ATTACHMENT_STORE_OP_DONT_CARE
        VK_IMAGE_LAYOUT_UNDEFINED
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)

      (renderpass-attachment
        VK_FORMAT_D32_SFLOAT
        VK_SAMPLE_COUNT_1_BIT
        VK_ATTACHMENT_LOAD_OP_CLEAR
        VK_ATTACHMENT_STORE_OP_STORE
        VK_ATTACHMENT_LOAD_OP_DONT_CARE
        VK_ATTACHMENT_STORE_OP_DONT_CARE
        VK_IMAGE_LAYOUT_UNDEFINED
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)

      (subpass
        (pipeline-bindpoint VK_PIPELINE_BIND_POINT_GRAPHICS)
        (color-attachment (uint32 0) VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        (depth-attachment (uint32 1) VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)))

      (framebuffer
        (framebuffer-attachment
          VK_FORMAT_R8G8B8A8_UNORM
          (imageusageflags
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT
            VK_IMAGE_USAGE_SAMPLED_BIT)
          (imageaspectflags VK_IMAGE_ASPECT_COLOR_BIT))

        (framebuffer-attachment
          VK_FORMAT_D32_SFLOAT
          (imageusageflags VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
          (imageaspectflags VK_IMAGE_ASPECT_DEPTH_BIT))))
  )";


  scm::env_ptr global_env = scm::global_env();
  global_env->outer = innovator_env();

  std::any exp = scm::read(code.begin(), code.end());
  exp = scm::eval(exp, global_env);

  auto scene = std::any_cast<std::shared_ptr<Node>>(exp);

  devicevisitor.visit(scene.get());

  devicevisitor.instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  devicevisitor.instance_extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
  devicevisitor.device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  auto vulkan = std::make_shared<VulkanInstance>(
          "Innovator",
          devicevisitor.instance_layers,
          devicevisitor.instance_extensions);

  auto device = std::make_shared<VulkanDevice>(
          vulkan,
          devicevisitor.device_features2,
          devicevisitor.device_layers,
          devicevisitor.device_extensions);

  auto surface = std::make_shared<VulkanSurface>(
          vulkan,
          app->window);

  auto swapchain = std::make_shared<SwapchainObject>(
          surface,
          VK_PRESENT_MODE_FIFO_KHR);

  VkSurfaceCapabilitiesKHR surface_capabilities = surface->getSurfaceCapabilities(device);
  auto extent = std::make_shared<VkExtent2D>(surface_capabilities.currentExtent);

  InitVisitors(vulkan, device, extent);

  root = std::make_shared<Separator>();
  root->children = {
    scene,
    swapchain
  };

  allocvisitor.visit(root.get());
  pipelinevisitor.visit(root.get());
  recordvisitor.visit(root.get());
}

// Process the next main command.
void handle_cmd(android_app* app, int32_t cmd) {
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      // The window is being shown, get it ready.
      init(app);
      break;
    case APP_CMD_TERM_WINDOW:
      // The window is being hidden or closed, clean it up.
      // DeleteVulkan();
      break;
    default:
      __android_log_print(ANDROID_LOG_INFO, "Vulkan Tutorials",
                          "event not handled: %d", cmd);
  }
}

void android_main(struct android_app* app) {

  // Set the callback to process system events
  app->onAppCmd = handle_cmd;

  // Used to poll the events in the main loop
  int events;
  android_poll_source* source;

  // Main loop
  do {
    if (ALooper_pollAll(root ? 1 : 0, nullptr,
                        &events, (void**)&source) >= 0) {
      if (source != NULL) source->process(app, source);
    }

    // render if vulkan is ready
    if (root) {
      rendervisitor.visit(root.get());
      presentvisitor.visit(root.get());
    }
  } while (app->destroyRequested == 0);
}
