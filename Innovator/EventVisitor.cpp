
#include <Innovator/EventVisitor.h>
#include <Innovator/Nodes.h>

EventVisitor::EventVisitor(std::shared_ptr<Context> context)
	: Visitor(context)
{
	this->register_callback<ViewMatrix>([this](ViewMatrix* node) {
		this->visit(node);
		});
}

void 
EventVisitor::visit(ViewMatrix* node)
{
	auto press = std::dynamic_pointer_cast<MousePressEvent>(context->event);
	if (press) {
		this->press = std::move(press);
	}

	if (std::dynamic_pointer_cast<MouseReleaseEvent>(context->event)) {
		this->press.reset();
		context->event.reset();
	}

	auto move = std::dynamic_pointer_cast<MouseMoveEvent>(context->event);
	if (move && this->press) {
		glm::dvec2 dx = (this->press->pos - move->pos) * .01;
		dx[1] = -dx[1];
		switch (this->press->button) {
		case 1: node->pan(dx); break;
		case 2: node->zoom(dx[1]); break;
		default: break;
		}
		this->press->pos = move->pos;
		context->event.reset();
	}
}
