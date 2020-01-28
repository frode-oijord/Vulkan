
#include <Innovator/Visitor.h>
#include <Innovator/Nodes.h>

Visitor::Visitor()
{
	auto visit_group = [this](Group* node, Context* context) {
		this->visit_group(node, context);
	};
	auto visit_separator = [this](Separator* node, Context* context) {
		this->visit_separator(node, context);
	};

	this->register_callback<Group>(visit_group);
	this->register_callback<Renderpass>(visit_group);
	this->register_callback<Separator>(visit_separator);
	this->register_callback<Framebuffer>(visit_group);
	this->register_callback<SubpassDescription>(visit_group);
	this->register_callback<RenderpassDescription>(visit_group);
}


void 
Visitor::visit_group(Group* node, Context* context)
{
	for (auto child : node->children) {
		child->visit(this, context);
	}
}

void
Visitor::visit_separator(Separator* node, Context* context)
{
	StateScope scope(&context->state);
	this->visit_group(node, context);
}

void
Visitor::visit(Node* node, Context* context)
{
	context->command->begin();

	context->begin();
	node->visit(this, context);
	context->end();

	context->fence->reset();
	context->command->end();
	context->command->submit(
		context->queue,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		context->fence->fence,
		context->wait_semaphores);

	context->fence->wait();
}


void 
RenderVisitor::visit(class Node* node, class Context* context)
{
	context->begin();
	node->visit(this, context);
	context->end();
}


EventVisitor::EventVisitor()
{
	this->register_callback<ViewMatrix>([this](ViewMatrix* node, Context* context) {
		this->visit(node, context);
	});
	this->register_callback<TextureOffset>([this](TextureOffset* node, Context* context) {
		this->visit(node, context);
	});
}

void
EventVisitor::visit(ViewMatrix* node, Context* context)
{
	auto press = std::dynamic_pointer_cast<MousePressEvent>(context->event);
	if (press) {
		this->press = press;
	}

	if (std::dynamic_pointer_cast<MouseReleaseEvent>(context->event)) {
		this->press.reset();
	}

	auto move = std::dynamic_pointer_cast<MouseMoveEvent>(context->event);
	if (move && this->press) {
		glm::dvec2 dx = (this->press->pos - move->pos) * .01;
		dx[1] = -dx[1];
		switch (this->press->button) {
		case 1: node->pan(dx * 0.1); break;
		case 2: node->zoom(dx[1] * 0.5); break;
		default: break;
		}
		this->press->pos = move->pos;
	}
}


void 
EventVisitor::visit(class TextureOffset* node, class Context* context)
{
	//auto press = std::dynamic_pointer_cast<MousePressEvent>(context->event);
	//if (press) {
	//	this->press = press;
	//}

	//if (std::dynamic_pointer_cast<MouseReleaseEvent>(context->event)) {
	//	this->press.reset();
	//}

	//auto move = std::dynamic_pointer_cast<MouseMoveEvent>(context->event);
	//if (move && this->press) {
	//	glm::dvec2 dx = (this->press->pos - move->pos) * .01;
	//	dx[1] = -dx[1];
	//	switch (this->press->button) {
	//	case 0: node->offset += dx[1] * 0.01f; break;
	//	default: break;
	//	}
	//	this->press->pos = move->pos;
	//}
}
