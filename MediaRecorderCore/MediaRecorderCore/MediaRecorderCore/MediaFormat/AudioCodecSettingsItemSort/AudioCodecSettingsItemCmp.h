#pragma once
#include "AudioCodecSettingsItemCmpDetails.h"

class AudioCodecSettingsItemCmp {
public:
    static auto AudioCodecSettingsItemCmp::Make(const AudioCodecSettingsItem& target) {
        return[base = AudioCodecSettingsItemCmpDetails::MakeComparatorBase(target)](const AudioCodecSettingsItem& first, const AudioCodecSettingsItem& second)
        {
            bool firstBeforeSecond = base(first, second) == CmpOrder::FirstIsBeforeSecond;
            return firstBeforeSecond;
        };
    }
};
