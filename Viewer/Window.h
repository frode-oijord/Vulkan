#pragma once

#include <Innovator/Nodes.h>
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

		if (!this->hWnd) {
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

	VulkanWindow(std::shared_ptr<Node> scene)
	{
		devicevisitor.visit(scene.get());

		devicevisitor.instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		devicevisitor.instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		devicevisitor.device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

#ifdef DEBUG
		//devicevisitor.device_layers.push_back("VK_LAYER_KHRONOS_validation");
		//devicevisitor.instance_layers.push_back("VK_LAYER_KHRONOS_validation");
		devicevisitor.instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		devicevisitor.instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		auto vulkan = std::make_shared<VulkanInstance>(
			"Innovator",
			devicevisitor.instance_layers,
			devicevisitor.instance_extensions);

#ifdef DEBUG
		auto debugcb = std::make_unique<VulkanDebugCallback>(
			vulkan,
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_DEBUG_BIT_EXT);
#endif

		auto device = std::make_shared<VulkanDevice>(
			vulkan,
			devicevisitor.device_features2,
			devicevisitor.device_layers,
			devicevisitor.device_extensions);

		auto surface = std::make_shared<VulkanSurface>(
			vulkan,
			this->hWnd,
			this->hInstance);

		auto swapchain = std::make_shared<SwapchainObject>(
			surface,
			VK_PRESENT_MODE_FIFO_KHR);

		VkSurfaceCapabilitiesKHR surface_capabilities = surface->getSurfaceCapabilities(device);
		this->extent = std::make_shared<VkExtent2D>(surface_capabilities.currentExtent);

		InitVisitors(vulkan, device, this->extent);

		this->root = std::make_shared<Separator>();
		this->root->children = {
		  scene,
		  swapchain
		};

		allocvisitor.visit(this->root.get());
		pipelinevisitor.visit(this->root.get());
		recordvisitor.visit(this->root.get());
	}


	void redraw() override
	{
		try {
			rendervisitor.visit(this->root.get());
			presentvisitor.visit(this->root.get());
		}
		catch (VkException& e) {
			std::cerr << e.what() << std::endl;
			// recreate swapchain, try again next frame
		}
	}

	void resize(int width, int height) override
	{
		*this->extent = VkExtent2D{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		resizevisitor.visit(this->root.get());
		recordvisitor.visit(this->root.get());
		this->redraw();
	}

	void mousePressed(int x, int y, int button) override
	{
		eventvisitor.mousePressed(this->root.get(), x, y, button);
	}

	void mouseReleased() override
	{
		eventvisitor.mouseReleased(this->root.get());
	}

	void mouseMoved(int x, int y)
	{
		eventvisitor.mouseMoved(this->root.get(), x, y);
		this->redraw();
	}

	std::shared_ptr<Group> root;
	std::shared_ptr<VkExtent2D> extent;
};
