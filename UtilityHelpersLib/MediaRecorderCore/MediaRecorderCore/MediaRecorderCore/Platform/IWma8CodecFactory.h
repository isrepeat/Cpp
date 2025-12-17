#pragma once

#include <cstdint>
#include <libhelpers/MediaFoundation/MFInclude.h>

class IWma8CodecFactory {
public:
    IWma8CodecFactory() {}
    virtual ~IWma8CodecFactory() {}

    virtual Microsoft::WRL::ComPtr<IMFTransform> CreateIMFTransform() = 0;
};