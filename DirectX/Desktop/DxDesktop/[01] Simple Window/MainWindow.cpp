#include "MainWindow.h"
#include <Helpers/Logger.h>
#include <exception>

namespace DxDesktop {
	MainWindow::MainWindow(HINSTANCE hInstance, int width, int height)
		: hInstance{ hInstance }
		, width{ width }
		, height{ height }
	{
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = &MainWindow::WindowProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = this->hInstance;
		wcex.hIcon = LoadIconW(this->hInstance, IDI_APPLICATION);
		wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = L"DxWin32_MainWindow_class";
		wcex.hIconSm = LoadIconW(wcex.hInstance, IDI_APPLICATION);

		if (!RegisterClassExW(&wcex)) {
			throw std::exception("RegisterClassEx fail");
		}

		this->hWnd = CreateWindowW(
			wcex.lpszClassName,
			L"Main window",
			WS_OVERLAPPEDWINDOW,
			100,
			100,
			this->width,
			this->height,
			nullptr,
			nullptr,
			this->hInstance,
			this
		);

		if (!this->hWnd) {
			throw std::exception("CreateWindowW fail");
		}

		ShowWindow(this->hWnd, SW_SHOW /*nCmdShow*/);
		UpdateWindow(this->hWnd);
	}

	MainWindow::~MainWindow() {
		//this->isMessageLoopRunning = false;
		
		//std::unique_lock lk{ mx };
		//this->cvMessageLoop.wait(lk);
	}


	void MainWindow::StartMessageLoop() {
		this->isMessageLoopRunning = true;
	    while (this->isMessageLoopRunning) {
	        MSG msg = {};
	        while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) { // WM_QUIT should be handle here, window message queue thread (WindowProc) not receive it.
					this->isMessageLoopRunning = false;
					break;
				}
	            TranslateMessage(&msg);
	            DispatchMessageW(&msg);
	        }
			
			// Render ...
	    }

		//this->cvMessageLoop.notify_all();
	}


	LRESULT MainWindow::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		//LOG_DEBUG_D("WindowProc(hwnd, message = {:#x})", message);
		MainWindow* pThis = nullptr;

		if (message == WM_NCCREATE) {
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			pThis = (MainWindow*)pCreate->lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->hWnd = hWnd;
		}
		else {
			pThis = (MainWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		}

		if (pThis) {
			pThis->HandleMessage(message, wParam, lParam);
		}
		else {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

	LRESULT MainWindow::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			EndPaint(hWnd, &ps);
			break;
		}

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
}