#pragma once
#include "AudioCodecTypeDef.h"

enum class AudioCodecType {
    AudioCodecType_Enum

    CountHelper,
    Count = CountHelper - 1, // don't count Unknown
    First = AAC
};