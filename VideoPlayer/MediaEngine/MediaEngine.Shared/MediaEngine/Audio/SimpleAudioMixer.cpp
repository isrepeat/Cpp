#include "SimpleAudioMixer.h"
#include "../HMath.h"
#include <algorithm>

void SimpleAudioMixer::Mix(SimpleMixerParams* params) {
	float invCount = 1.f;
	if (params->audioSourceCount > 1) {
		invCount = 1.0f / (float)params->audioSourceCount;
	}

	assert(params->bufferData.size() == params->sampleData.size());

	for (int64_t i = 0; i < params->bufferData.size(); i++) {
		float inBuffer = params->bufferData[i];
		float toBuffer = params->sampleData[i] * invCount;
		float sample = (inBuffer + toBuffer);

		sample = HMath::Clamp(sample, -1.0f, 1.0f);

		params->bufferData[i] = sample;
	}
}


// Mb move to Helpers?
template<typename T>
struct Range {
	T start;
	T end;
};

AudioSampleMixStatus DefaultAudioMixer::MixAudioSample(
	MF::MFAudioSample* mfAudioSample,
	FloatAudioBuffer* floatAudioBuffer,
	IPlaybackClock* playbackClock,
	H::Chrono::Hns /*startPts*/,
	size_t sourceCount)
{
	if (!mfAudioSample || !floatAudioBuffer)
		return AudioSampleMixStatus::InvalidData;

	H::Rational<int64_t>::Units units{ 1, floatAudioBuffer->waveFormatEx.nSamplesPerSec };
	using Range_t = Range<H::Rational<int64_t>>;

	Range_t sampleRange{ units, units };
	sampleRange.start = mfAudioSample->offset + mfAudioSample->pts/* - startPts*/;
	sampleRange.end = sampleRange.start + mfAudioSample->duration;

	Range_t bufferRange{ units, units };
	bufferRange.start = playbackClock->GetLastPlaybackPts();
	bufferRange.end = bufferRange.start + floatAudioBuffer->duration;

	//LOG_DEBUG_D("\n"
	//	"trackId = {}\n"
	//	"mfAudioSample: offset = {}ms, pts = {}ms\n"
	//	"sampleRange = {{{}, {}}} [ms]\n"
	//	"bufferRange = {{{}, {}}} [ms]\n"
	//	, mfAudioSample->trackId
	//	, H::Chrono::milliseconds_f{ mfAudioSample->offset }.count(), H::Chrono::milliseconds_f{ mfAudioSample->pts }.count()
	//	, H::Chrono::milliseconds_f{ sampleRange.start }.count(), H::Chrono::milliseconds_f{ sampleRange.end }.count()
	//	, H::Chrono::milliseconds_f{ bufferRange.start }.count(), H::Chrono::milliseconds_f{ bufferRange.end }.count()
	//);

	if (sampleRange.end < bufferRange.start) {
		return AudioSampleMixStatus::OldSample;
	}

	if (sampleRange.start > bufferRange.end) {
		return AudioSampleMixStatus::FutureSample;
	}

	// calculate sample mix range that constrained by buffer range
	Range_t sampleRangeMix{ units, units };
	sampleRangeMix.start = HMath::Clamp(sampleRange.start, bufferRange.start, bufferRange.end);
	sampleRangeMix.end = HMath::Clamp(sampleRange.end, bufferRange.start, bufferRange.end);

	// calculate buffer mix range that constrained by previous sample mix range
	Range_t bufferRangeMix{ units, units };
	bufferRangeMix.start = HMath::Clamp(bufferRange.start, sampleRangeMix.start, sampleRangeMix.end);
	bufferRangeMix.end = HMath::Clamp(bufferRange.end, sampleRangeMix.start, sampleRangeMix.end);

	auto sampleMixStartIdx = (sampleRangeMix.start - sampleRange.start).Value();
	auto sampleMixEndIdx = (sampleRangeMix.end - sampleRange.start).Value();

	auto bufferMixStartIdx = (bufferRangeMix.start - bufferRange.start).Value();
	auto bufferMixEndIdx = (bufferRangeMix.end - bufferRange.start).Value();

	auto mfAudioSampleDataAccessor = mfAudioSample->GetDataAccessor();
	float* samlpeFloatData = static_cast<float*>(mfAudioSampleDataAccessor.GetData());

	SimpleMixerParams simpleParams;
	simpleParams.audioSourceCount = sourceCount;
	// TODO: add out-of-bounds check
	simpleParams.sampleData = H::span<float>{
		samlpeFloatData + sampleMixStartIdx * floatAudioBuffer->waveFormatEx.nChannels,
		samlpeFloatData + sampleMixEndIdx * floatAudioBuffer->waveFormatEx.nChannels
	};
	simpleParams.bufferData = H::span<float>{
		floatAudioBuffer->data + bufferMixStartIdx * floatAudioBuffer->waveFormatEx.nChannels,
		floatAudioBuffer->data + bufferMixEndIdx * floatAudioBuffer->waveFormatEx.nChannels
	};

	this->simpleAudioMixer.Mix(&simpleParams);

	if (sampleRange.end > bufferRange.end) {
		return AudioSampleMixStatus::NotCompleted;
	}

	return AudioSampleMixStatus::Completed;
}