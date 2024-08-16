#include <Helpers/Dx/SwapChainPanel.h>
#include <Helpers/Logger.h>

#include <Helpers/MediaFoundation/MFLibs.h>
#include <Helpers/Dx/DxLibs.h>

#include "SimpleSceneRenderer.h"
#include "MainWindow.h"


int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE /*hPrevInstance*/,
	LPSTR /*lpCmdLine*/,
	int /*nShowCmd*/)
{
	auto mainWindow = DxDesktop::MainWindow{ hInstance, 800, 600 };

	auto swapChainPanel = Microsoft::WRL::Make<H::Dx::SwapChainPanel>(
		H::Dx::SwapChainPanel::Environment::Desktop,
		Callback<void, IDXGISwapChain3*>{},
		mainWindow.GetHwnd()
		);
	swapChainPanel->InitSwapChainPanelInfo(
		H::Size_f{ 0, 0 },
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

	mainWindow.eventWindowSizeChanged.Add([&] (UINT width, UINT height) {
		LOG_DEBUG_D("[event] WindowSizeChanged(width {}, height = {})", width, height);
		swapChainPanel->SetLogicalSize(H::Size_f{
			static_cast<float>(width),
			static_cast<float>(height)
			});
		});


	auto simpleSceneRenderer = DxDesktop::SimpleSceneRenderer{ swapChainPanel };

	isRenderRunning = true;
	auto renderThread = std::thread([&] {
		while (isRenderRunning) {
			//auto dxDev = swapChainPanel->GetDxDevice()->Lock();
			//auto dxCtx = dxDev->LockContext();
			//auto d3dCtx = dxCtx->D3D();

			simpleSceneRenderer.Render();
			swapChainPanel->Present();
		}
		});

	mainWindow.StartMessageListening();

	if (renderThread.joinable())
		renderThread.join();

	return 0;
}