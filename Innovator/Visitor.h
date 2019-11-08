#pragma once

#include <any>
#include <typeindex>
#include <functional>

class Visitor {
public:
	Visitor(std::shared_ptr<class Context> context);

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

	std::shared_ptr<class Context> context;
	std::unordered_map<std::type_index, std::any> callbacks;
};
