#pragma once
#include <MediaEngine/MediaPlayer.h>
#include "VideoSceneRenderer.h"
#include <Helpers/Dx/SwapChainPanel.h>


namespace MediaEngineWinRT {
	public delegate void MediaPlayerEventHandler();

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class Player sealed {
    public:
        Player(Windows::UI::Xaml::Controls::SwapChainPanel^ xamlSwapChainPanel);
		virtual ~Player();

		void OpenMedia(Windows::Storage::Streams::IRandomAccessStream^ stream);

		/* Playback control */
		void Play();
		void Pause();
		void TogglePlayback();
		void Stop();
		void Seek(int64_t mftime);

		/* Playback state properties */
		bool IsPlaying();
		bool IsPaused();
		bool IsStopped();
		int64_t GetMediaDuration();
		int64_t GetCurrentPosition();

		/* Audio controls (unimplemented) */
		float GetVolume();
		void SetVolume(float volume);
		bool IsMuted();
		void SetMuted(bool mutes);

		/* Miscellaneous properties */
		bool HasVideo();
		bool HasAudio();
		//float GetFPS();

		void SetDynamicRenderResolution(int value);

		/* MediaPlayer event registration */
		event MediaPlayerEventHandler^ OnMediaOpenPending;
		event MediaPlayerEventHandler^ OnPlaybackStarted;
		event MediaPlayerEventHandler^ OnPlaybackPaused;
		event MediaPlayerEventHandler^ OnPlaybackStopped;

		/* Event handlers */
		void OnSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
		void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel^ sender, Platform::Object^ args);
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnResuming(Platform::Object^ sender, Platform::Object^ args);

	private:
		//std::shared_ptr<H::Dx::SwapChainPanel> QuerySwapChainPanelNative();
		std::unique_ptr<IAudioClientFactory> CreateAudioClientFactory();

	private:
		Concurrency::critical_section criticalSection;
		Windows::Foundation::IAsyncAction^ fileOpenAction;
		Windows::Foundation::IAsyncAction^ renderLoopWorker;
		//MediaPlayer mediaPlayer;
		Helpers::WinRt::Dx::SwapChainPanel^ swapChainPanelWinRt;
		Microsoft::WRL::ComPtr<H::Dx::ISwapChainPanel> swapChainPanel;
		VideoSceneRenderer videoSceneRenderer;
    };
}