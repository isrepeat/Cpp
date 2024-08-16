#include "MediaPlayer.h"


MediaPlayer::MediaPlayer(std::unique_ptr<IAudioClientFactory> audioClientFactory, H::Size /*windowSize*/)
	: dxDeviceSafeObj{ std::make_unique<H::Dx::details::DxVideoDeviceMF>() }
	, avTrack(&dxDeviceSafeObj, std::move(audioClientFactory))
{
	this->avTrack.SetEventListener(this);
}

MediaPlayer::~MediaPlayer() {
} 

HRESULT MediaPlayer::OpenMedia(IStream* pStream) {
	HRESULT hr = S_OK;

	try {
		this->avTrack.InitSourceReader(pStream);
	}
	catch (_com_error& err) {
		return err.Error();
	}
	catch (...) {
		return E_FAIL;
	}

	this->state = PlayerState::Stopped;
	hr = this->Play();

	return hr;
}


void MediaPlayer::RegisterCallback(PlayerState state, PlayerStateCallback callback) {
	this->stateCallbacks.emplace(state, callback);
}

HRESULT MediaPlayer::Play() {
	if (this->state != PlayerState::Paused && this->state != PlayerState::Stopped)
		return MF_E_INVALIDREQUEST;

	HRESULT hr = this->avTrack.Start();
	H::System::ThrowIfFailed(hr);

	this->SetState(PlayerState::Started);
	return hr;
}

HRESULT MediaPlayer::Pause() {
	if (!this->IsPlaying())
		return MF_E_INVALIDREQUEST;

	HRESULT hr = this->avTrack.Pause();
	H::System::ThrowIfFailed(hr);

	this->SetState(PlayerState::Paused);
	return hr;
}

HRESULT MediaPlayer::TogglePlayback() {
	return this->IsPlaying() ? this->Pause() : this->Play();
}

HRESULT MediaPlayer::Stop() {
	if (!this->IsPlaying() && !this->IsPaused())
		return MF_E_INVALIDREQUEST;

	HRESULT hr = this->avTrack.Stop();
	H::System::ThrowIfFailed(hr);
	
	this->SetState(PlayerState::Stopped);
	return S_OK;
}

HRESULT MediaPlayer::ResizeVideo(UINT width, UINT height, float logicalDpi) {
	return this->avTrack.Resize(H::Size{ width, height }, logicalDpi);
}


Microsoft::WRL::ComPtr<IDXGISwapChain1> MediaPlayer::GetSwapChain() {
	return this->avTrack.GetSwapChain();
}


void MediaPlayer::SetState(PlayerState state) {
	if (this->state == state)
		return;

	this->state = state;
	auto it = this->stateCallbacks.find(state);
	if (it == this->stateCallbacks.end())
		return;

	it->second(*this);
}

PlayerState MediaPlayer::GetState() {
	return this->state;
}

bool MediaPlayer::IsPlaying() {
	return this->state == PlayerState::Started;
}

bool MediaPlayer::IsPaused() {
	return this->state == PlayerState::Paused;
}

bool MediaPlayer::IsStopped() {
	return this->state == PlayerState::Stopped;
}

BOOL MediaPlayer::HasVideo() {
	return this->avTrack.HasVideo();
}

BOOL MediaPlayer::HasAudio() {
	return this->avTrack.HasAudio();
}

H::Chrono::Hns MediaPlayer::GetMediaDuration() {
	return this->avTrack.GetSourceDuration();
}

H::Chrono::Hns MediaPlayer::GetCurrentPosition() {
	return this->avTrack.GetCurrentPosition();
}

HRESULT MediaPlayer::Seek(H::Chrono::Hns pos) {
	return this->avTrack.Seek(pos);
}

void MediaPlayer::SetVolume(float volume) {
	this->avTrack.SetVolume(volume);
}

float MediaPlayer::GetVolume() {
	return this->avTrack.GetVolume();
}

bool MediaPlayer::IsMuted() {
	return this->avTrack.IsMuted();
}

void MediaPlayer::SetMuted(bool muted) {
	this->avTrack.SetMuted(muted);
}