#include "pch.h"
#include "DxPlayerMain.h"
#include "Common\DirectXHelper.h"

#if ENGINE_TYPE == MEDIA_ENDIGNE_LOGIC
using namespace DxPlayer;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::Streams;
using namespace Concurrency;


// Loads and initializes application assets when the application is loaded.
DxPlayerMain::DxPlayerMain(Helpers::WinRt::Dx::SwapChainPanel^ swapChainPanelWinRt)
	: swapChainPanelWinRt{ swapChainPanelWinRt }
	, swapChainPanelNative{ H::Dx::WinRt::Tools::QuerySwapChainPanelNative(swapChainPanelWinRt->GetSwapChainPanelNativeAsObject()) }
	, m_pointerLocationX{ 0.0f }
{
	// Register to be notified if the Device is lost or recreated
	auto dxDevResources = this->swapChainPanelNative->GetDxDevice()->Lock();
	//dxDevResources->RegisterDeviceNotify(this);

	// TODO: Replace this with your app's content initialization.
	m_sceneRenderer = std::unique_ptr<VideoSceneRenderer>(new VideoSceneRenderer(this->swapChainPanelNative));

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

DxPlayerMain::~DxPlayerMain()
{
	//// Deregister device notification
	//auto dxDevResources = m_deviceResources->Lock();
	//dxDevResources->RegisterDeviceNotify(nullptr);
}

// Updates application state when the window size changes (e.g. device orientation change)
void DxPlayerMain::CreateWindowSizeDependentResources()
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

void DxPlayerMain::StartRenderLoop() {
	//if (m_openFileAction && m_openFileAction->Status == AsyncStatus::Started) {
	if (m_openFileAction) {
		return;
	}

	// If the animation render loop is already running then do not start another thread.
	if (m_renderLoopWorker && m_renderLoopWorker->Status == AsyncStatus::Started) {
		return;
	}

	auto dispatcher = Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher;

	m_openFileAction = dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([this] {
		auto openPicker = ref new FileOpenPicker();
		openPicker->SuggestedStartLocation = PickerLocationId::DocumentsLibrary;
		openPicker->FileTypeFilter->Append(".mp4");
		openPicker->FileTypeFilter->Append(".mkv");
		openPicker->FileTypeFilter->Append(".ts");

		concurrency::create_task(openPicker->PickSingleFileAsync()).then([this](StorageFile^ file) {
			if (file) {
				concurrency::create_task(file->OpenAsync(Windows::Storage::FileAccessMode::ReadWrite)).then([this](IRandomAccessStream^ stream) {
					m_openFileAction = nullptr;

					auto avReader = std::make_unique<AvReader>(reinterpret_cast<IStream*>(stream), this->swapChainPanelNative->GetDxDevice());
					m_sceneRenderer->Init(std::move(avReader));

					auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction^ action) {
						Update();

						while (action->Status == AsyncStatus::Started) {
							auto tpStart = std::chrono::high_resolution_clock::now();
							critical_section::scoped_lock lock(m_criticalSection);
							auto tpA1 = std::chrono::high_resolution_clock::now();

							// Calculate the updated frame and render once per vertical blanking interval.
							//Update();
							auto rendered = Render();

							auto tpA2 = std::chrono::high_resolution_clock::now();
							std::chrono::steady_clock::time_point tpB1;
							std::chrono::steady_clock::time_point tpB2;
							if (rendered) {
								tpB1 = std::chrono::high_resolution_clock::now();
								this->swapChainPanelNative->Present();
								tpB2 = std::chrono::high_resolution_clock::now();
							}

							auto tpEnd = std::chrono::high_resolution_clock::now();
							auto dtRenderIteration = H::Chrono::milliseconds_f{ tpEnd - tpStart };
							LOG_DEBUG_D("\n"
								"dt [Render iteration]{} = {}\n"
								"dt [Render] = {}\n"
								"dt [Present] = {}\n"
								, dtRenderIteration > 20ms ? " [slow]" : "", dtRenderIteration
								, tpA2 - tpA1
								, tpB2 - tpB1
							);
						}
						});

					// Run task on a dedicated high priority background thread.
					m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
					});
			}
			});
		}));
}


void DxPlayerMain::StopRenderLoop()
{
	if (m_renderLoopWorker) {
		m_renderLoopWorker->Cancel();
	}

	if (m_openFileAction) {
		m_openFileAction->Cancel();
	}
}


// Updates the application state once per frame.
void DxPlayerMain::Update()
{
	ProcessInput();

	// Update scene objects.
	m_timer.Tick([&]()
		{
		});
}

// Process all input from the user before updating game state
void DxPlayerMain::ProcessInput()
{
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool DxPlayerMain::Render()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	//auto dxDevResources = this->swapChainPanelNative->GetDxDevice()->Lock();
	//auto dxCtx = dxDevResources->LockContext();
	//auto d3dCtx = dxCtx->D3D();

	//// Reset the viewport to target the whole screen.
	//auto viewport = this->swapChainPanelNative->GetScreenViewport();
	//d3dCtx->RSSetViewports(1, &viewport);

	//// Reset render targets to the screen.
	//ID3D11RenderTargetView* const targets[1] = { this->swapChainPanelNative->GetBackBufferRenderTargetView() };
	//d3dCtx->OMSetRenderTargets(1, targets, this->swapChainPanelNative->GetDepthStencilView());

	//// Clear the back buffer and depth stencil view.
	//d3dCtx->ClearRenderTargetView(this->swapChainPanelNative->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	//d3dCtx->ClearDepthStencilView(this->swapChainPanelNative->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render the scene objects.
	// TODO: Replace this with your app's content rendering functions.
	m_sceneRenderer->Render();

	return true;
}

// Notifies renderers that device resources need to be released.
void DxPlayerMain::OnDeviceLost()
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void DxPlayerMain::OnDeviceRestored()
{
	m_sceneRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

#elif ENGINE_TYPE == DX_PLAYER_RENDER

using namespace DxPlayer;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::Streams;
using namespace Concurrency;

// Loads and initializes application assets when the application is loaded.
DxPlayerMain::DxPlayerMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources), m_pointerLocationX(0.0f)
{
	// Register to be notified if the Device is lost or recreated
	auto dxDevResources = m_deviceResources->Lock();
	dxDevResources->RegisterDeviceNotify(this);

	// TODO: Replace this with your app's content initialization.
	m_sceneRenderer = std::unique_ptr<Sample3DSceneRenderer>(new Sample3DSceneRenderer(m_deviceResources));

	m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

DxPlayerMain::~DxPlayerMain()
{
	// Deregister device notification
	auto dxDevResources = m_deviceResources->Lock();
	dxDevResources->RegisterDeviceNotify(nullptr);
}

// Updates application state when the window size changes (e.g. device orientation change)
void DxPlayerMain::CreateWindowSizeDependentResources() 
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

#if VIDEO_RENDERER
void DxPlayerMain::StartRenderLoop() {
	//if (m_openFileAction && m_openFileAction->Status == AsyncStatus::Started) {
	if (m_openFileAction) {
		return;
	}

	// If the animation render loop is already running then do not start another thread.
	if (m_renderLoopWorker && m_renderLoopWorker->Status == AsyncStatus::Started) {
		return;
	}

	auto dispatcher = Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher;

	m_openFileAction = dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([this] {
		auto openPicker = ref new FileOpenPicker();
		openPicker->SuggestedStartLocation = PickerLocationId::DocumentsLibrary;
		openPicker->FileTypeFilter->Append(".mp4");
		openPicker->FileTypeFilter->Append(".mkv");
		openPicker->FileTypeFilter->Append(".ts");

		concurrency::create_task(openPicker->PickSingleFileAsync()).then([this](StorageFile^ file) {
			if (file) {
				concurrency::create_task(file->OpenAsync(Windows::Storage::FileAccessMode::ReadWrite)).then([this](IRandomAccessStream^ stream) {
					m_openFileAction = nullptr;

					auto avReader = std::make_unique<AvReader>(reinterpret_cast<IStream*>(stream), m_deviceResources);
					m_sceneRenderer->Init(std::move(avReader));

					auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction^ action) {
						while (action->Status == AsyncStatus::Started) {
							auto tpStart = std::chrono::high_resolution_clock::now();
							critical_section::scoped_lock lock(m_criticalSection);
							auto tpA1 = std::chrono::high_resolution_clock::now();
							
							// Calculate the updated frame and render once per vertical blanking interval.
							Update();
							auto rendered = Render();

							auto tpA2 = std::chrono::high_resolution_clock::now();
							std::chrono::steady_clock::time_point tpB1;
							std::chrono::steady_clock::time_point tpB2;
							if (rendered) {
								tpB1 = std::chrono::high_resolution_clock::now();
								auto dxDevResources = m_deviceResources->Lock();
								dxDevResources->Present();
								tpB2 = std::chrono::high_resolution_clock::now();
							}

							auto tpEnd = std::chrono::high_resolution_clock::now();
							auto dtRenderIteration = H::Chrono::milliseconds_f{ tpEnd - tpStart };
							//LOG_DEBUG_D("\n"
							//	"dt [Render iteration]{} = {}\n"
							//	"dt [Render] = {}\n"
							//	"dt [Present] = {}\n"
							//	, dtRenderIteration > 20ms ? " [slow]" : "", dtRenderIteration
							//	, tpA2 - tpA1
							//	, tpB2 - tpB1
							//);
						}
						});

					// Run task on a dedicated high priority background thread.
					m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
					});
			}
			});
		}));
}
#else
void DxPlayerMain::StartRenderLoop() {
	// If the animation render loop is already running then do not start another thread.
	if (m_renderLoopWorker && m_renderLoopWorker->Status == AsyncStatus::Started) {
		return;
	}

	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction^ action) {
		while (action->Status == AsyncStatus::Started) {
			critical_section::scoped_lock lock(m_criticalSection);
			// Calculate the updated frame and render once per vertical blanking interval.
			Update();
			if (Render()) {
				auto dxDevResources = m_deviceResources->Lock();
				dxDevResources->Present();
			}
		}
		});

	// Run task on a dedicated high priority background thread.
	m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}
#endif

void DxPlayerMain::StopRenderLoop()
{
	if (m_renderLoopWorker) {
		m_renderLoopWorker->Cancel();
	}

	if (m_openFileAction) {
		m_openFileAction->Cancel();
	}
}


// Updates the application state once per frame.
void DxPlayerMain::Update() 
{
	ProcessInput();

	// Update scene objects.
	m_timer.Tick([&]()
	{
		// TODO: Replace this with your app's content update functions.
		m_sceneRenderer->Update(m_timer);
		m_fpsTextRenderer->Update(m_timer);
	});
}

// Process all input from the user before updating game state
void DxPlayerMain::ProcessInput()
{
	// TODO: Add per frame input handling here.
	m_sceneRenderer->TrackingUpdate(m_pointerLocationX);
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool DxPlayerMain::Render() 
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto dxDevResources = m_deviceResources->Lock();
	auto d3dCtx = dxDevResources->GetD3DDeviceContext();

	// Reset the viewport to target the whole screen.
	auto viewport = dxDevResources->GetScreenViewport();
	d3dCtx->RSSetViewports(1, &viewport);

	// Reset render targets to the screen.
	ID3D11RenderTargetView *const targets[1] = { dxDevResources->GetBackBufferRenderTargetView() };
	d3dCtx->OMSetRenderTargets(1, targets, dxDevResources->GetDepthStencilView());

	// Clear the back buffer and depth stencil view.
	d3dCtx->ClearRenderTargetView(dxDevResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	d3dCtx->ClearDepthStencilView(dxDevResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render the scene objects.
	// TODO: Replace this with your app's content rendering functions.
	m_sceneRenderer->Render();
	m_fpsTextRenderer->Render();

	return true;
}

// Notifies renderers that device resources need to be released.
void DxPlayerMain::OnDeviceLost()
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void DxPlayerMain::OnDeviceRestored()
{
	m_sceneRenderer->CreateDeviceDependentResources();
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}
#endif