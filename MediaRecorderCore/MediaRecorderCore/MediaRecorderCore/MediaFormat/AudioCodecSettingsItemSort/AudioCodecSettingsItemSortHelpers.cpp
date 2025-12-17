#include "pch.h"
#include "AudioCodecSettingsItemSortHelpers.h"
#include "AbsOptionalDistance.h"

#include <cassert>

namespace AudioCodecSettingsItemSort {
    CmpOrder IsFirstGreaterOrCloserToTarget(
        const std::optional<uint32_t>& first,
        const std::optional<uint32_t>& second,
        const std::optional<uint32_t>& target)
    {
        if (first == target && second == target) {
            return CmpOrder::FirstIsNotBeforeSecond;
        }
        else if (
            (first >= target && second >= target) ||
            (first <= target && second <= target)
            )
        {
            // first and second both or greater equals to target or both less equals to target
            // if first closer then first should be before second(FirstIsBeforeSecond)
            // otherwise FirstIsNotBeforeSecond
            auto distFirstToTarget = AbsOptionalDistance(first, target);
            auto distSecondToTarget = AbsOptionalDistance(second, target);

            if (distFirstToTarget < distSecondToTarget) {
                return CmpOrder::FirstIsBeforeSecond;
            }

            return CmpOrder::FirstIsNotBeforeSecond;
        }
        else if (first > target) {
            // after previos test second must be less than target
            assert(second < target&& second != target);
            return CmpOrder::FirstIsBeforeSecond;
        }
        else if (second > target) {
            // after previos test first must be less than target
            assert(first < target&& first != target);
            return CmpOrder::FirstIsNotBeforeSecond;
        }

        // must not happen
        assert(false);
        return CmpOrder::FirstIsNotBeforeSecond;
    }

    bool IsAllNullOrAllValue(
        const std::optional<uint32_t>& a,
        const std::optional<uint32_t>& b)
    {
        return (a && b) || (!a && !b);
    }

    uint32_t GetIncrementForNullOrValueOverlap(
        const std::optional<uint32_t>& a,
        const std::optional<uint32_t>& b)
    {
        if (IsAllNullOrAllValue(a, b)) {
            return 1;
        }

        return 0;
    }
}
