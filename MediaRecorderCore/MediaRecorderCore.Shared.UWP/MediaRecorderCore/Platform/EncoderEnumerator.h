#pragma once
#include "Platform/IEncoderEnumerator.h"

class EncoderEnumerator : public IEncoderEnumerator {
public:
    void EnumAudio(std::function<void(const GUID &)> fn) override;
    void EnumVideo(std::function<void(const GUID &)> fn) override;
};