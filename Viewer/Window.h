#pragma once

#include <Innovator/Nodes.h>
#include <Innovator/Defines.h>
#include <Innovator/VulkanRenderer.h>

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
	virtual ~VulkanWindow() = default;

	VulkanWindow(std::shared_ptr<Node> scene) :
		renderer(std::make_unique<VulkanRenderer>(scene, hWnd, hInstance))
	{}


	void redraw() override
	{
		this->renderer->redraw();
	}

	void resize(int width, int height) override
	{
		this->renderer->resize(width, height);
	}

	void mousePressed(int x, int y, int button) override
	{
		this->renderer->mousePressed(x, y, button);
	}

	void mouseReleased() override
	{
		this->renderer->mouseReleased();
	}

	void mouseMoved(int x, int y)
	{
		this->renderer->mouseMoved(x, y);
	}

	std::unique_ptr<VulkanRenderer> renderer;
};
