
#include <Innovator/Visitor.h>
#include <Innovator/Nodes.h>

#include <glm/glm.hpp>

void
Visitor::visit(Node* node)
{
	StateScope scope(this->state.get());
	node->visit(this);
}


void
CommandVisitor::visit(Node* node)
{
	StateScope scope(this->state.get());

	this->state->default_command->begin();
	this->state->wait_semaphores.clear();

	node->visit(this);

	this->state->fence->reset();
	this->state->default_command->end();
	this->state->default_command->submit(
		this->state->queue,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		this->state->fence->fence,
		this->state->wait_semaphores);

	this->state->fence->wait();
}


EventVisitor::EventVisitor(std::shared_ptr<State> state) :
	Visitor(state)
{
	this->register_callback<ViewMatrix>([this](ViewMatrix* node) {
		this->visit(node);
		});
	this->register_callback<ModelMatrix>([this](ModelMatrix* node) {
		this->visit(node);
		});
	this->register_callback<TextureMatrix>([this](TextureMatrix* node) {
		this->visit(node);
		});
}


void
EventVisitor::visit(ViewMatrix* node)
{
	if (this->move && this->press && !this->interact) {
		glm::dvec2 dx = this->prevpos - this->currpos;
		dx[0] /= this->state->extent.width;
		dx[1] /= this->state->extent.height;
		dx *= 20.0;
		switch (this->button) {
		case 0: node->orbit(dx); break;
		case 1: node->pan(dx); break;
		case 2: node->zoom(dx[1]); break;
		default: break;
		}
	}
}


void
EventVisitor::visit(class TextureMatrix* node)
{
	if (this->move && this->press && this->interact) {
		glm::dvec2 dx = this->prevpos - this->currpos;
		double translation = dx[1] * 0.001;
		glm::dvec3 t(0, translation, 0);
		switch (this->button) {
		case 0: {
			node->mat = glm::translate(node->mat, t);
			break;
		}
		default: break;
		}
	}
}


void
EventVisitor::visit(class ModelMatrix* node)
{
	if (this->move && this->press && this->interact) {
		glm::dvec2 dx = this->prevpos - this->currpos;
		double translation = dx[1] * 0.001;
		glm::dvec3 t(0, translation, 0);
		switch (this->button) {
		case 0: {
			node->mat = glm::translate(node->mat, t);
			break;
		}
		default: break;
		}
	}
}
