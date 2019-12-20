#pragma once

#include <Innovator/Defines.h>
#include <Innovator/State.h>
#include <Innovator/Events.h>

#include <set>
#include <memory>
#include <vector>
#include <iostream>


class Context {
public:
  Context(std::shared_ptr<VulkanInstance> vulkan,
          std::shared_ptr<VulkanDevice> device,
          VkExtent2D extent) :
    vulkan(std::move(vulkan)),
    device(std::move(device)),
    extent(extent),
    redraw(false),
    fence(std::make_unique<VulkanFence>(this->device)),
    command(std::make_unique<VulkanCommandBuffers>(this->device)),
    pipelinecache(std::make_shared<VulkanPipelineCache>(this->device))
  {
    this->queue = this->device->getQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT);
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

  void begin()
  {
    this->wait_semaphores.clear();
    this->redraw = false;
    this->state = State();
		this->state.extent = this->extent;
  }

  void end()
  {
  }

  std::shared_ptr<VulkanInstance> vulkan;
  std::shared_ptr<VulkanDevice> device;
  VkQueue queue{ nullptr };

  State state;
  std::shared_ptr<Event> event;
  bool redraw;

  std::shared_ptr<VulkanFence> fence;
  std::vector<VkSemaphore> wait_semaphores;
  std::unique_ptr<VulkanCommandBuffers> command;
  std::shared_ptr<VulkanPipelineCache> pipelinecache;

	std::vector<class ImageObject*> imageobjects;
	std::vector<class BufferObject*> bufferobjects;

  std::set<uint32_t> tiles;

private:
  VkExtent2D extent;
};
