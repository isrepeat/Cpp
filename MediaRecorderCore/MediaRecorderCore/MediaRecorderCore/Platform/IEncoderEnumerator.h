#pragma once

#include <functional>
#include <Windows.h>

class IEncoderEnumerator {
public:
    IEncoderEnumerator() {}
    virtual ~IEncoderEnumerator() {}

    virtual void EnumAudio(std::function<void(const GUID &)> fn) = 0;
    virtual void EnumVideo(std::function<void(const GUID &)> fn) = 0;
};