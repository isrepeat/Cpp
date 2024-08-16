////#include "pch.h"
//#include "XAudio2Output.h"
//#include <Helpers/Memory.h>
//#include <Helpers/System.h>
//
//PlaybackClockXAudio2::PlaybackClockXAudio2(Microsoft::WRL::ComPtr<IAudioClock> audioClock, WAVEFORMATEX waveFmt)
//	: audioClock{ audioClock }
//	, waveFmt{ waveFmt }
//	, offsetPts{ 0_hns }
//{}
//
//PlaybackClockXAudio2::~PlaybackClockXAudio2() {
//}
//
//void PlaybackClockXAudio2::Start() {
//	std::lock_guard lk{ mx };
//}
//
//void PlaybackClockXAudio2::Pause() {
//	std::lock_guard lk{ mx };
//}
//
//void PlaybackClockXAudio2::Stop() {
//	std::lock_guard lk{ mx };
//}
//
//void PlaybackClockXAudio2::SetOffsetPts(H::Chrono::Hns offsetPts) {
//	std::lock_guard lk{ mx };
//	this->offsetPts = offsetPts;
//}
//
//void PlaybackClockXAudio2::UpdateLastPlaybackPts(H::Chrono::Hns lastPlaybackPts) {
//	//std::lock_guard lk{ mx };
//	//this->playbackPts = newPlaybackPts;
//}
//
//H::Chrono::Hns PlaybackClockXAudio2::GetLastPlaybackPts() {
//	return {};
//}
//
//H::Chrono::Hns PlaybackClockXAudio2::GetPlaybackPts() {
//	HRESULT hr = S_OK;
//	uint64_t frequency;
//	uint64_t position, perfCounterPosition;
//
//	hr = this->audioClock->GetFrequency(&frequency);
//	H::System::ThrowIfFailed(hr);
//
//	hr = this->audioClock->GetPosition(&position, &perfCounterPosition);
//	H::System::ThrowIfFailed(hr);
//
//	// position and frequency measured in bytes
//	// need convert them to audio frames
//	position /= this->waveFmt.nBlockAlign;
//	frequency /= this->waveFmt.nBlockAlign;
//
//	auto res = static_cast<int64_t>(HMath::Convert2(position, frequency, 10000000ULL));
//	return this->offsetPts + H::Chrono::Hns{ res };
//}
//
//
//XAudio2Output::XAudio2Output(IAudioClientFactory* audioClientFactory, H::Chrono::Hns audioBufferMaxDuration) 
//	: audioBufferMaxDuration{ audioBufferMaxDuration }
//	, voiceCallback{ std::make_unique<VoiceCallback>(this) }
//{
//	HRESULT hr = S_OK;
//	{
//		H::CoUniquePtr<WAVEFORMATEX> audioClientWaveFormat;
//
//		audioClient = audioClientFactory->CreateAudioClient(AudioClientType::RenderAudioClient);
//		hr = audioClient->GetMixFormat(audioClientWaveFormat.GetAddressOf());
//		H::System::ThrowIfFailed(hr);
//
//		// Use AudioWaveFormat helper to copy memory
//		audioFormatExtensible = AudioWaveFormat{ audioClientWaveFormat.get() }.GetWaveFormatExtensible();
//	}
//
//
//	//Microsoft::WRL::ComPtr<IAudioClock> audioClock;
//	//hr = this->audioClient->GetService(IID_PPV_ARGS(audioClock.GetAddressOf()));
//	//H::System::ThrowIfFailed(hr);
//
//	//this->playbackClockAudio = std::make_unique<PlaybackClockXAudio2>(audioClock, this->audioFormatExtensible.Format);
//
//	
//	//WAVEFORMATEX& waveFmt = audioFormatExtensible.Format;
//	////waveFmt.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
//	//waveFmt.wFormatTag = WAVE_FORMAT_PCM;
//	//waveFmt.nChannels = 2;
//	//waveFmt.nSamplesPerSec = 48000;
//	//waveFmt.wBitsPerSample = sizeof(float) * 8;
//	//waveFmt.nBlockAlign = waveFmt.nChannels * (waveFmt.wBitsPerSample / 8);
//	//waveFmt.nAvgBytesPerSec = waveFmt.nBlockAlign * waveFmt.nSamplesPerSec;
//	//waveFmt.cbSize = -1;
//	
//
//	hr = XAudio2Create(&audioEngine, 0, XAUDIO2_DEFAULT_PROCESSOR);
//	H::System::ThrowIfFailed(hr);
//
//	hr = audioEngine->CreateMasteringVoice(&masteringVoice);
//	H::System::ThrowIfFailed(hr);
//
//#ifdef _DEBUG
//	XAUDIO2_DEBUG_CONFIGURATION debugConfiguration = { 0 };
//	debugConfiguration.BreakMask = XAUDIO2_LOG_ERRORS;
//	debugConfiguration.TraceMask = XAUDIO2_LOG_ERRORS;
//	audioEngine->SetDebugConfiguration(&debugConfiguration);
//#endif
//
//	this->Reset();
//}
//
//XAudio2Output::~XAudio2Output() {
//	if (sourceVoice) {
//		sourceVoice->DestroyVoice();
//	}
//}
//
//WAVEFORMATEX XAudio2Output::GetWaveFormat() {
//	return audioFormatExtensible.Format;
//}
//
//int64_t XAudio2Output::GetMinFrameTimeHns() {
//	return int64_t();
//}
//
//IPlaybackClock* XAudio2Output::GetPlaybackClock() {
//	return this->playbackClockAudio.get();
//}
//
//
//FloatAudioBuffer XAudio2Output::AllocateBuffer() {
//	std::unique_lock lk{ mx };
//
//	//cvPlayback.wait(lk, [this] {
//	//	return static_cast<bool>(isPlaying);
//	//	});
//
//	XAUDIO2_VOICE_STATE state;
//	sourceVoice->GetState(&state);
//
//	if (state.BuffersQueued >= MAX_COUNT_QUEUED_BUFFERS) {
//		LOG_DEBUG_D("audio buffer queue overflow, wait");
//		cvAudioBuffersQueueAvailable.wait(lk);
//	}
//
//	int64_t maxDuration = HMath::ConvertCeil((int64_t)audioBufferMaxDuration, 10000000LL, (int64_t)audioFormatExtensible.Format.nSamplesPerSec);
//
//	FloatAudioBuffer floatAudioBuffer;
//	floatAudioBuffer.waveFormatEx = audioFormatExtensible.Format;
//	floatAudioBuffer.duration = maxDuration;
//	floatAudioBuffer.dataSize = (size_t)floatAudioBuffer.duration * sizeof(float) * floatAudioBuffer.waveFormatEx.nChannels;
//	floatAudioBuffer.dataFloatCount = (int64_t)(floatAudioBuffer.dataSize / sizeof(float));
//
//	auto audioBuffer = std::make_unique<AudioBuffer>();
//	audioBuffer->bufferData.resize(floatAudioBuffer.dataSize);
//	audioBuffer->bufferDesc.pAudioData = audioBuffer->bufferData.data();
//	audioBuffer->bufferDesc.AudioBytes = static_cast<UINT32>(audioBuffer->bufferData.size());
//	//audioBuffer->bufferDesc.pContext = (void*)audioBuffer->bufferDesc.pAudioData;
//	audioBuffer->bufferDesc.pContext = (void*)audioBuffer.get();
////#ifdef _DEBUG
////	audioBuffer->idx = this->bufferIdx++;
////#endif
//
//	floatAudioBuffer.data = (float*)audioBuffer->bufferDesc.pAudioData;
//	
//	audioBuffers.push(std::move(audioBuffer));
//	return floatAudioBuffer;
//}
//
//
//void XAudio2Output::SendBuffer() {
//	std::unique_lock lk{ mx };
//	cvPlayback.wait(lk, [this] {
//		return static_cast<bool>(isPlaying);
//		});
//
//	sourceVoice->SubmitSourceBuffer(&audioBuffers.back()->bufferDesc);
//}
//
//
//void XAudio2Output::Play() {
//	std::unique_lock lk{ mx };
//	HRESULT hr = S_OK;
//
//	hr = sourceVoice->Start();
//	//hr = audioEngine->StartEngine();
//	//if (FAILED(hr)) {
//	//	Dbreak;
//	//}
//
//	//hr = sourceVoice->Discontinuity();
//	//if (FAILED(hr)) {
//	//	Dbreak;
//	//}
//
//	isPlaying = true;
//}
//
//void XAudio2Output::Pause() {
//	this->Stop();
//}
//
//void XAudio2Output::Stop() {
//	std::unique_lock lk{ mx };
//	HRESULT hr = S_OK;
//
//	//audioEngine->StopEngine();
//	//if (FAILED(hr)) {
//	//	Dbreak;
//	//}
//
//	hr = sourceVoice->Stop();
//	if (FAILED(hr)) {
//		Dbreak;
//	}
//
//	hr = sourceVoice->FlushSourceBuffers();
//	if (FAILED(hr)) {
//		Dbreak;
//	}
//
//	audioBuffers = {};
//	isPlaying = false;
//}
//
//// CHECK: Maybe enaugh Stop() logic instead recreating sourceVoice
//void XAudio2Output::Reset() {
//	std::unique_lock lk{ mx };
//	HRESULT hr = S_OK;
//
//	if (sourceVoice) {
//		sourceVoice->Stop();
//		sourceVoice->DestroyVoice();
//	}
//
//	hr = audioEngine->CreateSourceVoice(&sourceVoice, &audioFormatExtensible.Format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, voiceCallback.get());
//	H::System::ThrowIfFailed(hr);
//
//	hr = sourceVoice->FlushSourceBuffers();
//	if (FAILED(hr)) {
//		Dbreak;
//	}
//
//	audioBuffers = {};
//	isPlaying = false;
//
//	hr = sourceVoice->Start();
//	if (FAILED(hr)) {
//		Dbreak;
//	}
//}
//
//
//void XAudio2Output::OnAudioBufferEnd(void* pBufferContext) {
//	std::unique_lock lk{ mx };
//	auto context = reinterpret_cast<AudioBuffer*>(pBufferContext);
////#ifdef _DEBUG	
////	LOG_DEBUG_D("\n"
////		"context:\n"
////		"  ->idx = {}\n"
////		"  ->bufferData.data = {}\n"
////		, context->idx
////		, static_cast<void*>(context->bufferData.data())
////	);
////#endif
//	
//	if (audioBuffers.size() > 0) {
//		assert(audioBuffers.front()->bufferDesc.pAudioData == context->bufferData.data());
//		audioBuffers.pop();
//	}
//	else {
//		// this happens when we do flush
//	}
//	cvAudioBuffersQueueAvailable.notify_all();
//}
//
//
//
////
//// VoiceCallback
////
//XAudio2Output::VoiceCallback::VoiceCallback(XAudio2Output* parent)
//	: parent(parent)
//{
//}
//
//XAudio2Output::VoiceCallback::~VoiceCallback() {
//}
//
//STDMETHODIMP_(void) XAudio2Output::VoiceCallback::OnBufferEnd(THIS_ void* pBufferContext) {
//	this->parent->OnAudioBufferEnd(pBufferContext);
//}