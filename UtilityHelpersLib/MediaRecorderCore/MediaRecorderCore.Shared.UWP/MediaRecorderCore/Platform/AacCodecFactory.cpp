#include "pch.h"
#include "AacCodecFactory.h"
#include "PlatformHelpers.h"

#include <libhelpers/HSystem.h>

struct AacParams {
	static const uint32_t channels = 2;
	static const uint32_t sampleRate = 44100;
	static const uint32_t bitrate = 16;
};

Microsoft::WRL::ComPtr<IMFTransform> AacCodecFactory::CreateIMFTransform() {
	return PlatformHelpers::CreateTransform2(L".m4a",
		[](uint32_t &numChannels, uint32_t &sampleRate, uint32_t &bitsPerSample)
	{
		AacCodecFactory::GetInParams(numChannels, sampleRate, bitsPerSample);
	},
		[](uint32_t i, uint32_t numChannels, uint32_t sampleRate, uint32_t bitsPerSample)
	{
		return AacCodecFactory::CreateOutType(i, numChannels, sampleRate, bitsPerSample);
	});
}

void AacCodecFactory::GetInParams(uint32_t & numChannels, uint32_t & sampleRate, uint32_t & bitsPerSample)
{
	numChannels = AacParams::channels;
	sampleRate = AacParams::sampleRate;
	bitsPerSample = AacParams::bitrate;
}

Microsoft::WRL::ComPtr<IMFMediaType> AacCodecFactory::CreateOutType(
	uint32_t typeIndex,
	uint32_t numChannels,
	uint32_t sampleRate,
	uint32_t bitsPerSample)
{
	if (typeIndex >= 3) {
		return nullptr;
	}

	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<IMFMediaType> type;

	hr = MFCreateMediaType(type.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	H::System::ThrowIfFailed(hr);

	hr = type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
	H::System::ThrowIfFailed(hr);

	hr = type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, numChannels);
	H::System::ThrowIfFailed(hr);

	hr = type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sampleRate);
	H::System::ThrowIfFailed(hr);

	if (typeIndex >= 1)
	{
		hr = type->SetUINT32(MF_MT_AVG_BITRATE, 128000);
		H::System::ThrowIfFailed(hr);

		hr = type->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 128000 / 8);
		H::System::ThrowIfFailed(hr);

		hr = type->SetUINT32(MF_MT_COMPRESSED, 1);
		H::System::ThrowIfFailed(hr);

		hr = type->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 1);
		H::System::ThrowIfFailed(hr);

		hr = type->SetUINT32(MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION, 41);
		H::System::ThrowIfFailed(hr);

		hr = type->SetUINT32(MF_MT_AUDIO_PREFER_WAVEFORMATEX, 1);
		H::System::ThrowIfFailed(hr);
	}
	if (typeIndex >= 2)
	{
		BYTE aUserData[14] = { 0, 0, 41, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 8 };

		hr = type->SetBlob(MF_MT_USER_DATA, aUserData, 14);
		H::System::ThrowIfFailed(hr);

		hr = type->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, 0);
		H::System::ThrowIfFailed(hr);

		hr = type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample);
		H::System::ThrowIfFailed(hr);

		hr = type->SetUINT32(MF_MT_AAC_PAYLOAD_TYPE, 0);
		H::System::ThrowIfFailed(hr);
	}
	return type;
}
//1
//MF_MT_AUDIO_AVG_BYTES_PER_SECOND = 16000
//MF_MT_AVG_BITRATE = 128000
//MF_MT_AUDIO_BLOCK_ALIGNMENT = 1
//MF_MT_AUDIO_NUM_CHANNELS = 1
//MF_MT_COMPRESSED = 1
//MF_MT_MAJOR_TYPE = MFMediaType_Audio
//MF_MT_AUDIO_SAMPLES_PER_SECOND = 44100
//MF_MT_AM_FORMAT_TYPE = { 05589F81 - C356 - 11CE - BF01 - 00AA0055595A }
//MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION = 41
//MF_MT_AUDIO_PREFER_WAVEFORMATEX = 1
//MF_MT_USER_DATA = 0; 0; 41; 0; 0; 0; 0; 0; 0; 0; 0; 0; 18; 8
//MF_MT_FIXED_SIZE_SAMPLES = 0
//MF_MT_AAC_PAYLOAD_TYPE = 0
//MF_MT_AUDIO_BITS_PER_SAMPLE = 16
//MF_MT_SUBTYPE = MFAudioFormat_AAC


