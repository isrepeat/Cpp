#pragma once
#include <Helpers/HWindows.h>
#include <Helpers/Signal.h>
#include <Helpers/Math.h>

#include <condition_variable>
#include <functional>
#include <thread>

namespace DxDesktop {
	class MainWindow {
	public:
		MainWindow(HINSTANCE hInstance, int width, int height, int x = 100, int y = 100);
		~MainWindow();

		HWND GetHwnd();
		H::Size GetSize();

		void StartMessageListening();

		H::Signal<void()> eventQuit;
		H::Signal<void(H::Size)> eventWindowSizeChanged;

	private:
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

	private:
		std::mutex mx;

		HINSTANCE hInstance;
		H::Size windowSize;

		HWND hWnd;
		std::atomic<bool> isThreadsStopped = true;
	};
}