#pragma once

#include <Innovator/Defines.h>
#include <Innovator/Node.h>
#include <Innovator/State.h>
#include <Innovator/VulkanObjects.h>

#include <map>
#include <memory>
#include <utility>
#include <vector>
#include <fstream>
#include <iostream>
#include <functional>

template <typename T>
std::shared_ptr<T> find_first(std::shared_ptr<Node> root)
{
  auto node = std::dynamic_pointer_cast<T>(root);
  if (node) {
    return node;
  }

  auto group = std::dynamic_pointer_cast<Group>(root);
  if (group) {
    for (auto node : group->children) {
      auto first = find_first<T>(node);
      if (first) {
        return first;
      }
    }
  }
  return node;
}

class SceneRenderer {
public:
  NO_COPY_OR_ASSIGNMENT(SceneRenderer)
  SceneRenderer() = delete;
  ~SceneRenderer() = default;

  explicit SceneRenderer(std::shared_ptr<VulkanInstance> vulkan,
                         std::shared_ptr<VulkanDevice> device,
                         VkExtent2D extent) :
    vulkan(std::move(vulkan)),
    device(std::move(device)),
    extent(extent)
  {}

  std::shared_ptr<VulkanInstance> vulkan;
  std::shared_ptr<VulkanDevice> device;
  RenderState state;
  VulkanCommandBuffers* command{ nullptr };
  VkExtent2D extent;  
};

struct TraversalContext {
  TraversalContext(std::shared_ptr<VulkanInstance> vulkan,
                   std::shared_ptr<VulkanDevice> device,
                   VkExtent2D extent) :
    vulkan(std::move(vulkan)),
    device(std::move(device)),
    extent(extent),
    fence(std::make_unique<VulkanFence>(this->device)),
    command(std::make_unique<VulkanCommandBuffers>(this->device)),
    pipelinecache(std::make_shared<VulkanPipelineCache>(this->device))
  {
    this->queue = this->device->getQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
  }

  virtual ~TraversalContext()
  {
    try {
      THROW_ON_ERROR(vkDeviceWaitIdle(this->device->device));
    }
    catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }

  void resize(VkExtent2D extent)
  {
    // make sure all work submitted is done before we start recreating stuff
    THROW_ON_ERROR(vkDeviceWaitIdle(this->device->device));
    this->extent = extent;
  }

  void begin_alloc()
  {
    this->imageobjects.clear();
    this->bufferobjects.clear();
  }

  void end_alloc()
  {
    for (auto& image_object : this->imageobjects) {
      const auto memory = std::make_shared<VulkanMemory>(
        this->device,
        image_object->memory_requirements.size,
        image_object->memory_type_index);

      const VkDeviceSize offset = 0;
      image_object->bind(memory, offset);
    }

    for (auto& buffer_object : this->bufferobjects) {
      const auto memory = std::make_shared<VulkanMemory>(
        this->device,
        buffer_object->memory_requirements.size,
        buffer_object->memory_type_index);

      const VkDeviceSize offset = 0;
      buffer_object->bind(memory, offset);
    }
  }

  void traverse(std::function<void()> action)
  {
    this->state = State();

    this->begin_alloc();
    {
      VulkanCommandBufferScope scope(this->command->buffer());
      action();
    }
    this->end_alloc();

    FenceScope fence_scope(this->device->device, this->fence->fence);

    this->command->submit(this->queue,
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
      this->fence->fence);
  }

  std::shared_ptr<VulkanInstance> vulkan;
  std::shared_ptr<VulkanDevice> device;
  VkExtent2D extent;
  VkQueue queue{ nullptr };

  State state;

  std::shared_ptr<VulkanFence> fence;
  std::unique_ptr<VulkanCommandBuffers> command;
  std::shared_ptr<VulkanPipelineCache> pipelinecache;

  std::vector<std::shared_ptr<ImageObject>> imageobjects;
  std::vector<std::shared_ptr<BufferObject>> bufferobjects;
};

class Scene {
public:
  NO_COPY_OR_ASSIGNMENT(Scene)
  Scene() = delete;
  virtual ~Scene() = default;

  explicit Scene(std::shared_ptr<Group> root) :
    root(std::move(root))    
  {}

  void init(std::shared_ptr<VulkanInstance> vulkan,
            std::shared_ptr<VulkanDevice> device,
            VkExtent2D extent)
  {
    this->context = std::make_shared<TraversalContext>(vulkan, device, extent);

    this->alloc();
    this->stage();
    this->pipeline();
    this->record();
  }

  void redraw()
  {
    try {
      SceneRenderer renderer(this->context->vulkan,
                             this->context->device,
                             this->context->extent);

      this->root->render(&renderer);
      this->root->present(this->context.get());
    }
    catch (VkException &) {
      // recreate swapchain, try again next frame
      //this->resize(root, this->extent);
    }
  }

  void resize(VkExtent2D extent)
  {
    this->context->resize(extent);

    this->resize();
    this->record();
    this->redraw();
  }

  void alloc()
  {    
    this->context->traverse([&]() {
      this->root->alloc(this->context.get());
    });
  }

  void resize()
  {
    this->context->traverse([&]() {
      this->root->resize(this->context.get());
    });
  }

  void stage()
  {
    this->context->traverse([&]() {
      this->root->stage(this->context.get());
    });
  }

  void pipeline()
  {
    this->context->traverse([&]() {
      this->root->pipeline(this->context.get());
    });
  }

  void record()
  {
    this->context->traverse([&]() {
      this->root->record(this->context.get());
    });
  }

  std::shared_ptr<Group> root;
  std::shared_ptr<TraversalContext> context;

};
