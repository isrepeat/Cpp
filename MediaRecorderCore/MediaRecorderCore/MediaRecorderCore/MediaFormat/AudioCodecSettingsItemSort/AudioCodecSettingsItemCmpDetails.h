#pragma once
#include "..\AudioCodecSettingsItem.h"
#include "AudioCodecSettingsItemSortHelpers.h"

class AudioCodecSettingsItemCmpDetails {
public:
    /*
    * Sort order
    * If target has field with nullopt then nullopt for that field will be first, after it values will start from min to max
    * If target has field with value then values will be sorted by IsFirstGreaterOrCloserToTarget and priority is numChannels, sampleRate, bitrate
    *
    * For target field with value first/second nullopt fileds in are most distant. For example:
    * target.numChannels = 9, first.numChannels = { nullopt, 1, 2, 9, 10 }
    * result.numChannels = { 9, 10, 2, 1, nullopt }
    *
    * For target field with nullopt first/second nullopt fileds are most closest. For example:
    * target.numChannels = nullopt, first.numChannels = { nullopt, 1, 2, 9, 10 }
    * result.numChannels = { nullopt, 1, 2, 9, 10 }
    */
    static auto MakeComparatorBase(const AudioCodecSettingsItem& target) {
        using namespace AudioCodecSettingsItemSort;

        auto getNumChannels = CreateOptionalFromMemberFn(&AudioCodecSettingsItem::basicSettings, &AudioCodecBasicSettings::numChannels);
        auto getSampleRate = CreateOptionalFromMemberFn(&AudioCodecSettingsItem::basicSettings, &AudioCodecBasicSettings::sampleRate);
        auto getBitrate = CreateOptionalFromMemberFn(&AudioCodecSettingsItem::bitrateSettings, &AudioCodecBitrateSettings::bitrate);

        return [&target, getNumChannels, getSampleRate, getBitrate](const AudioCodecSettingsItem& first, const AudioCodecSettingsItem& second) {
            auto optNumChannelsA = getNumChannels(first);
            auto optNumChannelsB = getNumChannels(second);
            auto optNumChannelsC = getNumChannels(target);

            auto optSampleRateA = getSampleRate(first);
            auto optSampleRateB = getSampleRate(second);
            auto optSampleRateC = getSampleRate(target);

            auto optBitrateA = getBitrate(first);
            auto optBitrateB = getBitrate(second);
            auto optBitrateC = getBitrate(target);

            uint32_t overlapCountAC = 0;
            uint32_t overlapCountBC = 0;

            overlapCountAC += GetIncrementForNullOrValueOverlap(optNumChannelsA, optNumChannelsC);
            overlapCountAC += GetIncrementForNullOrValueOverlap(optSampleRateA, optSampleRateC);
            overlapCountAC += GetIncrementForNullOrValueOverlap(optBitrateA, optBitrateC);

            overlapCountBC += GetIncrementForNullOrValueOverlap(optNumChannelsB, optNumChannelsC);
            overlapCountBC += GetIncrementForNullOrValueOverlap(optSampleRateB, optSampleRateC);
            overlapCountBC += GetIncrementForNullOrValueOverlap(optBitrateB, optBitrateC);

            if (overlapCountAC == overlapCountBC) {
                if (optNumChannelsA != optNumChannelsB) {
                    return IsFirstGreaterOrCloserToTarget(optNumChannelsA, optNumChannelsB, optNumChannelsC);
                }

                if (optSampleRateA != optSampleRateB) {
                    return IsFirstGreaterOrCloserToTarget(optSampleRateA, optSampleRateB, optSampleRateC);
                }

                return IsFirstGreaterOrCloserToTarget(optBitrateA, optBitrateB, optBitrateC);
            }

            if (overlapCountAC > overlapCountBC) {
                // First is before Second if First has more nullopt/values overlaps with Target
                // First is more similar to Target by nullopt/values overlaps
                return CmpOrder::FirstIsBeforeSecond;
            }

            return CmpOrder::FirstIsNotBeforeSecond;
        };
    }
};
