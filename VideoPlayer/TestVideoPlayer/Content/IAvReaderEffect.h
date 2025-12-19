#pragma once
#include "MediaSample.h"
#include <memory>

class IAvReaderEffect {
public:
    IAvReaderEffect() = default;
    virtual ~IAvReaderEffect() = default;

    virtual std::unique_ptr<MF::MFVideoSample> Process(std::unique_ptr<MF::MFSample> mfSample) = 0;
};