#pragma once
#include "IMediaRecorder.h"

#include <memory>

class IMediaRecorderFactory {
public:
    IMediaRecorderFactory() {}
    virtual ~IMediaRecorderFactory() {}

    virtual std::unique_ptr<IMediaRecorder> CreateMediaRecorder() = 0;
};