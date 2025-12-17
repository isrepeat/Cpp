#include "pch.h"
#include "AlacCodecFactory.h"
#include "PlatformHelpers.h"

#include <libhelpers/HSystem.h>

struct AlacParams {
	static const uint32_t channels = 1;
	static const uint32_t sampleRate = 44100;
	static const uint32_t bitrate = 24;
};

Microsoft::WRL::ComPtr<IMFTransform> AlacCodecFactory::CreateIMFTransform() {
	return PlatformHelpers::CreateTransform2(L".m4a",
		[](uint32_t &numChannels, uint32_t &sampleRate, uint32_t &bitsPerSample)
	{
		AlacCodecFactory::GetInParams(numChannels, sampleRate, bitsPerSample);
	},
		[](uint32_t i, uint32_t numChannels, uint32_t sampleRate, uint32_t bitsPerSample)
	{
		return AlacCodecFactory::CreateOutType(i, numChannels, sampleRate, bitsPerSample);
	});
}

void AlacCodecFactory::GetInParams(uint32_t & numChannels, uint32_t & sampleRate, uint32_t & bitsPerSample)
{
	numChannels = AlacParams::channels;
	sampleRate = AlacParams::sampleRate;
	bitsPerSample = AlacParams::bitrate;
}

Microsoft::WRL::ComPtr<IMFMediaType> AlacCodecFactory::CreateOutType(
	uint32_t typeIndex,
	uint32_t numChannels,
	uint32_t sampleRate,
	uint32_t bitsPerSample)
{
	if (typeIndex >= 1) {
		return nullptr;
	}

	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<IMFMediaType> type;

	hr = MFCreateMediaType(type.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	H::System::ThrowIfFailed(hr);

	hr = type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_ALAC);
	H::System::ThrowIfFailed(hr);

	hr = type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, numChannels);
	H::System::ThrowIfFailed(hr);

	hr = type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sampleRate);
	H::System::ThrowIfFailed(hr);

	hr = type->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, (bitsPerSample / 8) * numChannels * sampleRate);
	H::System::ThrowIfFailed(hr);

	return type;
}
//1
//MF_MT_AUDIO_AVG_BYTES_PER_SECOND = 132300
//MF_MT_AUDIO_NUM_CHANNELS = 1
//MF_MT_MAJOR_TYPE = MFMediaType_Audio
//MF_MT_AUDIO_SAMPLES_PER_SECOND = 44100
//MF_MT_AUDIO_BITS_PER_SAMPLE = 24
//MF_MT_SUBTYPE = MFAudioFormat_ALAC


