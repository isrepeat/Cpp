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
	auto mainWindow = DxDesktop::MainWindow{ hInstance, 1040, 540, 300, 150};

	H::Dx::SwapChainPanel::InitData swapChainPanelInitData;
	swapChainPanelInitData.environment = H::Dx::SwapChainPanel::InitData::Environment::Desktop;
	swapChainPanelInitData.hWnd = mainWindow.GetHwnd();
	swapChainPanelInitData.backBufferFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
	//swapChainPanelInitData.backBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	swapChainPanelInitData.optionFlags = H::Dx::SwapChainPanel::InitData::Options::EnableHDR;

	auto swapChainPanel = Microsoft::WRL::Make<H::Dx::SwapChainPanel>(swapChainPanelInitData);

	// TODO: put real display info
	swapChainPanel->InitSwapChainPanelInfo(
		static_cast<H::Size_f>(mainWindow.GetSize()),
		H::Dx::DisplayOrientations::Landscape,
		H::Dx::DisplayOrientations::Landscape,
		2.5f, // dpi scale factor X
		2.5f, // dpi scale factor Y
		240 // dpi value
	);


	std::atomic<bool> isRenderRunning = false;
	auto simpleSceneRenderer = DxDesktop::SimpleSceneRenderer{ swapChainPanel };

	mainWindow.eventQuit.Add([&] {
		LOG_DEBUG_D("[event] Quit");
		isRenderRunning = false;
		});

	mainWindow.eventWindowSizeChanged.Add([&](H::Size size) {
		LOG_DEBUG_D("[event] WindowSizeChanged(width {}, height = {})", size.width, size.height);
		simpleSceneRenderer.OnWindowSizeChanged(size);
		});

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