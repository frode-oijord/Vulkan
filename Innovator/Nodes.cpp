
#include <Innovator/Nodes.h>
#include <Innovator/Visitor.h>

class StateScope {
public:
	NO_COPY_OR_ASSIGNMENT(StateScope)
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
	NO_COPY_OR_ASSIGNMENT(RenderStateScope)
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

IMPLEMENT_VISITABLE(Separator)

Separator::Separator(std::vector<std::shared_ptr<Node>> children) :
	Group(std::move(children))
{}

void 
Separator::doAlloc(Context* context)
{
	StateScope scope(&context->state);
	Group::doAlloc(context);
}

void 
Separator::doResize(Context* context)
{
	StateScope scope(&context->state);
	Group::doResize(context);
}

void 
Separator::doStage(Context* context)
{
	StateScope scope(&context->state);
	Group::doStage(context);
}

void 
Separator::doPipeline(Context* context)
{
	StateScope scope(&context->state);
	Group::doPipeline(context);
}

void 
Separator::doRecord(Context* context)
{
	StateScope scope(&context->state);
	Group::doRecord(context);
}

void 
Separator::doRender(Context* context)
{
	RenderStateScope scope(&context->state);
	Group::doRender(context);
}

