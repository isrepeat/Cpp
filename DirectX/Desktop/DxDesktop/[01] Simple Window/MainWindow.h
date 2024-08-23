#pragma once
#include <Helpers/HWindows.h>
#include <condition_variable>
#include <atomic>

namespace DxDesktop {
	class MainWindow {
	public:
		MainWindow(HINSTANCE hInstance, int width, int height);
		~MainWindow();

		void StartMessageLoop();

	private:
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

	private:
		std::mutex mx;

		HINSTANCE hInstance;
		int width;
		int height;

		HWND hWnd;
		std::atomic<bool> isMessageLoopRunning = false;
		std::condition_variable cvMessageLoop;
	};
}