#include <Helpers/Dx/SwapChainPanel.h>
#include <Helpers/Logger.h>

#include <Helpers/MediaFoundation/MFLibs.h>
#include <Helpers/Dx/DxLibs.h>

#include "SimpleSceneRenderer.h"
#include "MainWindow.h"


int Run(HINSTANCE hInstance);

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE /*hPrevInstance*/,
	LPSTR /*lpCmdLine*/,
	int /*nShowCmd*/)
{
	// NOTE: Initialize the library at the beginning to avoid implicit initialization and releasing, 
	//		 which will cause the problem of COM ojects destructions.
	CoInitialize(nullptr);

	// Wrap all work in "Run" stack to guarantee that all COM objects will destroyed before CoUnitialize
	// (or wrap CoInit / CoUninit to RAII class and place at top of the stack)
	Run(hInstance);

	CoUninitialize();
	return 0;
}

int Run(HINSTANCE hInstance) {
	auto mainWindow = DxDesktop::MainWindow{ hInstance, 800, 600 };
	auto swapChainPanel = Microsoft::WRL::Make<H::Dx::SwapChainPanel>(
		H::Dx::SwapChainPanel::Environment::Desktop,
		Callback<void, IDXGISwapChain3*>{},
		mainWindow.GetHwnd()
		);

	auto mainWindowSize = mainWindow.GetSize();
	swapChainPanel->InitSwapChainPanelInfo(
		static_cast<H::Size_f>(mainWindowSize),
		H::Dx::DisplayOrientations::Landscape,
		H::Dx::DisplayOrientations::Landscape,
		2.5f,
		2.5f,
		240
	);

	std::atomic<bool> isRenderRunning = false;

	mainWindow.eventQuit.Add([&] {
		LOG_DEBUG_D("[event] Quit");
		isRenderRunning = false;
		});

	mainWindow.eventWindowSizeChanged.Add([&](H::Size size) {
		LOG_DEBUG_D("[event] WindowSizeChanged(width {}, height = {})", size.width, size.height);
		swapChainPanel->SetLogicalSize(static_cast<H::Size_f>(size));
		});


	auto simpleSceneRenderer = DxDesktop::SimpleSceneRenderer{ swapChainPanel };

	isRenderRunning = true;
	auto renderThread = std::thread([&] {
		while (isRenderRunning) {
			Sleep(15);
			simpleSceneRenderer.Tick();
			swapChainPanel->Present();
		}
		});

	mainWindow.StartMessageListening();

	if (renderThread.joinable())
		renderThread.join();
	
	return 0;
}