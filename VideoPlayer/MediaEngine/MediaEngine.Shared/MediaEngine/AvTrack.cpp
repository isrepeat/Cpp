#include "AvTrack.h"
#include "MediaPlayer.h"

#include <Helpers/MediaFoundation/MediaTypeInfo.h>
#include <Helpers/Dx/DxSharedTexture.h>
#include <Helpers/Dx/DxgiDeviceLock.h>
#include <Helpers/Dx/DxHelpers.h>
#include <Helpers/Container.hpp>
#include <Helpers/Memory.h>
#include <Helpers/Math.h>

#include <shlwapi.h>
#include <chrono>
#include <timeapi.h>
#include <dwmapi.h>

//#include <Helpers/MediaFoundation/MFLibs.h>
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "Mfreadwrite.lib")


AvTrack::AvTrack(H::Dx::DxDeviceSafeObj* dxDeviceSafeObj, std::unique_ptr<IAudioClientFactory> audioClientFactory)
	// [device]
	: dxDeviceSafeObj{ dxDeviceSafeObj }
	, audioClientFactory{ std::move(audioClientFactory) }
	// [video]
	, playbackClockVideo{ std::make_shared<PlaybackClockVideo>() }
	, dynamicPlaybackClockVideo{ std::make_shared<PlaybackClockDynamic>(playbackClockVideo) }
	// NOTE: use first adapter (output must bind to it)
	, monitorRefreshRate{ H::Dx::GetRefreshRateForDXGIOutput(H::Dx::EnumOutputsState{ H::Dx::EnumAdaptersState{}.Next() }.Next().GetDXGIOutput()) }
	, monitorRefreshDuration{ monitorRefreshRate.Inversed().As<H::Chrono::seconds_f>() }
	, lastRenderedVideoSampleTimePoint{ std::chrono::high_resolution_clock::now() }
	// [audio]
	, audioRenderer{ this->audioClientFactory.get() }
	// [playback]
	, playbackClock{ std::make_shared<PlaybackClock>() }
{	
	lg::DefaultLoggers::Init("VideoPlayerPrototype.log", lg::InitFlags::DefaultFlags | lg::InitFlags::CreateInPackageFolder);
	H::Dx::LogDeviceInfo();

	Initialize();

	this->playbackClock->SetPlaybackClockVideo(this->dynamicPlaybackClockVideo);
	this->playbackClock->SetPlaybackClockAudio(this->audioRenderer.GetPlaybackClock());

//#if VIDEO_OPTION == 1 || VIDEO_OPTION == 2
	this->videoRenderThread = std::thread([this] {
		LOG_THREAD(L"Render VIDEO thread");
		while (!this->stopThreads) {
			this->RenderVideo();
		}
		});
//#endif

#if AUDIO_ENABLED
	this->audioRenderThread = std::thread([this] {
		LOG_THREAD(L"Render AUDIO thread");
		while (!this->stopThreads) {
			this->RenderAudio();
		}
		});
#endif
}

AvTrack::~AvTrack() {
	this->stopThreads = true;

	if (this->videoRenderThread.joinable())
		this->videoRenderThread.join();

	if (this->audioRenderThread.joinable())
		this->audioRenderThread.join();

	if (this->m_frameLatencyWaitableObject) {
		CloseHandle(this->m_frameLatencyWaitableObject);
	}
}


void AvTrack::SetEventListener(MediaPlayer* mediaPlayer) {
	std::unique_lock lk{ mx };
	this->mediaPlayer = mediaPlayer;
}

void AvTrack::InitSourceReader(IStream* pStream) {
	std::unique_lock lk{ mx };
	HRESULT hr = S_OK;

	this->avReader = std::make_unique<AvReader>(pStream, this->dxDeviceSafeObj);

	// Prepare renderer for playback
	this->playbackClock->SetPlaybackClockState(PlaybackClockState::Stopped);
	this->currentPosition = 0_hns;
	this->isEndOfStream = false;

	//this->baseTexture = std::make_shared<Texture>(TextureType::ARGB32, true, true);
}


//
// Playback control
//
HRESULT AvTrack::Start() {
	LOG_DEBUG_D("Start()");
	std::unique_lock lk{ mx };
	HRESULT hr = S_OK;

	if (this->isEndOfStream) {
		auto rewindParamsTmp = MakeRewindParams(0_hns);
		this->SeekInternal(rewindParamsTmp);
		rewindParamsTmp->asyncResult.Wait();

		this->isEndOfStream = false;
	}

	if (auto activeAudioStream = this->avReader->GetLockedAvSourceStreamManager()->GetActiveAudioStream()) {
		this->audioRenderer.Play();
	}

	this->playbackClock->SetPlaybackClockState(PlaybackClockState::Playing);
	return hr;
}

HRESULT AvTrack::Pause() {
	LOG_DEBUG_D("Pause()");
	std::unique_lock lk{ mx };

	if (auto activeAudioStream = this->avReader->GetLockedAvSourceStreamManager()->GetActiveAudioStream()) {
		this->audioRenderer.Pause();
	}
	return this->PauseInternal();
}

HRESULT AvTrack::Stop() {
	LOG_DEBUG_D("Stop()");
	std::unique_lock lk{ mx };

	this->playbackClock->SetPlaybackClockState(PlaybackClockState::Stopped);
	if (auto activeAudioStream = this->avReader->GetLockedAvSourceStreamManager()->GetActiveAudioStream()) {
		this->audioRenderer.Stop();
	}

	auto rewindParamsTmp = MakeRewindParams(0_hns);
	this->SeekInternal(rewindParamsTmp);
	rewindParamsTmp->asyncResult.Wait();

	return S_OK;
}


HRESULT AvTrack::Seek(H::Chrono::Hns pts) {
	LOG_DEBUG_D("Seek(pts = {})", pts);
	std::unique_lock lk{ mx };

	auto rewindParamsTmp = MakeRewindParams(pts);
	this->SeekInternal(rewindParamsTmp);
	rewindParamsTmp->asyncResult.Wait();

	this->isEndOfStream = false;
	return S_OK;
}


HRESULT AvTrack::Resize(H::Size size, float logicalDpi) {
	//std::unique_lock lk{ mx };
	//
	//auto dpi = Dx::Dpi{ logicalDpi };
	//this->videoOutputSize = size * dpi;

	//auto dxDev = this->dxDeviceSafeObj->Lock();
	//auto dxCtx = dxDev->LockContext();

	//this->backBuffer = nullptr;
	//this->targetBitmap = nullptr;
	//this->d3dRenderTargetView = nullptr;
	//dxCtx->D2D()->SetTarget(nullptr);
	//dxCtx->D3D()->Flush();

	//HRESULT hr = S_OK;
	//hr = ResizeSwapChain(this->swapChain, this->videoOutputSize);
	//H::System::ThrowIfFailed(hr);

	//hr = SwapChainResized(dpi);
	//H::System::ThrowIfFailed(hr);
	//return hr;
	return S_OK;
}

Microsoft::WRL::ComPtr<IDXGISwapChain1> AvTrack::GetSwapChain() {
	std::unique_lock lk(mx);
	return this->swapChain;
}


//
// Common media properties
//
bool AvTrack::HasVideo() {
	std::unique_lock lk(mx);
	return static_cast<bool>(this->avReader->GetLockedAvSourceStreamManager()->GetActiveVideoStream());
}

bool AvTrack::HasAudio() {
	std::unique_lock lk(mx);
	return static_cast<bool>(this->avReader->GetLockedAvSourceStreamManager()->GetActiveAudioStream());
}


//
// Audio properties
//
void AvTrack::SetVolume(float v) {
	std::unique_lock lk(mx);
	this->volume = v;
}

float AvTrack::GetVolume() {
	std::unique_lock lk(mx);
	return this->volume;
}

bool AvTrack::IsMuted() {
	std::unique_lock lk(mx);
	return this->muted;
}

void AvTrack::SetMuted(bool muted) {
	std::unique_lock lk(mx);
	this->muted = muted;
}


H::Chrono::Hns AvTrack::GetSourceDuration() {
	std::unique_lock lk(mx);
	if (!this->avReader) {
		return 0_hns;
	}
	return this->avReader->GetSourceDuration();
}

//
// Playback / stream properties
//
H::Chrono::Hns AvTrack::GetCurrentPosition() {
	std::unique_lock lk(mx);
	return this->currentPosition;
}

bool AvTrack::IsPlaying() {
	std::unique_lock lk(mx);
	return this->playbackClock->GetPlaybackClockState() == PlaybackClockState::Playing;
}

bool AvTrack::IsEOS() {
	std::unique_lock lk(mx);
	return this->isEndOfStream;
}


//
// Initialization
//
HRESULT AvTrack::Initialize() {
	HRESULT hr = S_OK;

	// Initialize DirectX.
	hr = InitializeDirectX();
	H::System::ThrowIfFailed(hr);

	// Create the swap chain.
	hr = CreateSwapChain(this->dxDeviceSafeObj->Lock()->GetD3DDevice(), this->swapChain.ReleaseAndGetAddressOf());
	H::System::ThrowIfFailed(hr);

	//hr = SwapChainResized(Dx::Dpi::StandardDpi());
	hr = SwapChainResized();
	H::System::ThrowIfFailed(hr);
	return hr;
}

HRESULT AvTrack::InitializeDirectX() {
	HRESULT hr = S_OK;
	auto dxDev = this->dxDeviceSafeObj->Lock();
	auto dxCtx = dxDev->LockContext();

	// Create frame transformation effects
	hr = dxCtx->D2D()->CreateEffect(CLSID_D2D1Scale, &this->scaleEffect);
	H::System::ThrowIfFailed(hr);

	hr = dxCtx->D2D()->CreateEffect(CLSID_D2D12DAffineTransform, &this->centerEffect);
	H::System::ThrowIfFailed(hr);

	return hr;
}


//
// Rendering
//
void AvTrack::RenderVideo() {
	auto tpRenderIteration1 = std::chrono::high_resolution_clock::now();
	auto tp1 = std::chrono::high_resolution_clock::now();

	if (auto avReaderRewindAsyncResult = this->avReaderRewindAsyncResultWeak.lock()) {
		avReaderRewindAsyncResult->WaitFirstVideoSample();
	}

	// waiting too long under lk can give more priority to render thread and only render thread will be able to lock mx
	{
		// save to tmp because while unlocked m_frameLatencyWaitableObject can change in resize
		auto tmpFrameLatencyWaitableObject = this->m_frameLatencyWaitableObject;
		DWORD result = WaitForSingleObjectEx(
			tmpFrameLatencyWaitableObject,
			1000, // 1 second timeout (shouldn't ever occur)
			true
		);
	}

	auto tp2 = std::chrono::high_resolution_clock::now();
	auto dxDev = this->dxDeviceSafeObj->Lock();

	// Here we have ~15ms to handle frame before next VSync lock
	// ...

	if (this->playbackClock->GetPlaybackClockState() == PlaybackClockState::Playing) {
		//auto videoClockPlaybackPts = this->playbackClock->VideoClock()->GetLocked()->playbackClockInterface->GetPlaybackPts();
		//auto audioClockPlaybackPts = this->playbackClock->AudioClock()->GetLocked()->playbackClockInterface->GetPlaybackPts();
		//LOG_DEBUG_D("\n"
		//	"VideoClock: playbackPts = {}\n"
		//	"AudioClock: playbackPts = {}\n"
		//	, videoClockPlaybackPts
		//	, audioClockPlaybackPts
		//);

		std::unique_ptr<MF::MFVideoSample> videoSample;

		if (this->lastRenderedVideoSample) {
			H::Chrono::Hns elapsedTimeFromLastRenderedVideoSample = std::chrono::duration_cast<H::Chrono::Hns::_MyBase>(
				std::chrono::high_resolution_clock::now() - this->lastRenderedVideoSampleTimePoint);

			auto distanceToNextVideoSample = this->lastRenderedVideoSample->duration - elapsedTimeFromLastRenderedVideoSample;
			auto distanceToNextVideoSampleMs = distanceToNextVideoSample.ToDuration<H::Chrono::milliseconds_f>();

			bool elapsedTimeGreaterLastFrameDuration = elapsedTimeFromLastRenderedVideoSample > this->lastRenderedVideoSample->duration;
			bool nextVideoSampleNearCurrentVSyncPts = std::abs(distanceToNextVideoSampleMs.count()) < this->monitorRefreshDuration.count() / 4;

			bool shouldPresentNextSampleInThisIteration = elapsedTimeGreaterLastFrameDuration || nextVideoSampleNearCurrentVSyncPts;

			if (shouldPresentNextSampleInThisIteration) {
				this->lastRenderedVideoSampleTimePoint = std::chrono::high_resolution_clock::now();
				videoSample = this->avReader->PopNextVideoSample();
			}
		}
		else {
			this->lastRenderedVideoSampleTimePoint = std::chrono::high_resolution_clock::now();
			videoSample = this->avReader->PopNextVideoSample();
		}

		if (videoSample) {
			//LOG_DEBUG_D("Get sample [pts = {}]", videoSample->pts);
			this->playbackClock->VideoClock()->GetLocked()->playbackClockInterface->UpdateLastPlaybackPts(videoSample->pts);
			this->currentPosition = videoSample->pts;
			this->lastRenderedVideoSample = std::move(videoSample);
		}
	}

	auto tp3 = std::chrono::high_resolution_clock::now();

	this->RenderVideoSample(this->lastRenderedVideoSample);

	auto tp4 = std::chrono::high_resolution_clock::now();
	
	if (this->avReader) {
		this->avReader->RequestNeccessaryVideoSamples();
	}

	auto tpRenderIteration2 = std::chrono::high_resolution_clock::now();
	auto dtRenderIteration = H::Chrono::milliseconds_f{ tpRenderIteration2 - tpRenderIteration1 };
	LOG_DEBUG_D("\n"
		"dt [Render iteration]{} = {}\n"
		"dt [Vsync] = {}\n"
		"dt [Processing] = {}\n"
		"dt [Render frame] = {}\n"
		, dtRenderIteration > 20ms ? " [slow]" : ""
		, dtRenderIteration
		, tp2 - tp1
		, tp3 - tp2
		, tp4 - tp3
	);
}


#if AUDIO_ENABLED
void AvTrack::RenderAudio() {
	auto tpRenderAudioIteration1 = std::chrono::high_resolution_clock::now();

	if (playbackClock->GetPlaybackClockState() == PlaybackClockState::Playing) {
		requestsSamplesQueue->Push({ "RenderAudio::lambda__processSample", [this] {
			std::unique_lock lkFlush{ mxFlush };
			cvFlushFinish.wait(lkFlush, [this] {
				return !flushInProcess;
				});

			std::unique_ptr<MF::MFAudioSample> audioSample;
			{
				std::unique_lock lk{ mx };
				audioSample = PopNextAudioSample(lk);
				RequestNeccessaryAudioSamples(lk);
			}

			if (audioSample) {
				std::unique_lock lk{ mxAudioSamplesProcessing };
				LOG_DEBUG_D("push audioSampleProcessed [pts = {}ms]", H::Chrono::milliseconds_f{ audioSample->pts }.count());
				audioSamplesProcessed.push(std::move(audioSample));
			}
			} });

		// block current thread until buffer overflow
		AudioRenderer::Buffer audioBuffer = audioRenderer.GetBuffer();

		bool futureSample = false;
		while (!futureSample) {
			MF::MFAudioSample* audioSample = nullptr;
			{
				std::unique_lock lk{ mxAudioSamplesProcessing };
				if (audioSamplesProcessed.size() > 0) {
					audioSample = audioSamplesProcessed.front().get();
				}
			}

			if (audioSample) {
				bool endMixed = false, oldSample = false;
				audioBuffer.Mix(audioSample, endMixed, oldSample, futureSample, 0, 1);

				if (oldSample || endMixed) {
					std::unique_lock lk{ mxAudioSamplesProcessing };
					if (audioSamplesProcessed.size() > 0) {
						audioSamplesProcessed.pop();
						--audioRequestedSamples;
					}
				}
				else {
					futureSample = true;
				}
			}
			else {
				break;
			}
		}
	}
	auto tpRenderAudioIteration2 = std::chrono::high_resolution_clock::now();
	//LOG_DEBUG_D("Render AUDIO iteration time = {}ms", std::chrono::duration_cast<H::Chrono::milliseconds_f>(tpRenderAudioIteration2 - tpRenderAudioIteration1).count());

	// TODO: check performance if use Sleep(15) here
}
#endif

HRESULT AvTrack::RenderVideoSample(const std::unique_ptr<MF::MFVideoSample>& videoSample) {
	HRESULT hr = S_OK;
	//auto dxDev = this->dxDeviceSafeObj->Lock();
	//auto dxCtx = dxDev->LockContext();
	//auto d2dCtx = dxCtx->D2D();

	//d2dCtx->BeginDraw();
	//d2dCtx->Clear(D2D1::ColorF(D2D1::ColorF::Gray));

	//if (videoSample) {
	//	auto dpiScaleFactor = this->videoOutputDpi->GetScaleFactor();

	//	// Create a ID2D1Bitmap1 out of the frame texture
	//	auto frame = DxTools::CreateFrameBitmap(dxCtx, videoSample->texture);
	//	this->scaleEffect->SetInput(0, frame.Get());
	//	
	//	D3D11_TEXTURE2D_DESC frameDesc;
	//	videoSample->texture->GetDesc(&frameDesc);


	//	// Scale the frame keeping aspect ratio
	//	float scaleX = static_cast<float>(this->videoOutputSize.width) / frameDesc.Width;
	//	float scaleY = static_cast<float>(this->videoOutputSize.height) / frameDesc.Height;
	//	float scaleFactor = std::min(scaleX, scaleY);
	//	this->scaleEffect->SetValue(
	//		D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(scaleFactor, scaleFactor));

	//	Microsoft::WRL::ComPtr<ID2D1Image> pScaledFrame;
	//	this->scaleEffect->GetOutput(&pScaledFrame);
	//	this->centerEffect->SetInput(0, pScaledFrame.Get());


	//	// NOTE: For scaleEffect keep 'scaleFactor' as it is. But for centerEffect 'scaleX' & 'scaleFactor' 
	//	//       need divide on dpiScaleFactor because swapChain have transform matrix 2x2:
	//	//		 | 1 / dpiScaleFactor,  0 |
	//	//		 | 0,  1 / dpiScaleFactor |
	//	//
	//	float freeWidthFrame = (frameDesc.Width) * (scaleX - scaleFactor) / dpiScaleFactor; // rest of scale X
	//	float freeHeightFrame = (frameDesc.Height) * (scaleY - scaleFactor) / dpiScaleFactor; // rest of scale Y
	//	float offsetX = freeWidthFrame / 2;
	//	float offsetY = freeHeightFrame / 2;
	//	D2D1_MATRIX_3X2_F matrix{
	//		1, 0,
	//		0, 1,
	//		offsetX, offsetY
	//	};
	//	this->centerEffect->SetValue(
	//		D2D1_2DAFFINETRANSFORM_PROP_TRANSFORM_MATRIX, matrix);


	//	d2dCtx->DrawImage(this->centerEffect.Get());

	//	// TODO: rewrite render frame via d3dCtx
	//	//this->baseTexture->SetD3DTexture(dxDev, videoSample->texture);
	//	//this->baseTexture->SetToContext(dxCtx);
	//}

	//d2dCtx->EndDraw();

	//
	//// The first argument instructs DXGI to block until VSync, putting the application
	//// to sleep until the next VSync. This ensures we don't waste any cycles rendering
	//// frames that will never be displayed to the screen.
	//hr = this->swapChain->Present(1, 0);
	//if (FAILED(hr)) {
	//	Dbreak;
	//}

	////// Discard the contents of the render target.
	////// This is a valid operation only when the existing contents will be entirely
	////// overwritten. If dirty or scroll rects are used, this call should be removed.
	////d3dDeviceContext->DiscardView(m_d3dRenderTargetView.Get());

	//////// Discard the contents of the depth stencil.
	//////d3dDeviceContext->DiscardView(m_d3dDepthStencilView.Get());

	return hr;
}


//HRESULT AvTrack::SwapChainResized(Dx::Dpi logicalDpi) {
//	auto dxDev = this->dxDeviceSafeObj->Lock();
//	auto dxCtx = dxDev->LockContext();
//	
//	this->videoOutputDpi = std::make_unique<Dx::Dpi>(logicalDpi);
//
//	HRESULT hr = S_OK;
//	Microsoft::WRL::ComPtr<IDXGISwapChain2> swapChain2;
//	hr = this->swapChain.As(&swapChain2);
//	H::System::ThrowIfFailed(hr);
//
//	if (this->m_frameLatencyWaitableObject) {
//		CloseHandle(this->m_frameLatencyWaitableObject);
//	}
//	this->m_frameLatencyWaitableObject = swapChain2->GetFrameLatencyWaitableObject();
//
//
//	DXGI_MATRIX_3X2_F swapchainTransform = { 0 };
//	auto dpiScaleFactor = this->videoOutputDpi->GetScaleFactor();
//	swapchainTransform._11 = 1.0f / dpiScaleFactor;
//	swapchainTransform._22 = 1.0f / dpiScaleFactor;
//	
//	hr = swapChain2->SetMatrixTransform(&swapchainTransform);
//	H::System::ThrowIfFailed(hr);
//
//
//	// Create a render target view of the swap chain back buffer.
//	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
//	hr = this->swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
//	H::System::ThrowIfFailed(hr);
//
//	hr = dxDev->GetD3DDevice()->CreateRenderTargetView(
//		backBuffer.Get(),
//		nullptr,
//		&this->d3dRenderTargetView);
//	H::System::ThrowIfFailed(hr);
//
//
//	hr = this->swapChain->GetBuffer(0, IID_PPV_ARGS(&this->backBuffer));
//	H::System::ThrowIfFailed(hr);
//
//	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
//	D2D1::BitmapProperties1(
//		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
//		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)
//	);
//
//	// Direct2D needs the dxgi version of the backbuffer surface pointer.
//	Microsoft::WRL::ComPtr<IDXGISurface> dxgiBackBuffer;
//	hr = this->swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
//	H::System::ThrowIfFailed(hr);
//
//	// Get a D2D surface from the DXGI back buffer to use as the D2D render target.
//	hr = dxCtx->D2D()->CreateBitmapFromDxgiSurface(dxgiBackBuffer.Get(), &bitmapProperties, this->targetBitmap.ReleaseAndGetAddressOf());
//	H::System::ThrowIfFailed(hr);
//
//	// Set the Direct2D render target.
//	dxCtx->D2D()->SetTarget(this->targetBitmap.Get());
//	dxCtx->D2D()->SetDpi(logicalDpi.GetDpiX(), logicalDpi.GetDpiY());
//
//	return hr;
//}
HRESULT AvTrack::SwapChainResized() {
	return S_OK;
}

// 
// MFsourceReader operations
//

void AvTrack::SeekInternal(std::shared_ptr<RewindParams> rewindParams) {
	//LOG_DEBUG_D("SeekInternal(pos = {}ms)", H::Chrono::milliseconds_f{ pos }.count());
	auto avReaderRewindParams = rewindParams->MakeAvReaderRewindParams(this);
	this->avReaderRewindAsyncResultWeak = avReaderRewindParams.asyncResultWeak;

	if (auto avReaderRewindAsyncResult = this->avReaderRewindAsyncResultWeak.lock()) {
		avReaderRewindAsyncResult->GetEvents()->gotFirstVideoSampleEvent.Add([this](MF::SampleInfo mfVideoSample) {
			LOG_DEBUG_D("Got first video sample = [{}ms + {}ms = {}ms]"
				, mfVideoSample.offset
				, mfVideoSample.pts
				, mfVideoSample.offset + mfVideoSample.pts
			);
			// TODO: use guard for lastRenderedVideoSample
			//thread::critical_section::scoped_lock lk(this->viewCs); // WARNING: may be dedalock, use another mutex
			this->lastRenderedVideoSample = nullptr;
			});
	}

	this->avReader->SeekAsync(avReaderRewindParams);
}


void AvTrack::StreamEnded() {
	LOG_DEBUG_D("End of stream");
	this->isEndOfStream = true;
	this->PauseInternal();

	if (this->mediaPlayer)
		this->mediaPlayer->SetState(PlayerState::Stopped);
}


HRESULT AvTrack::PauseInternal() {
	if (this->playbackClock->GetPlaybackClockState() == PlaybackClockState::Paused) {
		return E_UNEXPECTED;
	}

	LOG_DEBUG_D("Pausing playback");
	this->playbackClock->SetPlaybackClockState(PlaybackClockState::Paused);
	return S_OK;
}