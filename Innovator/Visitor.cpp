
#include <Innovator/Visitor.h>
#include <Innovator/Nodes.h>

Visitor::Visitor()
{
	auto visit_group = [this](Group* node) {
		this->visit_group(node);
	};
	auto visit_separator = [this](Separator* node) {
		this->visit_separator(node);
	};

	this->register_callback<Group>(visit_group);
	this->register_callback<Scene>(visit_group);
	this->register_callback<Renderpass>(visit_group);
	this->register_callback<Separator>(visit_separator);
	this->register_callback<Framebuffer>(visit_group);
	this->register_callback<SubpassDescription>(visit_group);
	this->register_callback<RenderpassDescription>(visit_group);
}


void 
Visitor::visit_group(Group* node)
{
	for (auto child : node->children) {
		child->visit(this);
	}
}

void
Visitor::visit_separator(Separator* node)
{
	StateScope scope(&context->state);
	this->visit_group(node);
}


EventVisitor::EventVisitor()
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


void
AllocVisitor::visit(Node* node)
{
	this->context->imageobjects.clear();
	this->context->bufferobjects.clear();

	this->context->begin();
	node->visit(this);
	this->context->end();

	for (auto image_object : this->context->imageobjects) {
		const auto memory = std::make_shared<VulkanMemory>(
			this->context->device,
			image_object->memory_requirements.size,
			image_object->memory_type_index);

		const VkDeviceSize offset = 0;
		image_object->bind(memory, offset);
	}

	for (auto buffer_object : this->context->bufferobjects) {
		const auto memory = std::make_shared<VulkanMemory>(
			this->context->device,
			buffer_object->memory_requirements.size,
			buffer_object->memory_type_index);

		const VkDeviceSize offset = 0;
		buffer_object->bind(memory, offset);
	}
}


void
StageVisitor::visit(Node* node)
{
	this->context->command->begin();

	this->context->begin();
	node->visit(this);
	this->context->end();

	this->context->fence->reset();
	this->context->command->end();
	this->context->command->submit(
		this->context->queue,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		this->context->fence->fence);

	this->context->fence->wait();
}


void
ResizeVisitor::visit(Node* node)
{
	this->context->command->begin();

	this->context->begin();
	node->visit(this);
	this->context->end();

	this->context->fence->reset();
	this->context->command->end();
	this->context->command->submit(
		this->context->queue,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		this->context->fence->fence);

	this->context->fence->wait();
}


void
PipelineVisitor::visit(Node* node)
{
	this->context->begin();
	node->visit(this);
	this->context->end();
}


void
RecordVisitor::visit(Node* node)
{
	this->context->begin();
	node->visit(this);
	this->context->end();
}