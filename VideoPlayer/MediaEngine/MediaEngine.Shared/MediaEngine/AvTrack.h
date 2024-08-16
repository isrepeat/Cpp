#pragma once
#include <Helpers/MediaFoundation/SampleInfo.h>
#include <Helpers/MediaFoundation/MFUser.h>
#include <Helpers/Rational.h>

#include "Audio/IAudioClientFactory.h"
#include "Audio/AudioRenderer.h"
//#include "Dx/Texture/Texture.h"
#include <Helpers/Dx/DxDevice.h>
#include "Dx/directx.h"
#include "PlaybackClock.h"
#include "RewindParams.h"
#include "MediaSample.h"
#include "AvReader.h"
#include "MFTools.h"
#include "Track.h"

#include <xaudio2.h>
#include <mutex>
#include <deque>


#define AUDIO_ENABLED 0

class MediaPlayer;

class AvTrack : public Track {
public:
	AvTrack(H::Dx::DxDeviceSafeObj* dxDeviceSafeObj, std::unique_ptr<IAudioClientFactory> audioClientFactory);
	virtual ~AvTrack();

	void SetEventListener(MediaPlayer* mediaPlayer);
	void InitSourceReader(IStream* stream);

	//
	// Playback control
	//
	HRESULT Start();
	HRESULT Pause();
	HRESULT Stop();
	HRESULT Seek(H::Chrono::Hns pts);

	// Resize the swap chain.
	HRESULT Resize(H::Size size, float logicalDpi);
	Microsoft::WRL::ComPtr<IDXGISwapChain1> GetSwapChain();

	//
	// Common media properties
	//
	bool HasVideo();
	bool HasAudio();

	//
	// Audio properties
	//
	void SetVolume(float volume);
	float GetVolume();
	bool IsMuted();
	void SetMuted(bool muted);

	//
	// Playback / stream properties
	//
	H::Chrono::Hns GetSourceDuration();
	H::Chrono::Hns GetCurrentPosition();
	bool IsPlaying();
	bool IsEOS();

private:
	//
	// Initialization
	//
	HRESULT Initialize();
	HRESULT InitializeDirectX();

	//
	// Rendering
	//
	void RenderVideo();
#if AUDIO_ENABLED
	void RenderAudio();
#endif

	HRESULT RenderVideoSample(const std::unique_ptr<MF::MFVideoSample>& videoSample);
	//HRESULT SwapChainResized(Dx::Dpi logicalDpi); // Update render target and buffers when swap chain gets resized.
	HRESULT SwapChainResized(); // Update render target and buffers when swap chain gets resized.

	// 
	// MFsourceReader operations
	//
	void SeekInternal(std::shared_ptr<RewindParams> rewindParams);
	void StreamEnded(); // Called when reaching end of stream

	HRESULT PauseInternal();

private:
	std::mutex mx;

	//
	// Audio / Video device
	//
	H::Dx::DxDeviceSafeObj* dxDeviceSafeObj;
	std::unique_ptr<IAudioClientFactory> audioClientFactory;
	
	//
	// Video rendering-related members
	//
	std::shared_ptr<PlaybackClockVideo> playbackClockVideo;
	std::shared_ptr<PlaybackClockDynamic> dynamicPlaybackClockVideo;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> d3dRenderTargetView;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> targetBitmap;

	Microsoft::WRL::ComPtr<ID2D1Effect> scaleEffect;
	Microsoft::WRL::ComPtr<ID2D1Effect> centerEffect;
	
	//std::unique_ptr<Dx::Dpi> videoOutputDpi;
	H::Size videoOutputSize;
	H::Rational<double> monitorRefreshRate;
	H::Chrono::milliseconds_f monitorRefreshDuration;
	HANDLE m_frameLatencyWaitableObject = nullptr;

	std::queue<std::unique_ptr<MF::MFVideoSample>> videoSamples;
	std::queue<std::unique_ptr<MF::MFVideoSample>> videoSamplesProcessed;
	std::unique_ptr<MF::MFVideoSample> lastRenderedVideoSample;
	std::chrono::steady_clock::time_point lastRenderedVideoSampleTimePoint;

	//
	// Audio rendering-related members
	//
	AudioRenderer audioRenderer;
	std::queue<std::unique_ptr<MF::MFAudioSample>> audioSamples;
	std::queue<std::unique_ptr<MF::MFAudioSample>> audioSamplesProcessed;

	bool muted = false;
	float volume = 1.0f;


	//
	// Media members
	//
	std::unique_ptr<AvReader> avReader;
	MediaPlayer* mediaPlayer = nullptr; // Optional MediaPlayer* to notify about playback events


	//
	// Playback related members
	//
	std::shared_ptr<PlaybackClock> playbackClock;
	H::Chrono::Hns currentPosition = 0_hns;
	std::atomic<bool> isEndOfStream = false;
	std::weak_ptr<AvReaderRewindAsyncResult> avReaderRewindAsyncResultWeak;

	//
	// Threads
	//
	std::thread videoRenderThread;
	std::thread audioRenderThread;
	std::atomic<bool> stopThreads = false;
};