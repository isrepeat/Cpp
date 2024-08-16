#pragma once
#include "AvTrack.h"

enum class PlayerState {
	None = 0,    // Player is uninitialized.
	OpenPending, // Player is opening a file.
	Started,     // Playback is active.
	Paused,      // Playback is paused.
	Stopped      // Playback is stopped. 
};

using PlayerStateCallback = std::function<void(class MediaPlayer&)>;
using VideoRendererFactory = std::function<HRESULT(Microsoft::WRL::ComPtr<AvTrack>&)>;


class MediaPlayer {
public:
	MediaPlayer(std::unique_ptr<IAudioClientFactory> audioClientFactory, H::Size windowSize);
	virtual ~MediaPlayer();

public:
	// Playback
	HRESULT OpenMedia(IStream* byteStream);

	// Registers a callback to be invoked on player state change
	void RegisterCallback(PlayerState state, PlayerStateCallback callback);

	// Playback control
	HRESULT Play();
	HRESULT Pause();
	HRESULT TogglePlayback();
	HRESULT Stop();
	HRESULT ResizeVideo(UINT width, UINT height, float logicalDpi);

	Microsoft::WRL::ComPtr<IDXGISwapChain1> GetSwapChain();

	void SetState(PlayerState);
	PlayerState GetState();
	bool IsPlaying();
	bool IsPaused();
	bool IsStopped();

	BOOL HasVideo();
	BOOL HasAudio();
	H::Chrono::Hns GetMediaDuration();

	H::Chrono::Hns GetCurrentPosition();
	HRESULT Seek(H::Chrono::Hns pos);

	void SetVolume(float volume);
	float GetVolume();
	bool IsMuted();
	void SetMuted(bool muted);

private:
	H::Dx::DxDeviceSafeObj dxDeviceSafeObj;
	AvTrack avTrack;
	
	PlayerState state = PlayerState::None;
	H::Chrono::Hns currentDuration = 0_hns;

	// Callback map for playback state change events
	std::unordered_map<PlayerState, PlayerStateCallback> stateCallbacks;
};