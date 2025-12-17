#pragma once
#include "Platform/IAmrNbCodecFactory.h"

class AmrNbCodecFactory : public IAmrNbCodecFactory {
public:
    Microsoft::WRL::ComPtr<IMFTransform> CreateIMFTransform() override;

private:
    static Microsoft::WRL::ComPtr<IMFMediaType> CreateOutType(
        uint32_t typeIndex,
        uint32_t numChannels,
        uint32_t sampleRate,
        uint32_t bitsPerSample);
};