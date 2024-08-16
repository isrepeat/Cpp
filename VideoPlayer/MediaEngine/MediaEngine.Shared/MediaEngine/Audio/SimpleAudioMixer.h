#pragma once
#include <Helpers/Span.hpp>
#include "../PlaybackClock.h"
#include "../MediaSample.h"

class SimpleMixerParams {
public:
	size_t audioSourceCount = 0;
	H::span<float> sampleData;
	H::span<float> bufferData;
};

class SimpleAudioMixer {
public:
	SimpleAudioMixer() = default;

	void Mix(SimpleMixerParams* params);
};

struct AudioBufferDesc {
	const WAVEFORMATEX waveFormatEx;
	H::Rational<int64_t> duration; // [in sampleRate units] - number of requested audio samples
	H::Rational<int64_t> pts; // [in sampleRate units]

	AudioBufferDesc(WAVEFORMATEX waveFormatEx)
		: waveFormatEx{ waveFormatEx }
		, duration{ 1, waveFormatEx.nSamplesPerSec }
		, pts{ 1, waveFormatEx.nSamplesPerSec }
	{}
};

struct FloatAudioBuffer : AudioBufferDesc {
	float* data = nullptr;
	int64_t dataSize = 0; // allocated buffer size in bytes [duration * sizeof(float) * waveFormatEx.nChannels]
	int64_t dataFloatCount = 0; // allocated buffer size in float units [dataSize / sizeof(float)]

	FloatAudioBuffer(WAVEFORMATEX waveFormatEx)
		: AudioBufferDesc(waveFormatEx)
	{}
};

enum class AudioSampleMixStatus {
	InvalidData,
	NotCompleted,
	OldSample,
	FutureSample,
	Completed,
};

class DefaultAudioMixer {
public:
	DefaultAudioMixer() = default;
	~DefaultAudioMixer() = default;

	AudioSampleMixStatus MixAudioSample(
		MF::MFAudioSample* mfAudioSample,
		FloatAudioBuffer* floatAudioBuffer,
		IPlaybackClock* playbackClock,
		H::Chrono::Hns startPts = 0_hns,
		size_t sourceCount = 1
	);

private:
	SimpleAudioMixer simpleAudioMixer;
};