#pragma once

#include <Innovator/State.h>

#include <glm/glm.hpp>

#include <any>
#include <map>
#include <variant>
#include <unordered_map>
#include <typeindex>
#include <functional>

class Visitor {
public:
	Visitor(std::shared_ptr<State> state)
		: state(std::move(state)) {}

	template <typename NodeType>
	void register_callback(std::function<void(NodeType*)> callback)
	{
		this->callbacks[typeid(NodeType)] = callback;
	}

	void register_callback(std::type_index type, std::function<void(class Node*)> callback)
	{
		this->callbacks[type] = callback;
	}

	template <typename NodeType>
	void apply(NodeType* node)
	{
		auto it = this->callbacks.find(typeid(NodeType));
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
	EventVisitor(std::shared_ptr<State> state);

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
		Visitor::visit(root);
		this->prevpos = this->currpos;
	}

	void mouseReleased(Node* root)
	{
		this->press = false;
		Visitor::visit(root);
		this->prevpos = this->currpos;
	}

	void mouseMoved(Node* root, int x, int y)
	{
		this->move = true;
		this->currpos = glm::dvec2(x, y);
		Visitor::visit(root);
		this->prevpos = this->currpos;
		this->move = false;
	}

	bool press;
	bool move;
	int button;
	glm::dvec2 currpos;
	glm::dvec2 prevpos;
};


template <typename T>
struct VkStructureTypeMap {	static const VkStructureType type; };

const VkStructureType VkStructureTypeMap<VkPhysicalDeviceRayTracingFeaturesKHR>::type = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_FEATURES_KHR;
const VkStructureType VkStructureTypeMap<VkPhysicalDeviceBufferDeviceAddressFeatures>::type = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;


class DeviceVisitor : public Visitor {
public:
	DeviceVisitor(std::shared_ptr<State> state) : Visitor(state) {}

	template<typename T>
	T& getFeatures(VkStructureType type = VkStructureTypeMap<T>::type)
	{
		if (!this->required_device_features.contains(type)) {
			this->last_requested_feature = new T{
				.sType = type,
				.pNext = this->last_requested_feature,
			};
			this->required_device_features[type] = this->last_requested_feature;
		}
		return *static_cast<T*>(this->required_device_features.at(type));
	}

	VkPhysicalDeviceFeatures2 getDeviceFeatures()
	{
		return {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR,
			.pNext = this->last_requested_feature,
			.features = this->device_features,
		};
	}

	VkPhysicalDeviceFeatures device_features{};
	std::vector<const char*> instance_extensions;
	std::vector<const char*> instance_layers;
	std::vector<const char*> device_extensions;
	std::vector<const char*> device_layers;

private:
	void* last_requested_feature{ nullptr };
	std::map<VkStructureType, void*> required_device_features;
};

class CommandVisitor : public Visitor {
public:
	CommandVisitor(std::shared_ptr<State> state) : Visitor(state) {}
	void visit(class Node* node);
};


class RenderVisitor : public CommandVisitor {
public:
	RenderVisitor(std::shared_ptr<State> state) : CommandVisitor(state) {}
	class OffscreenImage* image{ nullptr };
};

inline std::shared_ptr<State> state = std::make_shared<State>();

inline EventVisitor eventvisitor(state);
inline DeviceVisitor devicevisitor(state);
inline CommandVisitor allocvisitor(state);
inline CommandVisitor resizevisitor(state);
inline Visitor pipelinevisitor(state);
inline RenderVisitor rendervisitor(state);
inline Visitor recordvisitor(state);
inline Visitor presentvisitor(state);
