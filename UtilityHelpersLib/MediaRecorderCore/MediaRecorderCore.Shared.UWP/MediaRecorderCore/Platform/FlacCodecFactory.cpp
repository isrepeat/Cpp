#include "pch.h"
#include "FlacCodecFactory.h"
#include "PlatformHelpers.h"

#include <libhelpers/HSystem.h>

struct FlacParams {
	static const uint32_t channels = 1;
	static const uint32_t sampleRate = 44100;
	static const uint32_t bitrate = 24;
};

Microsoft::WRL::ComPtr<IMFTransform> FlacCodecFactory::CreateIMFTransform() {
	return PlatformHelpers::CreateTransform2(L".flac",
		[](uint32_t &numChannels, uint32_t &sampleRate, uint32_t &bitsPerSample)
	{
		FlacCodecFactory::GetInParams(numChannels, sampleRate, bitsPerSample);
	},
		[](uint32_t i, uint32_t numChannels, uint32_t sampleRate, uint32_t bitsPerSample)
	{
		return FlacCodecFactory::CreateOutType(i, numChannels, sampleRate, bitsPerSample);
	});
}

void FlacCodecFactory::GetInParams(uint32_t & numChannels, uint32_t & sampleRate, uint32_t & bitsPerSample)
{
	numChannels = FlacParams::channels;
	sampleRate = FlacParams::sampleRate;
	bitsPerSample = FlacParams::bitrate;
}

Microsoft::WRL::ComPtr<IMFMediaType> FlacCodecFactory::CreateOutType(
	uint32_t typeIndex,
	uint32_t numChannels,
	uint32_t sampleRate,
	uint32_t bitsPerSample)
{
	if (typeIndex >= 2) {
		return nullptr;
	}

	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<IMFMediaType> type;

	hr = MFCreateMediaType(type.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	H::System::ThrowIfFailed(hr);

	hr = type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_FLAC);
	H::System::ThrowIfFailed(hr);

	hr = type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, numChannels);
	H::System::ThrowIfFailed(hr);

	hr = type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sampleRate);
	H::System::ThrowIfFailed(hr);

	hr = type->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 128);
	H::System::ThrowIfFailed(hr);

	if (typeIndex >= 1) {
		hr = type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample);
		H::System::ThrowIfFailed(hr);

		hr = type->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, 1);
		H::System::ThrowIfFailed(hr);

		hr = type->SetUINT32(MF_MT_AUDIO_VALID_BITS_PER_SAMPLE, bitsPerSample);
		H::System::ThrowIfFailed(hr);
	}

	return type;
}
//1
//MF_MT_AUDIO_AVG_BYTES_PER_SECOND = 128
//MF_MT_AUDIO_NUM_CHANNELS = 1
//MF_MT_MAJOR_TYPE = MFMediaType_Audio
//MF_MT_AUDIO_SAMPLES_PER_SECOND = 44100
//MF_MT_ALL_SAMPLES_INDEPENDENT = 1
//MF_MT_AUDIO_VALID_BITS_PER_SAMPLE = 24
//MF_MT_AUDIO_BITS_PER_SAMPLE = 24
//MF_MT_SUBTYPE = MFAudioFormat_FLAC

