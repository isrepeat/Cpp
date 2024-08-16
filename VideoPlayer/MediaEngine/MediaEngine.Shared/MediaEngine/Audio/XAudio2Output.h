//#pragma once
//#include "IAudioOutput.h"
//#include "IAudioClientFactory.h"
//#include "../MediaSample.h"
//#include <xaudio2.h>
//#include <queue>
//
//class PlaybackClockXAudio2 : public IPlaybackClock {
//public:
//	PlaybackClockXAudio2(Microsoft::WRL::ComPtr<IAudioClock> audioClock, WAVEFORMATEX waveFmt);
//	~PlaybackClockXAudio2();
//
//	void Start() override;
//	void Pause() override;
//	void Stop() override;
//	void SetOffsetPts(H::Chrono::Hns offsetPts) override;
//	void UpdateLastPlaybackPts(H::Chrono::Hns lastPlaybackPts) override;
//	H::Chrono::Hns GetLastPlaybackPts() override;
//	H::Chrono::Hns GetPlaybackPts() override;
//
//private:
//	std::mutex mx;
//	Microsoft::WRL::ComPtr<IAudioClock> audioClock;
//	WAVEFORMATEX waveFmt;
//	H::Chrono::Hns offsetPts;
//};
//
//
//class XAudio2Output : public IAudioOutput {
//public:
//	XAudio2Output(IAudioClientFactory* audioClientFactory, H::Chrono::Hns audioBufferMaxDuration);
//	virtual ~XAudio2Output();
//
//	WAVEFORMATEX GetWaveFormat() override;
//	int64_t GetMinFrameTimeHns() override;
//
//	IPlaybackClock* GetPlaybackClock() override;
//
//	FloatAudioBuffer AllocateBuffer() override;
//	void SendBuffer() override;
//
//	void Play() override;
//	void Pause() override;
//	void Stop() override;
//	void Reset() override;
//
//private:
//	void OnAudioBufferEnd(void* pBufferContext);
//
//private:
//	class VoiceCallback : public IXAudio2VoiceCallback {
//	public:
//		VoiceCallback(XAudio2Output* parent);
//		virtual ~VoiceCallback();
//
//		// Called just before this voice's processing pass begins.
//		STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired) override {};
//
//		// Called just after this voice's processing pass ends.
//		STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS) override {};
//
//		// Called when this voice has just finished isPlaying a buffer stream
//		// (as marked with the XAUDIO2_END_OF_STREAM flag on the last buffer).
//		STDMETHOD_(void, OnStreamEnd) (THIS) override {};
//
//		// Called when this voice is about to start processing a new buffer.
//		STDMETHOD_(void, OnBufferStart) (THIS_ void* pBufferContext) override {};
//
//		// Called when this voice has just finished processing a buffer.
//		// The buffer can now be reused or destroyed.
//		STDMETHOD_(void, OnBufferEnd) (THIS_ void* pBufferContext) override;
//
//		// Called when this voice has just reached the end position of a loop.
//		STDMETHOD_(void, OnLoopEnd) (THIS_ void* pBufferContext) override {};
//
//		// Called in the event of a critical error during voice processing,
//		// such as a failing xAPO or an error from the hardware XMA decoder.
//		// The voice may have to be destroyed and re-created to recover from
//		// the error.  The callback arguments report which buffer was being
//		// processed when the error occurred, and its HRESULT code.
//		STDMETHOD_(void, OnVoiceError) (THIS_ void* pBufferContext, HRESULT Error) override {};
//
//	private:
//		XAudio2Output* parent;
//	};
//
//private:
//	struct AudioBuffer {
//		std::vector<BYTE> bufferData;
//		XAUDIO2_BUFFER bufferDesc;
////#ifdef _DEBUG
////		int idx = 0;
////#endif
//	};
//
//private:
//	static constexpr int MAX_COUNT_QUEUED_BUFFERS = 5;
//
//	std::mutex mx;
//	Microsoft::WRL::ComPtr<IAudioClient> audioClient;
//	const H::Chrono::Hns audioBufferMaxDuration;
//	std::unique_ptr<VoiceCallback> voiceCallback;
//	std::unique_ptr<PlaybackClockXAudio2> playbackClockAudio;
//	
//	WAVEFORMATEXTENSIBLE audioFormatExtensible;
//	Microsoft::WRL::ComPtr<IXAudio2> audioEngine;
//	IXAudio2MasteringVoice* masteringVoice;
//	IXAudio2SourceVoice* sourceVoice;
//
//	std::queue<std::unique_ptr<AudioBuffer>> audioBuffers;
//	std::condition_variable cvAudioBuffersQueueAvailable;
//
//	std::atomic<bool> isPlaying = false;
//	std::condition_variable cvPlayback;
//
////#ifdef _DEBUG	
////	int bufferIdx = 0;
////#endif
//};