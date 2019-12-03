
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


AllocVisitor::AllocVisitor()
{
	this->register_callback<Shader>([this](Shader* node) {
		node->alloc(this->context.get());
	});
	this->register_callback<Sampler>([this](Sampler* node) {
		node->alloc(this->context.get());
	});
	this->register_callback<TextureImage>([this](TextureImage* node) {
		node->alloc(this->context.get());
	});
	this->register_callback<Image>([this](Image* node) {
		node->alloc(this->context.get(), this->imageobjects);
	});
	this->register_callback<DrawCommand>([this](DrawCommand* node) {
		node->alloc(this->context.get());
	});
	this->register_callback<IndexedDrawCommand>([this](IndexedDrawCommand* node) {
		node->alloc(this->context.get());
	});
	this->register_callback<FramebufferAttachment>([this](FramebufferAttachment* node) {
		node->alloc(this->context.get());
	});
	this->register_callback<Framebuffer>([this](Framebuffer* node) {
		this->visit_group(node);
		node->alloc(this->context.get());
	});
	this->register_callback<InputAttachment>([this](InputAttachment* node) {
		node->alloc(this->context.get());
	});
	this->register_callback<ColorAttachment>([this](ColorAttachment* node) {
		node->alloc(this->context.get());
	});
	this->register_callback<ResolveAttachment>([this](ResolveAttachment* node) {
		node->alloc(this->context.get());
	});
	this->register_callback<DepthStencilAttachment>([this](DepthStencilAttachment* node) {
		node->alloc(this->context.get());
	});
	this->register_callback<PreserveAttachment>([this](PreserveAttachment* node) {
		node->alloc(this->context.get());
	});
	this->register_callback<PipelineBindpoint>([this](PipelineBindpoint* node) {
		node->alloc(this->context.get());
	});
	this->register_callback<SubpassDescription>([this](SubpassDescription* node) {
		this->visit_group(node);
		node->alloc(this->context.get());
	});
	this->register_callback<RenderpassAttachment>([this](RenderpassAttachment* node) {
		node->alloc(this->context.get());
	});
	this->register_callback<Renderpass>([this](Renderpass* node) {
		this->visit_group(node);
		node->alloc(this->context.get());
	});
	this->register_callback<RenderpassDescription>([this](RenderpassDescription* node) {
		this->visit_group(node);
		node->alloc(this->context.get());
	});
	this->register_callback<SwapchainObject>([this](SwapchainObject* node) {
		node->alloc(this->context.get());
	});
	this->register_callback<OffscreenImage>([this](OffscreenImage* node) {
		node->alloc(this->context.get());
	});
}

void
AllocVisitor::visit(Node* node)
{
	this->imageobjects.clear();
	this->bufferobjects.clear();

	this->context->begin();
	node->visit(this);
	this->context->end();

	for (auto image_object : this->imageobjects) {
		const auto memory = std::make_shared<VulkanMemory>(
			this->context->device,
			image_object->memory_requirements.size,
			image_object->memory_type_index);

		const VkDeviceSize offset = 0;
		image_object->bind(memory, offset);
	}

	for (auto buffer_object : this->bufferobjects) {
		const auto memory = std::make_shared<VulkanMemory>(
			this->context->device,
			buffer_object->memory_requirements.size,
			buffer_object->memory_type_index);

		const VkDeviceSize offset = 0;
		buffer_object->bind(memory, offset);
	}
}


StageVisitor::StageVisitor()
{
	this->register_callback<BufferData>([this](BufferData* node) {
		node->stage(this->context.get());
	});
	this->register_callback<InlineBufferData<float>>([this](InlineBufferData<float>* node) {
		node->stage(this->context.get());
	});
	this->register_callback<InlineBufferData<uint32_t>>([this](InlineBufferData<uint32_t>* node) {
		node->stage(this->context.get());
	});
	this->register_callback<TextureImage>([this](TextureImage* node) {
		node->stage(this->context.get());
	});
	this->register_callback<Image>([this](Image* node) {
		node->stage(this->context.get());
	});
	this->register_callback<ImageView>([this](ImageView* node) {
		node->stage(this->context.get());
	});
	this->register_callback<SwapchainObject>([this](SwapchainObject* node) {
		node->stage(this->context.get());
	});
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



ResizeVisitor::ResizeVisitor()
{
	this->register_callback<ProjMatrix>([this](ProjMatrix* node) {
		node->resize(this->context.get());
	});
	this->register_callback<FramebufferAttachment>([this](FramebufferAttachment* node) {
		node->alloc(this->context.get());
	});
	this->register_callback<Framebuffer>([this](Framebuffer* node) {
		this->visit_group(node);
		node->alloc(this->context.get());
	});
	this->register_callback<Renderpass>([this](Renderpass* node) {
		this->visit_group(node);
		node->resize(this->context.get());
	});
	this->register_callback<RenderpassDescription>([this](RenderpassDescription* node) {
		node->resize(this->context.get());
		this->visit_group(node);
	});
	this->register_callback<SwapchainObject>([this](SwapchainObject* node) {
		node->stage(this->context.get());
	});
	this->register_callback<OffscreenImage>([this](OffscreenImage* node) {
		node->alloc(this->context.get());
	});
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


PipelineVisitor::PipelineVisitor()
{
	this->register_callback<InlineBufferData<float>>([this](InlineBufferData<float>* node) {
		node->pipeline(this->context.get());
		});
	this->register_callback<InlineBufferData<uint32_t>>([this](InlineBufferData<uint32_t>* node) {
		node->pipeline(this->context.get());
		});
	this->register_callback<VertexInputAttributeDescription>([this](VertexInputAttributeDescription* node) {
		node->pipeline(this->context.get());
		});
	this->register_callback<VertexInputBindingDescription>([this](VertexInputBindingDescription* node) {
		node->pipeline(this->context.get());
		});
	this->register_callback<DescriptorSetLayoutBinding>([this](DescriptorSetLayoutBinding* node) {
		node->pipeline(this->context.get());
		});
	this->register_callback<Shader>([this](Shader* node) {
		node->pipeline(this->context.get());
		});
	this->register_callback<Sampler>([this](Sampler* node) {
		node->pipeline(this->context.get());
		});
	this->register_callback<TextureImage>([this](TextureImage* node) {
		node->pipeline(this->context.get());
		});
	this->register_callback<Image>([this](Image* node) {
		node->pipeline(this->context.get());
		});
	this->register_callback<ImageView>([this](ImageView* node) {
		node->pipeline(this->context.get());
		});
	this->register_callback<CullMode>([this](CullMode* node) {
		node->pipeline(this->context.get());
		});
	this->register_callback<ComputeCommand>([this](ComputeCommand* node) {
		node->pipeline(this->context.get());
		});
	this->register_callback<DrawCommandBase>([this](DrawCommandBase* node) {
		node->pipeline(this->context.get());
		});
	this->register_callback<DrawCommand>([this](DrawCommand* node) {
		node->pipeline(this->context.get());
		});
	this->register_callback<IndexedDrawCommand>([this](IndexedDrawCommand* node) {
		node->pipeline(this->context.get());
		});
	this->register_callback<RenderpassDescription>([this](RenderpassDescription* node) {
		node->pipeline(this->context.get());
		this->visit_group(node);
		});

}

void
PipelineVisitor::visit(Node* node)
{
	this->context->begin();
	node->visit(this);
	this->context->end();
}


RecordVisitor::RecordVisitor()
{
	this->register_callback<InlineBufferData<float>>([this](InlineBufferData<float>* node) {
		node->record(this->context.get());
		});
	this->register_callback<InlineBufferData<uint32_t>>([this](InlineBufferData<uint32_t>* node) {
		node->record(this->context.get());
		});
	this->register_callback<IndexBufferDescription>([this](IndexBufferDescription* node) {
		node->record(this->context.get());
		});
	this->register_callback<VertexInputAttributeDescription>([this](VertexInputAttributeDescription* node) {
		node->record(this->context.get());
		});
	this->register_callback<TextureImage>([this](TextureImage* node) {
		node->record(this->context.get());
		});
	this->register_callback<DrawCommand>([this](DrawCommand* node) {
		node->record(this->context.get());
		});
	this->register_callback<IndexedDrawCommand>([this](IndexedDrawCommand* node) {
		node->record(this->context.get());
		});
	this->register_callback<RenderpassDescription>([this](RenderpassDescription* node) {
		node->record(this->context.get());
		this->visit_group(node);
		});
	this->register_callback<SwapchainObject>([this](SwapchainObject* node) {
		node->record(this->context.get());
		});
	this->register_callback<OffscreenImage>([this](OffscreenImage* node) {
		node->record(this->context.get());
		});
}

void
RecordVisitor::visit(Node* node)
{
	this->context->begin();
	node->visit(this);
	this->context->end();
}