#pragma once
#include "Platform/IAacCodecFactory.h"

class AacCodecFactory : public IAacCodecFactory {
public:
	Microsoft::WRL::ComPtr<IMFTransform> CreateIMFTransform() override;

private:
	static Microsoft::WRL::ComPtr<IMFMediaType> CreateOutType(
		uint32_t typeIndex,
		uint32_t numChannels,
		uint32_t sampleRate,
		uint32_t bitsPerSample);

	static void GetInParams(uint32_t &numChannels, uint32_t &sampleRate, uint32_t &bitsPerSample);
};
