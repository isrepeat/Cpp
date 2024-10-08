#include "pch.h"
#include "Player.h"
#include "Audio/AudioClientFactory.h"
#include <MediaEngine/AvReader.h>
#include <Helpers/Com/UnknownValue.h>

#define LOG_DEBUG_D(fmt, ...)

using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input::Inking;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Interop;

namespace MediaEngineWinRT {
	Player::Player(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanelXaml)
		: dxSettings{ ref new Helpers::WinRt::Dx::DxSettings() }
		, swapChainPanelWinRt{ this->CreateSwapChainPanelWinRt(swapChainPanelXaml) }
		, swapChainPanelNative{ H::Dx::WinRt::Tools::QuerySwapChainPanelNative(this->swapChainPanelWinRt->GetSwapChainPanelNativeAsObject()) }
		, videoSceneRenderer{ this->swapChainPanelNative }
	{
		lg::DefaultLoggers::Init("VideoPlayerPrototype.log", lg::InitFlags::DefaultFlags | lg::InitFlags::CreateInPackageFolder);

		// Setup control / application event handlers
		swapChainPanelXaml->SizeChanged += ref new SizeChangedEventHandler(
			this, &Player::OnSizeChanged
		);
		swapChainPanelXaml->CompositionScaleChanged += ref new TypedEventHandler<Windows::UI::Xaml::Controls::SwapChainPanel^, Object^>(
			this, &Player::OnCompositionScaleChanged
			);
		this->swapChainPanelWinRt->SetSwapChainPanelXaml(swapChainPanelXaml);



		auto dispatcher = Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher;

		this->fileOpenAction = dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([&] {
			auto openPicker = ref new Windows::Storage::Pickers::FileOpenPicker();
			openPicker->SuggestedStartLocation = Windows::Storage::Pickers::PickerLocationId::DocumentsLibrary;
			openPicker->FileTypeFilter->Append(".mp4");
			openPicker->FileTypeFilter->Append(".mkv");
			openPicker->FileTypeFilter->Append(".ts");

			concurrency::create_task(openPicker->PickSingleFileAsync()).then([&](Windows::Storage::StorageFile^ file) {
				if (file) {
					concurrency::create_task(file->OpenAsync(Windows::Storage::FileAccessMode::Read)).then([&](IRandomAccessStream^ stream) {
						this->OpenMedia(stream);
						});
				}
				});
			}));


	}

	Player::~Player() {
		if (this->renderLoopWorker) {
			this->renderLoopWorker->Cancel();
		}
	}

	void Player::OpenMedia(Windows::Storage::Streams::IRandomAccessStream^ stream) {
		auto avReader = std::make_unique<AvReader>(reinterpret_cast<IStream*>(stream), this->swapChainPanelNative->GetDxDevice());
		this->videoSceneRenderer.Init(std::move(avReader));

		auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction^ action) {
			while (action->Status == AsyncStatus::Started) {
				auto tpStart = std::chrono::high_resolution_clock::now();
				Concurrency::critical_section::scoped_lock lock(this->criticalSection);

				{
					auto dxDev = this->swapChainPanelNative->GetDxDevice()->Lock();
					auto dxCtx = dxDev->LockContext();
					auto d3dCtx = dxCtx->D3D();

					// Reset the viewport to target the whole screen.
					auto renderTargetView = this->swapChainPanelNative->GetRenderTargetView();
					d3dCtx->ClearRenderTargetView(renderTargetView.Get(), DirectX::Colors::Gray);

					ID3D11RenderTargetView* pRTVs[] = { renderTargetView.Get() };
					d3dCtx->OMSetRenderTargets(1, pRTVs, nullptr);

					auto viewport = this->swapChainPanelNative->GetScreenViewport();
					d3dCtx->RSSetViewports(1, &viewport);

					this->videoSceneRenderer.Render();
				}
				
				auto tp1 = std::chrono::high_resolution_clock::now();
				
				this->swapChainPanelNative->Present();
				
				auto tp2 = std::chrono::high_resolution_clock::now();
				
				this->videoSceneRenderer.ReadNeccessaryVideoSamples();

				auto tpEnd = std::chrono::high_resolution_clock::now();
				auto dtRenderIteration = H::Chrono::milliseconds_f{ tpEnd - tpStart };
				LOG_DEBUG_D("\n"
					"dt [Render iteration]{} = {}\n"
					"dt [Render] = {}\n"
					"dt [Present] = {}\n"
					"dt [ReadSamples] = {}\n"
					, dtRenderIteration > 20ms ? " [slow]" : "", dtRenderIteration
					, tp1 - tpStart
					, tp2 - tp1
					, tpEnd - tp2
				);
			}
			});

		// Run task on a dedicated high priority background thread.
		this->renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
	}

	void Player::Play() {
		//this->mediaPlayer.Play();
	}

	void Player::Pause() {
		//this->mediaPlayer.Pause();
	}

	void Player::TogglePlayback() {
		//this->mediaPlayer.TogglePlayback();
	}

	void Player::Stop() {
		//this->mediaPlayer.Stop();
	}

	bool Player::HasVideo() {
		//return this->mediaPlayer.HasVideo();
		return true;
	}

	bool Player::HasAudio() {
		//return this->mediaPlayer.HasAudio();
		return false;
	}

	void Player::SetDynamicRenderResolution(int resolutionPercent) {
		this->swapChainPanelNative->SetRenderResolutionScale(100.0f / resolutionPercent);
	}

	//float Player::GetFPS() {
	//	return this->mediaPlayer.GetVideoRenderer().GetFPS();
	//}

	bool Player::IsPlaying() {
		//return this->mediaPlayer.IsPlaying();
		return false;
	}

	bool Player::IsPaused() {
		//return this->mediaPlayer.IsPaused();
		return true;
	}

	bool Player::IsStopped() {
		//return this->mediaPlayer.IsStopped();
		return true;
	}

	int64_t Player::GetMediaDuration() {
		//return (int64_t)this->mediaPlayer.GetMediaDuration();
		return 0;
	}

	int64_t Player::GetCurrentPosition() {
		//return (int64_t)this->mediaPlayer.GetCurrentPosition();
		return 0;
	}

	void Player::Seek(int64_t mftime) {
		//this->mediaPlayer.Seek(H::Chrono::Hns(mftime));
	}

	float Player::GetVolume() {
		//return this->mediaPlayer.GetVolume();
		return 0;
	}

	void Player::SetVolume(float volume) {
		//this->mediaPlayer.SetVolume(volume);
	}

	bool Player::IsMuted() {
		//return this->mediaPlayer.IsMuted();
		return false;
	}

	void Player::SetMuted(bool muted) {
		//this->mediaPlayer.SetMuted(muted);
	}

	void Player::OnSizeChanged(Object^ sender, SizeChangedEventArgs^ e) {
		Concurrency::critical_section::scoped_lock lock{ criticalSection };

		auto size = e->NewSize;
		auto displayInformation = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();

		this->swapChainPanelWinRt->SetLogicalSize(e->NewSize);
	}

	void Player::OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Object^ args)
	{}

	void Player::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
	{}

	void Player::OnResuming(Object^ sender, Object^ args)
	{}


	Helpers::WinRt::Dx::SwapChainPanel^ Player::CreateSwapChainPanelWinRt(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanelXaml) {
		Helpers::WinRt::Dx::SwapChainPanelInitData initData;
		initData.deviceType = Helpers::WinRt::Dx::SwapChainPanelInitData_Device::DxDeviceMF;
		initData.deviceMutexType = Helpers::WinRt::Dx::SwapChainPanelInitData_DeviceMutex::Recursive;
		initData.optionFlags = Helpers::WinRt::Dx::SwapChainPanelInitData_Options::None;

		auto swapChainPanelWinRt = ref new Helpers::WinRt::Dx::SwapChainPanel(initData /*, this->dxSettings*/);
		swapChainPanelWinRt->SetSwapChainPanelXaml(swapChainPanelXaml);
		return swapChainPanelWinRt;
	}

	std::unique_ptr<IAudioClientFactory> Player::CreateAudioClientFactory() {
		return std::make_unique<AudioClientFactory>();
	}
}