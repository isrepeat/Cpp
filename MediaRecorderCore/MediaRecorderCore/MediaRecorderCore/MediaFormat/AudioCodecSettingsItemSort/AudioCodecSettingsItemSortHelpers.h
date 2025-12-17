#pragma once
#include "CmpOrder.h"

#include <cstdint>
#include <optional>

namespace AudioCodecSettingsItemSort {
    CmpOrder IsFirstGreaterOrCloserToTarget(
        const std::optional<uint32_t>& first,
        const std::optional<uint32_t>& second,
        const std::optional<uint32_t>& target);

    bool IsAllNullOrAllValue(
        const std::optional<uint32_t>& a,
        const std::optional<uint32_t>& b);

    uint32_t GetIncrementForNullOrValueOverlap(
        const std::optional<uint32_t>& a,
        const std::optional<uint32_t>& b);

    template <typename T, typename U>
    auto CreateOptionalFromMember(const std::optional<T>& obj, U T::* member) {
        return obj ? std::optional((*obj).*member) : std::nullopt;
    }

    template <typename T, typename U, typename T2, typename U2>
    auto CreateOptionalFromMemberFn(U T::* member, U2 T2::* subMember) {
        return [member, subMember](const T& obj)
        {
            return CreateOptionalFromMember(obj.*member, subMember);
        };
    }
}
