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
		this->isThreadsStopped = true;
		PostQuitMessage(0); // ensure PeekMessageW return control

		//if (this->renderThread.joinable())
		//	this->renderThread.join();
	}

	HWND MainWindow::GetHwnd() {
		return this->hWnd;
	}

	// Must be called in main thread
	void MainWindow::StartMessageListening() {
		this->isThreadsStopped = false;
		while (!this->isThreadsStopped) {
			MSG msg = {};
			while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) { // WM_QUIT should be handle here, window message queue thread (WindowProc) not receive it.
					this->isThreadsStopped = true;
					this->eventQuit();
					break;
				}
				TranslateMessage(&msg);
				DispatchMessageW(&msg); // forward msg to WindowProc
			}
		}
	}



	//void MainWindow::SetRenderHandler(std::function<void()> renderHandler) {
	//	this->renderHandler = renderHandler;
	//}

	//void MainWindow::StartRender() {
	//	this->isThreadsStopped = false;
	//	this->StartRenderRoutine();
	//	this->StartMessageListening();
	//}

	//void MainWindow::StartRenderRoutine() {
	//	this->renderThread = std::thread([this] {
	//		while (!this->isThreadsStopped) {
	//			if (this->renderHandler) {
	//				this->renderHandler();
	//			}
	//		}
	//		});
	//}




	LRESULT MainWindow::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
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
			return pThis->HandleMessage(message, wParam, lParam);
		}
		else {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

	LRESULT MainWindow::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		//case WM_PAINT:
		//	break;

		case WM_SIZE: {
			UINT width = LOWORD(lParam);
			UINT height = HIWORD(lParam);
			this->eventWindowSizeChanged(width, height);
			return 0;
		}

		case WM_DESTROY:
			PostQuitMessage(0);
			return 1;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
}