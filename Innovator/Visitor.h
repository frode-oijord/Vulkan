#pragma once

#include <Innovator/State.h>

#include <glm/glm.hpp>

#include <any>
#include <typeindex>
#include <functional>

class Visitor {
public:
	Visitor() = default;

	template <typename NodeType>
	void register_callback(std::function<void(NodeType*)> callback)
	{
		this->callbacks[typeid(NodeType)] = callback;
	}

	template <typename NodeType>
	void apply(NodeType* node)
	{
		auto it = callbacks.find(typeid(NodeType));
		if (it != this->callbacks.end()) {
			auto callback = std::any_cast<std::function<void(NodeType*)>>(it->second);
			callback(node);
		}
	}

	void visit(class Node* node);

	std::unordered_map<std::type_index, std::any> callbacks;

	void init(
		std::shared_ptr<VulkanInstance> vulkan,
		std::shared_ptr<VulkanDevice> device,
		VkExtent2D extent)
	{
		this->vulkan = std::move(vulkan);
		this->device = std::move(device);
		this->extent = extent;
	}

	void resize(VkExtent2D extent)
	{
		this->extent = extent;
	}

	std::shared_ptr<VulkanInstance> vulkan{ nullptr };
	std::shared_ptr<VulkanDevice> device{ nullptr };

	State state;
	VkExtent2D extent{ 0, 0 };
};


class EventVisitor : public Visitor {
public:
	EventVisitor();
	void visit(class ViewMatrix* node);
	void visit(class ModelMatrix* node);
	void visit(class TextureMatrix* node);

	bool press;
	bool move;
	int button;
	glm::dvec2 currpos;
	glm::dvec2 prevpos;
};

class DeviceVisitor : public Visitor {
public:
	void visit(class Node* node);

	VkPhysicalDeviceFeatures device_features;

	VkPhysicalDeviceBufferDeviceAddressFeatures device_address_features{
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR
	};

	VkPhysicalDeviceFeatures2 device_features2{
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR
	};

	std::vector<const char*> instance_extensions;
	std::vector<const char*> instance_layers;
	std::vector<const char*> device_extensions;
	std::vector<const char*> device_layers;
};

class CommandVisitor : public Visitor {
public:
	CommandVisitor();
	void visit(class Node* node);

	void init(
		std::shared_ptr<VulkanInstance> vulkan,
		std::shared_ptr<VulkanDevice> device,
		VkExtent2D extent)
	{
		Visitor::init(vulkan, device, extent);
		this->fence = std::make_unique<VulkanFence>(this->device);
		this->command = std::make_unique<VulkanCommandBuffers>(this->device);
		this->queue = this->device->getQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT);
	}


	VkQueue queue{ nullptr };
	std::shared_ptr<VulkanFence> fence{ nullptr };
	std::vector<VkSemaphore> wait_semaphores;
	std::unique_ptr<VulkanCommandBuffers> command{ nullptr };
};

class PipelineVisitor : public Visitor {
public:
	void init(
		std::shared_ptr<VulkanInstance> vulkan,
		std::shared_ptr<VulkanDevice> device,
		VkExtent2D extent)
	{
		Visitor::init(vulkan, device, extent);
		this->pipelinecache = std::make_shared<VulkanPipelineCache>(this->device);
	}
	std::shared_ptr<VulkanPipelineCache> pipelinecache{ nullptr };
};


class RenderVisitor : public CommandVisitor {
public:
	class OffscreenImage* image{ nullptr };
};

inline static EventVisitor eventvisitor;
inline static DeviceVisitor devicevisitor;

inline static CommandVisitor allocvisitor;
inline static CommandVisitor resizevisitor;
inline static RenderVisitor rendervisitor;
inline static PipelineVisitor pipelinevisitor;
inline static Visitor recordvisitor;
inline static Visitor presentvisitor;
