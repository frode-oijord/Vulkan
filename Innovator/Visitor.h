#pragma once

#include <Innovator/State.h>

#include <glm/glm.hpp>

#include <any>
#include <unordered_map>
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
	std::shared_ptr<State> state{ nullptr };
};


class EventVisitor : public Visitor {
public:
	EventVisitor();

	void visit(class Node* node);

private:
	void visit(class ViewMatrix* node);
	void visit(class ModelMatrix* node);
	void visit(class TextureMatrix* node);

public:
	void mousePressed(Node* root, int x, int y, int button)
	{
		this->press = true;
		this->button = button;
		this->currpos = glm::dvec2(x, y);
		this->visit(root);
		this->prevpos = this->currpos;
	}

	void mouseReleased(Node* root)
	{
		this->press = false;
		this->visit(root);
		this->prevpos = this->currpos;
	}

	void mouseMoved(Node* root, int x, int y)
	{
		this->move = true;
		this->currpos = glm::dvec2(x, y);
		this->visit(root);
		this->prevpos = this->currpos;
		this->move = false;
	}

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
#ifdef VK_USE_PLATFORM_WIN32_KHR
	VkPhysicalDeviceBufferDeviceAddressFeatures device_address_features{
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR
	};
	VkPhysicalDeviceRayTracingFeaturesKHR ray_tracing_features{
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_FEATURES_KHR
	};
#endif
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
};


class RenderVisitor : public CommandVisitor {
public:
	class OffscreenImage* image{ nullptr };
};

inline static EventVisitor eventvisitor;
inline static DeviceVisitor devicevisitor;
inline static CommandVisitor allocvisitor;
inline static CommandVisitor resizevisitor;
inline static Visitor pipelinevisitor;
inline static RenderVisitor rendervisitor;
inline static Visitor recordvisitor;
inline static Visitor presentvisitor;
