#pragma once

#include <Innovator/Defines.h>
#include <Innovator/State.h>

#include <set>
#include <memory>
#include <vector>
#include <iostream>
#include <functional>


class Context {
public:
	Context()
	{}

	void init(
		std::shared_ptr<VulkanInstance> vulkan,
		std::shared_ptr<VulkanDevice> device,
		VkExtent2D extent)
	{
		this->vulkan = std::move(vulkan);
		this->device = std::move(device);
		this->extent = extent;
		this->fence = std::make_unique<VulkanFence>(this->device);
		this->command = std::make_unique<VulkanCommandBuffers>(this->device);
		this->pipelinecache = std::make_shared<VulkanPipelineCache>(this->device);
		this->queue = this->device->getQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT);
	}

	void resize(VkExtent2D extent)
	{
		this->extent = extent;
	}

	void record(std::function<void()> callback)
	{
		this->command->begin();
		this->wait_semaphores.clear();
		this->state = State();
		this->state.extent = this->extent;

		callback();

		this->fence->reset();
		this->command->end();
		this->command->submit(
			this->queue,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			this->fence->fence,
			this->wait_semaphores);

		this->fence->wait();
	}

	std::shared_ptr<VulkanInstance> vulkan{ nullptr };
	std::shared_ptr<VulkanDevice> device{ nullptr };
	VkQueue queue{ nullptr };

	State state;

	std::shared_ptr<VulkanFence> fence{ nullptr };
	std::vector<VkSemaphore> wait_semaphores;
	std::unique_ptr<VulkanCommandBuffers> command{ nullptr };
	std::shared_ptr<VulkanPipelineCache> pipelinecache{ nullptr };

	class OffscreenImage* image{ nullptr };

	VkExtent2D extent{ 0, 0 };
};
