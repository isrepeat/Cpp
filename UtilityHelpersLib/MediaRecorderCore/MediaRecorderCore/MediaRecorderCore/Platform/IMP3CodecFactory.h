#pragma once

#include <cstdint>
#include <libhelpers/MediaFoundation/MFInclude.h>

class IMP3CodecFactory {
public:
    IMP3CodecFactory() {}
    virtual ~IMP3CodecFactory() {}

    virtual Microsoft::WRL::ComPtr<IMFTransform> CreateIMFTransform() = 0;
};