#pragma once

#include <Innovator/Nodes.h>
#include <Innovator/Defines.h>

#include <glm/glm.hpp>

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

class Window {
public:
	static inline TCHAR szWindowClass[] = _T("DesktopApp");
	
	class Init {
	public:
		Init()
		{
			HINSTANCE hInstance = GetModuleHandle(NULL);

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
		}
	};

	Window(int width, int height)
	{
		static Init init;
		this->hInstance = GetModuleHandle(NULL);

		this->hWnd = CreateWindow(
			szWindowClass,
			_T("Vulkan 3D Window App"),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			width, height,
			NULL,
			NULL,
			this->hInstance,
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
	virtual void keyPressed(int key) = 0;

	LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message) {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			this->redraw();
		}
			break;
		case WM_SIZE:
			this->resize(LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE) {
				this->keyPressed(0);
			}
			else if (wParam == VK_SPACE) {
				this->keyPressed(1);
			}
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
	virtual ~VulkanWindow() = default;

	VulkanWindow(VkExtent2D extent, std::shared_ptr<Node> scene) :
		Window(extent.width, extent.height)
	{
		devicevisitor.instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		devicevisitor.instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		devicevisitor.instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifdef DEBUG
		devicevisitor.instance_layers.push_back("VK_LAYER_KHRONOS_validation");
		devicevisitor.instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		devicevisitor.instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		devicevisitor.visit(scene.get());

		state->vulkan = std::make_shared<VulkanInstance>(
			"Innovator",
			devicevisitor.instance_layers,
			devicevisitor.instance_extensions);

#ifdef DEBUG
		auto debugcb = std::make_unique<VulkanDebugCallback>(
			state->vulkan,
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_DEBUG_BIT_EXT);
#endif

#ifdef DEBUG
		devicevisitor.device_layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
		devicevisitor.device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		state->device = std::make_shared<VulkanDevice>(
			state->vulkan,
			devicevisitor.getDeviceFeatures(),
			devicevisitor.device_layers,
			devicevisitor.device_extensions);

		state->pipelinecache = std::make_shared<VulkanPipelineCache>(state->device);
		state->fence = std::make_shared<VulkanFence>(state->device);
		state->default_command = std::make_shared<VulkanCommandBuffers>(state->device);
		state->queue = state->device->getQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

		surface = std::make_shared<VulkanSurface>(
			state->vulkan,
			this->hWnd,
			this->hInstance);

		auto swapchain = std::make_shared<Swapchain>(surface, VK_PRESENT_MODE_FIFO_KHR);

		this->scene = std::make_shared<Group>();
		this->scene->children = {
			scene,
			swapchain
		};

		VkSurfaceCapabilitiesKHR surface_capabilities = surface->getSurfaceCapabilities(state->device);
		state->extent = VkExtent3D{
			surface_capabilities.currentExtent.width,
			surface_capabilities.currentExtent.height,
			1
		};

		allocvisitor.visit(this->scene.get());
		pipelinevisitor.visit(this->scene.get());
	}

	void redraw() override
	{
		try {
			rendervisitor.visit(this->scene.get());
			presentvisitor.visit(this->scene.get());
		}
		catch (VkErrorOutOfDateException& e) {
			std::cerr << e.what() << std::endl;
			// recreate swapchain, try again next frame
		}
	}

	void resize(int width, int height) override
	{
		state->extent = VkExtent3D{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height),
			1
		};

		resizevisitor.visit(this->scene.get());
		recordvisitor.visit(this->scene.get());
		this->redraw();
	}

	void mousePressed(int x, int y, int button) override
	{
		eventvisitor.mousePressed(this->scene.get(), x, y, button);
	}

	void mouseReleased() override
	{
		eventvisitor.mouseReleased(this->scene.get());
	}

	void mouseMoved(int x, int y) override
	{
		eventvisitor.mouseMoved(this->scene.get(), x, y);
		if (eventvisitor.press) {
			this->redraw();
		}
	}

	void keyPressed(int key) override
	{
		eventvisitor.keyPressed(key);
	}

	std::shared_ptr<Group> scene;
	std::shared_ptr<VulkanSurface> surface;
};
