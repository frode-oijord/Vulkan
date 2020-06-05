
#include <Innovator/Visitor.h>
#include <Innovator/Nodes.h>


void
Visitor::visit(Node* node, Context* context)
{
	node->visit(this, context);
}


CommandVisitor::CommandVisitor()
{

}

void
CommandVisitor::visit(Node* node, Context* context)
{
	context->command->begin();
	context->wait_semaphores.clear();
	context->state = State();
	context->state.extent = context->extent;

	node->visit(this, context);

	context->fence->reset();
	context->command->end();
	context->command->submit(
		context->queue,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		context->fence->fence,
		context->wait_semaphores);

	context->fence->wait();
}


EventVisitor::EventVisitor()
{
	this->register_callback<ViewMatrix>([this](ViewMatrix* node, Context* context) {
		this->visit(node, context);
	});
	this->register_callback<ModelMatrix>([this](ModelMatrix* node, Context* context) {
		this->visit(node, context);
	});
	this->register_callback<TextureMatrix>([this](TextureMatrix* node, Context* context) {
		this->visit(node, context);
	});
}


void
EventVisitor::visit(ViewMatrix* node, Context* context)
{
	if (this->move && this->press) {
		glm::dvec2 dx = this->prevpos - this->currpos;
		dx[0] /= context->state.extent.width;
		dx[1] /= context->state.extent.height;
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
EventVisitor::visit(class TextureMatrix* node, class Context* context)
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
	//	case 0: {
	//		node->mat[3][2] += dx[1] * 0.2f;
	//		node->mat[3][2] = std::clamp(node->mat[3][2], 0.0, 1.0);
	//		break;
	//	}
	//	default: break;
	//	}
	//	this->press->pos = move->pos;
	//}
}


void
EventVisitor::visit(class ModelMatrix* node, class Context* context)
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
	//	case 0: node->matrix[3][2] += dx[1] * 0.2f; break;
	//	default: break;
	//	}
	//	this->press->pos = move->pos;
	//}
}


DeviceVisitor::DeviceVisitor()
{
	::memset(&device_features, VK_FALSE, sizeof(VkPhysicalDeviceFeatures));

	this->register_callback<SparseImage>([this](SparseImage* node, Context* context) {
		this->device_features.sparseBinding = VK_TRUE;
		this->device_features.sparseResidencyImage2D = VK_TRUE;
		this->device_features.sparseResidencyImage3D = VK_TRUE;
	});

	this->register_callback<Shader>([this](Shader* node, Context* context) {
		switch (node->stage) {
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
			this->device_features.tessellationShader = VK_TRUE;
			break;
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
			this->device_features.tessellationShader = VK_TRUE;
			break;
		case VK_SHADER_STAGE_GEOMETRY_BIT:
			this->device_features.geometryShader = VK_TRUE;
			break;
		default: break;
		}
	});
}

void
DeviceVisitor::visit(class Node* node, class Context* context)
{
	::memset(&device_features, VK_FALSE, sizeof(VkPhysicalDeviceFeatures));
	node->visit(this, context);
}
