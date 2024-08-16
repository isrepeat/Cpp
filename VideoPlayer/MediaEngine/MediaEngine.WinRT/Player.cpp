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
	//Player::Player(Windows::UI::Xaml::Controls::SwapChainPanel^ panel)
	//	: mediaPlayer{ this->CreateAudioClientFactory(), H::Size{ static_cast<UINT>(panel->Width), static_cast<UINT>(panel->Height) } }
	//{
	//	// Set the swap chain for the supplied SwapChainPanel.
	//	panel->Dispatcher->RunAsync(CoreDispatcherPriority::Normal,
	//		ref new DispatchedHandler([=]() {
	//			HRESULT hr = S_OK;

	//			//Get backing native interface for SwapChainPanel.
	//			Microsoft::WRL::ComPtr<ISwapChainPanelNative> panelNative;
	//			hr = reinterpret_cast<IUnknown*>(panel)->QueryInterface(IID_PPV_ARGS(panelNative.GetAddressOf()));
	//			H::System::WinRt::ThrowIfFailed(hr);

	//			// Associate swap chain with SwapChainPanel.
	//			hr = panelNative->SetSwapChain(this->mediaPlayer.GetSwapChain().Get());
	//			H::System::WinRt::ThrowIfFailed(hr);
	//			
	//		}, CallbackContext::Any)
	//	);

	//	// Setup control / application event handlers
	//	panel->SizeChanged += ref new SizeChangedEventHandler(
	//		this, &Player::OnSizeChanged
	//	);

	//	panel->CompositionScaleChanged += ref new TypedEventHandler<Windows::UI::Xaml::Controls::SwapChainPanel^, Object^>(
	//		this, &Player::OnCompositionScaleChanged
	//	);

	//	Application::Current->Suspending += ref new SuspendingEventHandler(
	//		this, &Player::OnSuspending
	//	);

	//	Application::Current->Resuming += ref new EventHandler<Object^>(
	//		this, &Player::OnResuming
	//	);

	//	// Register playback state event handlers
	//	this->mediaPlayer.RegisterCallback(PlayerState::Started, [&](auto&) {
	//		OnPlaybackStarted();
	//	});

	//	this->mediaPlayer.RegisterCallback(PlayerState::Paused, [&](auto&) {
	//		OnPlaybackPaused();
	//	});

	//	this->mediaPlayer.RegisterCallback(PlayerState::Stopped, [&](auto&) {
	//		OnPlaybackStopped();
	//	});

	//	this->mediaPlayer.RegisterCallback(PlayerState::OpenPending, [&](auto&) {
	//		OnMediaOpenPending();
	//	});
	//}

	//Player::~Player() {
	//}

	//void Player::OpenMedia(Windows::Storage::Streams::IRandomAccessStream^ stream) {
	//	H::System::WinRt::ThrowIfFailed(this->mediaPlayer.OpenMedia(reinterpret_cast<IStream*>(stream)));
	//}

	//void Player::Play(){
	//	this->mediaPlayer.Play();
	//}

	//void Player::Pause(){
	//	this->mediaPlayer.Pause();
	//}

	//void Player::TogglePlayback() {
	//	this->mediaPlayer.TogglePlayback();
	//}

	//void Player::Stop() {
	//	this->mediaPlayer.Stop();
	//}

	//bool Player::HasVideo() {
	//	return this->mediaPlayer.HasVideo();
	//}

	//bool Player::HasAudio() {
	//	return this->mediaPlayer.HasAudio();
	//}

	////float Player::GetFPS() {
	////	return this->mediaPlayer.GetVideoRenderer().GetFPS();
	////}

	//bool Player::IsPlaying() {
	//	return this->mediaPlayer.IsPlaying();
	//}

	//bool Player::IsPaused() {
	//	return this->mediaPlayer.IsPaused();
	//}

	//bool Player::IsStopped() {
	//	return this->mediaPlayer.IsStopped();
	//}

	//int64_t Player::GetMediaDuration() {
	//	return (int64_t)this->mediaPlayer.GetMediaDuration();
	//}

	//int64_t Player::GetCurrentPosition() {
	//	return (int64_t)this->mediaPlayer.GetCurrentPosition();
	//}

	//void Player::Seek(int64_t mftime) {
	//	this->mediaPlayer.Seek(H::Chrono::Hns(mftime));
	//}

	//float Player::GetVolume() {
	//	return this->mediaPlayer.GetVolume();
	//}

	//void Player::SetVolume(float volume) {
	//	this->mediaPlayer.SetVolume(volume);
	//}

	//bool Player::IsMuted() {
	//	return this->mediaPlayer.IsMuted();
	//}

	//void Player::SetMuted(bool muted) {
	//	this->mediaPlayer.SetMuted(muted);
	//}

	//void Player::OnSizeChanged(Object^ sender, SizeChangedEventArgs^ e) {
	//	Concurrency::critical_section::scoped_lock lock{ criticalSection };
	//	
	//	auto size = e->NewSize;
	//	auto displayInformation = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();

	//	this->mediaPlayer.ResizeVideo(
	//		static_cast<WORD>(size.Width),
	//		static_cast<WORD>(size.Height),
	//		displayInformation->LogicalDpi
	//	);
	//}

	//void Player::OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Object^ args)
	//{}

	//void Player::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
	//{}

	//void Player::OnResuming(Object^ sender, Object^ args)
	//{}

	//std::unique_ptr<IAudioClientFactory> Player::CreateAudioClientFactory() {
	//	return std::make_unique<AudioClientFactory>();
	//}


	namespace Tools {
		Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> QuerySwapChainPanelNative(Helpers::WinRt::Dx::SwapChainPanel^ swapChainPanelWinRt) {
			HRESULT hr = S_OK;
			auto unk = reinterpret_cast<IUnknown*>(swapChainPanelWinRt->GetSwapChainNative());

			Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel;
			hr = unk->QueryInterface(swapChainPanel.ReleaseAndGetAddressOf());
			H::System::ThrowIfFailed(hr);

			return swapChainPanel;
		}
	}


	Player::Player(Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanelXaml)
		: swapChainPanelWinRt{ ref new Helpers::WinRt::Dx::SwapChainPanel() }
		, swapChainPanel{ Tools::QuerySwapChainPanelNative(this->swapChainPanelWinRt) }
		, videoSceneRenderer{ this->swapChainPanel }
	{
		lg::DefaultLoggers::Init("VideoPlayerPrototype.log", lg::InitFlags::DefaultFlags | lg::InitFlags::CreateInPackageFolder);

		// Setup control / application event handlers
		swapChainPanelXaml->SizeChanged += ref new SizeChangedEventHandler(
			this, &Player::OnSizeChanged
		);
		swapChainPanelXaml->CompositionScaleChanged += ref new TypedEventHandler<Windows::UI::Xaml::Controls::SwapChainPanel^, Object^>(
			this, &Player::OnCompositionScaleChanged
			);
		this->swapChainPanelWinRt->SetSwapChainPanel(swapChainPanelXaml);



		auto dispatcher = Windows::ApplicationModel::Core::CoreApplication::MainView->CoreWindow->Dispatcher;

		this->fileOpenAction = dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([&] {
			auto openPicker = ref new Windows::Storage::Pickers::FileOpenPicker();
			openPicker->SuggestedStartLocation = Windows::Storage::Pickers::PickerLocationId::DocumentsLibrary;
			openPicker->FileTypeFilter->Append(".mp4");
			openPicker->FileTypeFilter->Append(".mkv");
			openPicker->FileTypeFilter->Append(".ts");

			concurrency::create_task(openPicker->PickSingleFileAsync()).then([&](Windows::Storage::StorageFile^ file) {
				if (file) {
					concurrency::create_task(file->OpenAsync(Windows::Storage::FileAccessMode::ReadWrite)).then([&](IRandomAccessStream^ stream) {
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
		auto avReader = std::make_unique<AvReader>(reinterpret_cast<IStream*>(stream), this->swapChainPanel->GetDxDevice());
		this->videoSceneRenderer.Init(std::move(avReader));

		auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction^ action) {
			while (action->Status == AsyncStatus::Started) {
				auto tpStart = std::chrono::high_resolution_clock::now();
				Concurrency::critical_section::scoped_lock lock(this->criticalSection);

				{
					auto dxDev = swapChainPanel->GetDxDevice()->Lock();
					auto dxCtx = dxDev->LockContext();
					auto d3dCtx = dxCtx->D3D();

					// Reset the viewport to target the whole screen.
					auto viewport = swapChainPanel->GetScreenViewport();
					d3dCtx->RSSetViewports(1, &viewport);

					// Reset render targets to the screen.
					ID3D11RenderTargetView* const targets[1] = { swapChainPanel->GetBackBufferRenderTargetView() };
					d3dCtx->OMSetRenderTargets(1, targets, swapChainPanel->GetDepthStencilView());

					// Clear the back buffer and depth stencil view.
					d3dCtx->ClearRenderTargetView(swapChainPanel->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
					d3dCtx->ClearDepthStencilView(swapChainPanel->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

					this->videoSceneRenderer.Render();
				}
				
				auto tp1 = std::chrono::high_resolution_clock::now();
				
				this->swapChainPanel->Present();
				
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
		this->swapChainPanel->SetRenderResolutionScale(100.0f / resolutionPercent);
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

		//this->mediaPlayer.ResizeVideo(
		//	static_cast<WORD>(size.Width),
		//	static_cast<WORD>(size.Height),
		//	displayInformation->LogicalDpi
		//);
	}

	void Player::OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Object^ args)
	{}

	void Player::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
	{}

	void Player::OnResuming(Object^ sender, Object^ args)
	{}


	std::unique_ptr<IAudioClientFactory> Player::CreateAudioClientFactory() {
		return std::make_unique<AudioClientFactory>();
	}
}