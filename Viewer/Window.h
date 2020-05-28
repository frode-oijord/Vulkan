#pragma once

#include <Innovator/Nodes.h>
#include <Innovator/Context.h>
#include <Innovator/Defines.h>
#include <Innovator/VulkanSurface.h>

#include <glm/glm.hpp>

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

class Window {
public:
	Window()
	{
		this->hInstance = GetModuleHandle(NULL);
		TCHAR szWindowClass[] = _T("DesktopApp");
		TCHAR szTitle[] = _T("Vulkan Window");

		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = szWindowClass;
		wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

		if (!RegisterClassEx(&wcex)) {
			throw std::runtime_error("unable to register window class");
		}

		this->hWnd = CreateWindow(
			szWindowClass,
			_T("Vulkan 3D Window App"),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			1920, 1080,
			NULL,
			NULL,
			hInstance,
			this);

		if (!hWnd) {
			throw std::runtime_error("Call to CreateWindow failed!");
		}
	}

	virtual ~Window() = default;

	virtual void redraw() = 0;
	virtual void resize(int width, int height) = 0;
	virtual void mousePressed(int x, int y, int button) = 0;
	virtual void mouseReleased() = 0;
	virtual void mouseMoved(int x, int y) = 0;

	LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message) {
		case WM_PAINT:
			this->redraw();
			break;
		case WM_SIZE:
			this->resize(LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_LBUTTONDOWN:
			this->mousePressed(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);
			break;
		case WM_MBUTTONDOWN:
			this->mousePressed(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 1);
			break;
		case WM_RBUTTONDOWN:
			this->mousePressed(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 2);
			break;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			this->mouseReleased();
			break;
		case WM_MOUSEMOVE:
			this->mouseMoved(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
		}
		return 0;
	}


	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		Window* self = nullptr;
		if (message == WM_NCCREATE) {
			LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
			self = static_cast<Window*>(lpcs->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
		}
		else {
			self = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		}
		if (self) {
			return self->wndProc(hWnd, message, wParam, lParam);
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}


	int show()
	{
		ShowWindow(this->hWnd, SW_SHOWDEFAULT);
		UpdateWindow(this->hWnd);

		// Main message loop:
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return (int)msg.wParam;
	}

protected:
	HWND hWnd;
	HMODULE hInstance;
};

class VulkanWindow : public Window {
public:
	NO_COPY_OR_ASSIGNMENT(VulkanWindow)
	virtual ~VulkanWindow() = default;

	VulkanWindow(std::shared_ptr<Node> scene, std::shared_ptr<FramebufferAttachment> color_attachment)
	{
		std::vector<const char*> instance_layers{
	#ifdef DEBUG
		  "VK_LAYER_LUNARG_standard_validation",
	#endif
		};

		std::vector<const char*> instance_extensions{
		  VK_KHR_SURFACE_EXTENSION_NAME,
		  VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	#ifdef DEBUG
		  VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
	#endif
		};

		auto vulkan = std::make_shared<VulkanInstance>(
			"Innovator",
			instance_layers,
			instance_extensions);

#ifdef DEBUG
		auto debugcb = std::make_unique<VulkanDebugCallback>(
			vulkan,
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_DEBUG_BIT_EXT);
#endif

		std::vector<const char*> device_layers{
	  #ifdef DEBUG
			"VK_LAYER_LUNARG_standard_validation",
	  #endif
		};

		std::vector<const char*> device_extensions{
		  VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		VkPhysicalDeviceFeatures device_features;
		::memset(&device_features, VK_FALSE, sizeof(VkPhysicalDeviceFeatures));
		device_features.sparseBinding = VK_TRUE;
		device_features.sparseResidencyImage2D = VK_TRUE;
		device_features.sparseResidencyImage3D = VK_TRUE;

		auto device = std::make_shared<VulkanDevice>(
			vulkan,
			device_features,
			device_layers,
			device_extensions);

		auto surface = std::make_shared<::VulkanSurface>(vulkan, this->hWnd, this->hInstance);
		VkSurfaceCapabilitiesKHR surface_capabilities = surface->getSurfaceCapabilities(device);
		VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

		surface->checkPresentModeSupport(device, present_mode);
		VkSurfaceFormatKHR surface_format = surface->getSupportedSurfaceFormat(device, color_attachment->format);

		auto swapchain = std::make_shared<SwapchainObject>(
			color_attachment,
			surface,
			surface_format,
			present_mode);

		this->context = std::make_shared<Context>(
			vulkan,
			device,
			surface_capabilities.currentExtent);

		this->root = std::make_shared<Separator>();
		this->root->children = {
		  scene,
		  swapchain
		};

		allocvisitor.visit(this->root.get(), this->context.get());
		pipelinevisitor.visit(this->root.get(), this->context.get());
		recordvisitor.visit(this->root.get(), this->context.get());
	}


	void redraw() override
	{
		try {
			rendervisitor.visit(this->root.get(), this->context.get());
			presentvisitor.visit(this->root.get(), this->context.get());
		}
		catch (VkException&) {
			// recreate swapchain, try again next frame
		}
	}

	void resize(int width, int height) override
	{
		VkExtent2D extent{
		  static_cast<uint32_t>(width),
		  static_cast<uint32_t>(height)
		};
		this->context->resize(extent);

		resizevisitor.visit(this->root.get(), this->context.get());
		recordvisitor.visit(this->root.get(), this->context.get());
		this->redraw();
	}

	void mousePressed(int x, int y, int button)
	{
		eventvisitor.press = true;
		eventvisitor.button = button;
		eventvisitor.currpos = glm::dvec2(x, y);
		this->root->visit(&eventvisitor, this->context.get());
		eventvisitor.prevpos = eventvisitor.currpos;
	}

	void mouseReleased() override
	{
		eventvisitor.press = false;
		this->root->visit(&eventvisitor, this->context.get());
		eventvisitor.prevpos = eventvisitor.currpos;
	}

	void mouseMoved(int x, int y)
	{
		eventvisitor.move = true;
		eventvisitor.currpos = glm::dvec2(x, y);
		this->root->visit(&eventvisitor, this->context.get());
		eventvisitor.prevpos = eventvisitor.currpos;
		eventvisitor.move = false;

		if (this->context->redraw) {
			this->redraw();
		}
	}

	std::shared_ptr<Group> root;
	std::shared_ptr<Context> context;
};
