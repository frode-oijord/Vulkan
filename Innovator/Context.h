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

class Context {
public:
  Context(std::shared_ptr<VulkanInstance> vulkan,
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

  virtual ~Context()
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

  std::shared_ptr<VulkanInstance> vulkan;
  std::shared_ptr<VulkanDevice> device;
  VkExtent2D extent;
  VkQueue queue{ nullptr };

  State state;

  std::shared_ptr<VulkanFence> fence;
  std::unique_ptr<VulkanCommandBuffers> command;
  std::shared_ptr<VulkanPipelineCache> pipelinecache;

  std::vector<ImageObject*> imageobjects;
  std::vector<BufferObject*> bufferobjects;

  class Scope {
  public:
    Scope(Context* self)
      : self(self)
    {
      this->command_scope = std::make_unique<VulkanCommandBufferScope>(self->command->buffer());
      self->state = State();
      self->imageobjects.clear();
      self->bufferobjects.clear();
    }

    ~Scope()
    {
      this->command_scope.reset();

      for (auto image_object : self->imageobjects) {
        const auto memory = std::make_shared<VulkanMemory>(
          self->device,
          image_object->memory_requirements.size,
          image_object->memory_type_index);

        const VkDeviceSize offset = 0;
        image_object->bind(memory, offset);
      }

      for (auto buffer_object : self->bufferobjects) {
        const auto memory = std::make_shared<VulkanMemory>(
          self->device,
          buffer_object->memory_requirements.size,
          buffer_object->memory_type_index);

        const VkDeviceSize offset = 0;
        buffer_object->bind(memory, offset);
      }

      FenceScope fence_scope(self->device->device,
        self->fence->fence);

      self->command->submit(self->queue,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        self->fence->fence);
    }

    Context* self;
    std::unique_ptr<VulkanCommandBufferScope> command_scope;
  };
};
