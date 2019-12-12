
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


EventVisitor::EventVisitor()
{
	this->register_callback<ViewMatrix>([this](ViewMatrix* node, Context* context) {
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
		case 1: node->pan(dx); break;
		case 2: node->zoom(dx[1]); break;
		default: break;
		}
		this->press->pos = move->pos;
	}
}


void
AllocVisitor::visit(Node* node, Context* context)
{
	context->imageobjects.clear();
	context->bufferobjects.clear();

	context->begin();
	node->visit(this, context);
	context->end();

	for (auto image_object : context->imageobjects) {
		const auto memory = std::make_shared<VulkanMemory>(
			context->device,
			image_object->memory_requirements.size,
			image_object->memory_type_index);

		const VkDeviceSize offset = 0;
		image_object->bind(memory, offset);
	}

	for (auto buffer_object : context->bufferobjects) {
		const auto memory = std::make_shared<VulkanMemory>(
			context->device,
			buffer_object->memory_requirements.size,
			buffer_object->memory_type_index);

		const VkDeviceSize offset = 0;
		buffer_object->bind(memory, offset);
	}
}


void
StageVisitor::visit(Node* node, Context* context)
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
		context->fence->fence);

	context->fence->wait();
}


void
ResizeVisitor::visit(Node* node, Context* context)
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
		context->fence->fence);

	context->fence->wait();
}


void
PipelineVisitor::visit(Node* node, Context* context)
{
	context->begin();
	node->visit(this, context);
	context->end();
}


void
RecordVisitor::visit(Node* node, Context* context)
{
	context->begin();
	node->visit(this, context);
	context->end();
}