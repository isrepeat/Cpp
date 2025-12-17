#pragma once

#include <cstdint>
#include <libhelpers/MediaFoundation/MFInclude.h>

class IAmrNbCodecFactory {
public:
    IAmrNbCodecFactory() {}
    virtual ~IAmrNbCodecFactory() {}

    virtual Microsoft::WRL::ComPtr<IMFTransform> CreateIMFTransform() = 0;
};