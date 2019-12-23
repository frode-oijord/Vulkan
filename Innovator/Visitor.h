#pragma once

#include <Innovator/State.h>

#include <any>
#include <typeindex>
#include <functional>

class StateScope {
public:
	StateScope() = delete;

	explicit StateScope(State* state) :
		stateptr(state),
		statecpy(*state)
	{}

	~StateScope()
	{
		*stateptr = this->statecpy;
	}

	State* stateptr;
	State statecpy;
};


class Context;

class Visitor {
public:
	Visitor();

	template <typename NodeType>
	void register_callback(std::function<void(NodeType*, Context*)> callback)
	{
		this->callbacks[typeid(NodeType)] = callback;
	}

	template <typename NodeType>
	void apply(NodeType* node, Context* context)
	{
		auto it = callbacks.find(typeid(NodeType));
		if (it != this->callbacks.end()) {
			auto callback = std::any_cast<std::function<void(NodeType*, Context*)>>(it->second);
			callback(node, context);
		}
	}

	void visit(class Node* node, class Context* context);
	void visit_group(class Group* node, class Context* context);
	void visit_separator(class Separator* node, class Context* context);

	std::unordered_map<std::type_index, std::any> callbacks;
};


class EventVisitor : public Visitor {
public:
	EventVisitor();
	void visit(class ViewMatrix* node, class Context* context);
	void visit(class TextureOffset* node, class Context* context);

private:
	std::shared_ptr<class MousePressEvent> press{ nullptr };
};

inline static EventVisitor eventvisitor;

inline static Visitor allocvisitor;
inline static Visitor resizevisitor;
inline static Visitor pipelinevisitor;
inline static Visitor recordvisitor;
inline static Visitor rendervisitor;
inline static Visitor presentvisitor;