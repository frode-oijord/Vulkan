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


class Visitor {
public:
	Visitor();

	template <typename NodeType>
	void register_callback(std::function<void(NodeType*)> callback)
	{
		this->callbacks[typeid(NodeType)] = callback;
	}

  template <typename NodeType>
  void remove_callback()
  {
    this->callbacks.erase(typeid(NodeType));
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

	void visit_group(class Group* node);
	void visit_separator(class Separator* node);

	std::shared_ptr<class Context> context;
	std::unordered_map<std::type_index, std::any> callbacks;
};


class EventVisitor : public Visitor {
public:
	EventVisitor();
	void visit(class ViewMatrix* node);

private:
	std::shared_ptr<class MousePressEvent> press{ nullptr };
};


class AllocVisitor : public Visitor {
public:
	AllocVisitor();

	void visit(class Node* node);

	std::vector<class ImageObject*> imageobjects;
	std::vector<class BufferObject*> bufferobjects;
};


class StageVisitor : public Visitor {
public:
	StageVisitor();

	void visit(class Node* node);
};


class ResizeVisitor : public Visitor {
public:
	ResizeVisitor();

	void visit(class Node* node);
};

class PipelineVisitor : public Visitor {
public:
	PipelineVisitor();

	void visit(class Node* node);
};

class RecordVisitor : public Visitor {
public:
	RecordVisitor();

	void visit(class Node* node);
};

inline static EventVisitor eventvisitor;
inline static AllocVisitor allocvisitor;
inline static StageVisitor stagevisitor;
inline static ResizeVisitor resizevisitor;
inline static PipelineVisitor pipelinevisitor;
inline static RecordVisitor recordvisitor;
