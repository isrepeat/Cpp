#include "MainWindow.h"

#if COMPILE_FOR_DESKTOP
#include <Helpers/Logger.h>
#include <exception>

namespace HELPERS_NS {
	namespace Win32 {
		MainWindow::MainWindow(
			HINSTANCE hInstance,
			int width,
			int height,
			int x,
			int y,
			DWORD windowStyle)
			: hInstance{ hInstance }
			, windowSize{ width, height }
		{
			WNDCLASSEX wcex;
			wcex.cbSize = sizeof(WNDCLASSEX);

			wcex.style = CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc = &MainWindow::WindowProc;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = 0;
			wcex.hInstance = this->hInstance;
			wcex.hIcon = ::LoadIconW(this->hInstance, IDI_APPLICATION);
			wcex.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
			wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wcex.lpszMenuName = nullptr;
			wcex.lpszClassName = L"DxWin32_MainWindow_class";
			wcex.hIconSm = ::LoadIconW(wcex.hInstance, IDI_APPLICATION);

			if (!::RegisterClassExW(&wcex)) {
				LOG_ERROR_D("RegisterClassExW failed");
				LogLastError;
				throw std::exception("RegisterClassEx fail");
			}

			this->hWnd = CreateWindowW(
				wcex.lpszClassName,
				L"Main window",
				windowStyle,
				x,
				y,
				this->windowSize.width,
				this->windowSize.height,
				nullptr,
				nullptr,
				this->hInstance,
				this
			);

			if (!this->hWnd) {
				LOG_ERROR_D("CreateWindowW failed, this->hWnd == nullptr.");
				LogLastError;
				throw std::exception("CreateWindowW fail");
			}
		}


		MainWindow::~MainWindow() {
			::PostQuitMessage(0); // posts WM_QUIT to end message loop
		}


		HWND MainWindow::GetHwnd() {
			return this->hWnd;
		}


		H::Size MainWindow::GetSize() {
			std::lock_guard lk{ mx };
			return this->windowSize;
		}


		void MainWindow::AddMessageHook(const MessageHandler& handler) {
			this->messageHooks.push_back(handler);
		}


		// Must be called in main thread
		void MainWindow::RunMessageLoop() {
			MSG msg = {};

			// GetMessage returns:
			//  > 0 — a message was retrieved (msg filled), continue processing
			//    0 — WM_QUIT received, time to exit the message loop
			//   -1 — an error occurred (invalid HWND, system failure, etc.)
			while (::GetMessageW(&msg, nullptr, 0, 0) > 0) {
				::TranslateMessage(&msg);
				::DispatchMessageW(&msg); // forward msg to WindowProc
			}

			this->eventQuit.Invoke();
		}

		void MainWindow::Show() {
			::ShowWindow(this->hWnd, SW_SHOW);
			::UpdateWindow(this->hWnd);
		}

		void MainWindow::Hide() {
			::ShowWindow(this->hWnd, SW_HIDE);
		}



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
			for (const auto& messageHook : this->messageHooks) {
				if (messageHook(message, wParam, lParam)) {
					return 0; // Сообщение обработано хук-функцией
				}
			}

			switch (message) {
				//case WM_PAINT:
				//	break;

			case WM_SIZE: {
				{
					std::lock_guard lk{ mx };
					this->windowSize.width = LOWORD(lParam);
					this->windowSize.height = HIWORD(lParam);
				}
				this->eventWindowSizeChanged.Invoke(this->windowSize);
				return 0;
			}

			case WM_DESTROY:
				::PostQuitMessage(0);
				return 1;

			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
	}
}
#endif // COMPILE_FOR_DESKTOP