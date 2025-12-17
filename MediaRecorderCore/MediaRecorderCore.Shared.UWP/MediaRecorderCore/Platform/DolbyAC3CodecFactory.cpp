#include "pch.h"
#include "DolbyAC3CodecFactory.h"
#include "PlatformHelpers.h"

#include <libhelpers/HSystem.h>

struct DolbyAC3Params {
	static const uint32_t channels = 1;
	static const uint32_t sampleRate = 48000;
	static const uint32_t bitrate = 16;
};

Microsoft::WRL::ComPtr<IMFTransform> DolbyAC3CodecFactory::CreateIMFTransform() {
	return PlatformHelpers::CreateTransform2(L".m4a",
		[](uint32_t &numChannels, uint32_t &sampleRate, uint32_t &bitsPerSample)
	{
		DolbyAC3CodecFactory::GetInParams(numChannels, sampleRate, bitsPerSample);
	},
		[](uint32_t i, uint32_t numChannels, uint32_t sampleRate, uint32_t bitsPerSample)
	{
		return DolbyAC3CodecFactory::CreateOutType(i, numChannels, sampleRate, bitsPerSample);
	});
}

void DolbyAC3CodecFactory::GetInParams(uint32_t & numChannels, uint32_t & sampleRate, uint32_t & bitsPerSample)
{
	numChannels = DolbyAC3Params::channels;
	sampleRate = DolbyAC3Params::sampleRate;
	bitsPerSample = DolbyAC3Params::bitrate;
}

Microsoft::WRL::ComPtr<IMFMediaType> DolbyAC3CodecFactory::CreateOutType(
	uint32_t typeIndex,
	uint32_t numChannels,
	uint32_t sampleRate,
	uint32_t /*bitsPerSample*/)
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

	hr = type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Dolby_AC3);
	H::System::ThrowIfFailed(hr);

	hr = type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, numChannels);
	H::System::ThrowIfFailed(hr);

	hr = type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sampleRate);
	H::System::ThrowIfFailed(hr);

	if (typeIndex >= 1) {
		hr = type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 24000);
		H::System::ThrowIfFailed(hr);

		hr = type->SetUINT32(MF_MT_AUDIO_CHANNEL_MASK, 4);
		H::System::ThrowIfFailed(hr);
	}

	return type;
}
//1 :
//MF_MT_AUDIO_AVG_BYTES_PER_SECOND = 24000
//MF_MT_AUDIO_NUM_CHANNELS = 1
//MF_MT_MAJOR_TYPE = MFMediaType_Audio
//MF_MT_AUDIO_CHANNEL_MASK = 4
//MF_MT_AUDIO_SAMPLES_PER_SECOND = 48000
//MF_MT_SUBTYPE = MFAudioFormat_Dolby_AC3


