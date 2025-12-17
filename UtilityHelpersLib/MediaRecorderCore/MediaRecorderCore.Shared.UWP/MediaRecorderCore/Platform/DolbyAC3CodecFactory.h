#pragma once
#include "Platform/IDolbyAC3CodecFactory.h"

class DolbyAC3CodecFactory : public IDolbyAC3CodecFactory {
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
