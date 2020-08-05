#pragma once

#include <memory>
#include <Innovator/Nodes.h>

class VulkanRenderer {
public:
	~VulkanRenderer() = default;

	VulkanRenderer(std::shared_ptr<Node> scene, HWND hWnd, HINSTANCE hInstance) :
		scene(std::move(scene))
	{
#ifdef DEBUG
		//devicevisitor.device_layers.push_back("VK_LAYER_KHRONOS_validation");
		//devicevisitor.instance_layers.push_back("VK_LAYER_KHRONOS_validation");
		devicevisitor.instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		devicevisitor.instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		this->state = std::make_shared<State>();
		allocvisitor.state = this->state;
		resizevisitor.state = this->state;
		rendervisitor.state = this->state;
		pipelinevisitor.state = this->state;
		recordvisitor.state = this->state;
		presentvisitor.state = this->state;
		eventvisitor.state = this->state;
		devicevisitor.state = this->state;

		devicevisitor.visit(this->scene.get());


		this->state->vulkan = std::make_shared<VulkanInstance>(
			"Innovator",
			devicevisitor.instance_layers,
			devicevisitor.instance_extensions);

#ifdef DEBUG
		auto debugcb = std::make_unique<VulkanDebugCallback>(
			this->state->vulkan,
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_DEBUG_BIT_EXT);
#endif

		this->state->device = std::make_shared<VulkanDevice>(
			this->state->vulkan,
			devicevisitor.device_features2,
			devicevisitor.device_layers,
			devicevisitor.device_extensions);

		this->state->hWnd = hWnd;
		this->state->hInstance = hInstance;
		this->state->pipelinecache = std::make_shared<VulkanPipelineCache>(this->state->device);
		this->state->fence = std::make_shared<VulkanFence>(this->state->device);
		this->state->default_command = std::make_shared<VulkanCommandBuffers>(this->state->device);
		this->state->queue = this->state->device->getQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

		allocvisitor.visit(this->scene.get());
		pipelinevisitor.visit(this->scene.get());
		recordvisitor.visit(this->scene.get());

	}

	void redraw()
	{
		try {
			rendervisitor.visit(this->scene.get());
			presentvisitor.visit(this->scene.get());
		}
		catch (VkException& e) {
			std::cerr << e.what() << std::endl;
			// recreate swapchain, try again next frame
		}
	}

	void resize(int width, int height)
	{
		this->state->extent = VkExtent2D{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		resizevisitor.visit(this->scene.get());
		recordvisitor.visit(this->scene.get());
		this->redraw();
	}

	void mousePressed(int x, int y, int button)
	{
		eventvisitor.mousePressed(this->scene.get(), x, y, button);
	}

	void mouseReleased()
	{
		eventvisitor.mouseReleased(this->scene.get());
	}

	void mouseMoved(int x, int y)
	{
		eventvisitor.mouseMoved(this->scene.get(), x, y);
		if (eventvisitor.press) {
			this->redraw();
		}
	}


	std::shared_ptr<Node> scene;
	std::shared_ptr<State> state{ nullptr };

	inline static EventVisitor eventvisitor;
	inline static DeviceVisitor devicevisitor;
	inline static CommandVisitor allocvisitor;
	inline static CommandVisitor resizevisitor;
	inline static Visitor pipelinevisitor;
	inline static RenderVisitor rendervisitor;
	inline static Visitor recordvisitor;
	inline static Visitor presentvisitor;
};
