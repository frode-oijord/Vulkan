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

class RenderStateScope {
public:
	RenderStateScope() = delete;

	explicit RenderStateScope(State* state) :
		state(state),
		ModelMatrix(state->ModelMatrix),
		ViewMatrix(state->ViewMatrix),
		ProjMatrix(state->ProjMatrix)
	{}

	~RenderStateScope()
	{
		state->ModelMatrix = this->ModelMatrix;
		state->ViewMatrix = this->ViewMatrix;
		state->ProjMatrix = this->ProjMatrix;
	}

	State* state;

	glm::dmat4 ModelMatrix{ 1.0 };
	glm::dmat4 ViewMatrix{ 1.0 };
	glm::dmat4 ProjMatrix{ 1.0 };
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

	void visit_group(class Group* node, class Context* context);
	void visit_separator(class Separator* node, class Context* context);

	std::unordered_map<std::type_index, std::any> callbacks;
};


class EventVisitor : public Visitor {
public:
	EventVisitor();
	void visit(class ViewMatrix* node, class Context* context);

private:
	std::shared_ptr<class MousePressEvent> press{ nullptr };
};


class AllocVisitor : public Visitor {
public:
	void visit(class Node* node, class Context* context);
};


class StageVisitor : public Visitor {
public:
	void visit(class Node* node, class Context* context);
};


class ResizeVisitor : public Visitor {
public:
	void visit(class Node* node, class Context* context);
};


class PipelineVisitor : public Visitor {
public:
	void visit(class Node* node, class Context* context);
};


class RecordVisitor : public Visitor {
public:
	void visit(class Node* node, class Context* context);
};


inline static EventVisitor eventvisitor;
inline static AllocVisitor allocvisitor;
inline static StageVisitor stagevisitor;
inline static ResizeVisitor resizevisitor;
inline static PipelineVisitor pipelinevisitor;
inline static RecordVisitor recordvisitor;
