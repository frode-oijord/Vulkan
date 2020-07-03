
#include <Innovator/Visitor.h>
#include <Innovator/Nodes.h>

#include <glm/glm.hpp>

void
Visitor::visit(Node* node)
{
	this->state = State();
	this->state.extent = *this->extent;

	node->visit(this);
}

CommandVisitor::CommandVisitor()
{}

void
CommandVisitor::visit(Node* node)
{
	this->command->begin();
	this->wait_semaphores.clear();
	this->state = State();
	this->state.extent = *this->extent;

	node->visit(this);

	this->fence->reset();
	this->command->end();
	this->command->submit(
		this->queue,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		this->fence->fence,
		this->wait_semaphores);

	this->fence->wait();
}


EventVisitor::EventVisitor()
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
EventVisitor::visit(Node* node)
{
	this->state = State();
	this->state.extent = *this->extent;
	node->visit(this);
}


void
EventVisitor::visit(ViewMatrix* node)
{
	if (this->move && this->press) {
		glm::dvec2 dx = this->prevpos - this->currpos;
		dx[0] /= this->state.extent.width;
		dx[1] /= this->state.extent.height;
		dx *= 20.0;
		switch (this->button) {
			// case 0: node->orbit(dx); break;
			case 1: node->pan(dx); break;
			case 2: node->zoom(dx[1]); break;
			default: break;
		}
	}
}


void 
EventVisitor::visit(class TextureMatrix* node)
{

	if (this->move && this->press) {
		glm::dvec2 dx = this->prevpos - this->currpos;
		double translation = dx[1] * 0.001;
		glm::dvec3 t(0, 0, translation);
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
	if (this->move && this->press) {
		glm::dvec2 dx = this->prevpos - this->currpos;
		double translation = dx[1] * 0.001;
		glm::dvec3 t(0, 0, translation);
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
DeviceVisitor::visit(class Node* node)
{
	::memset(&device_features, VK_FALSE, sizeof(VkPhysicalDeviceFeatures));
	::memset(&device_features2, VK_FALSE, sizeof(VkPhysicalDeviceFeatures2));
	::memset(&device_address_features, VK_FALSE, sizeof(VkPhysicalDeviceBufferDeviceAddressFeatures));

	device_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
	device_address_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;
	device_features2.pNext = &device_address_features;

	node->visit(this);

	device_features2.features = device_features;
}
