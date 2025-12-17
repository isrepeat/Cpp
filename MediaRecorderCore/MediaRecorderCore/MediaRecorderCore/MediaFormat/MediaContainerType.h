#pragma once
#include "MediaContainerTypeDef.h"

#include <cstdint>

enum class MediaContainerType : uint32_t {
    MediaContainerType_Enum

    CountHelper,
    Count = CountHelper - 1,
    First = MP4
};