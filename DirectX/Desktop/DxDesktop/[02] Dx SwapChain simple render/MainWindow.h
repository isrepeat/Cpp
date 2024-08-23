#pragma once
#include <Helpers/HWindows.h>
#include <Helpers/Signal.h>

#include <condition_variable>
#include <functional>
#include <thread>

namespace DxDesktop {
	class MainWindow {
	public:
		MainWindow(HINSTANCE hInstance, int width, int height);
		~MainWindow();

		HWND GetHwnd();
		void StartMessageListening();
		//void SetRenderHandler(std::function<void()> renderHandler);
		//void StartRender();

		H::Signal<void()> eventQuit;
		H::Signal<void(UINT, UINT)> eventWindowSizeChanged;

	private:
		//void StartRenderRoutine();

		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

	private:
		std::mutex mx;

		HINSTANCE hInstance;
		int width;
		int height;

		//std::function<void()> renderHandler;

		HWND hWnd;
		std::atomic<bool> isThreadsStopped = true;
		//std::thread renderThread;
	};
}