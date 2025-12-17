#pragma once
#include "MediaRecorderMessageEnumDef.h"

namespace Native {
    enum class MediaRecorderMessageEnum {
        MediaRecorderMessageEnum_Enum
    };

    struct MediaRecorderEventArgs {
        MediaRecorderMessageEnum message = MediaRecorderMessageEnum::Nothing;
        int remainingTime = 0;
    };
}
