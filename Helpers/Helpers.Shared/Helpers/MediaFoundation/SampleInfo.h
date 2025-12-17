#pragma once
#include <Helpers/common.h>
#include <Helpers/Time.h>

namespace MEDIA_FOUNDATION_NS {
    struct SampleInfo {
        const HELPERS_NS::Chrono::Hns pts;
        const HELPERS_NS::Chrono::Hns duration;
        const HELPERS_NS::Chrono::Hns nextSamplePts;
        const HELPERS_NS::Chrono::Hns offset;

        SampleInfo()
            : pts{ 0_hns }
            , duration{ 0_hns }
            , nextSamplePts{ 0_hns }
            , offset{ 0_hns }
        {}

        SampleInfo(HELPERS_NS::Chrono::Hns pts, HELPERS_NS::Chrono::Hns duration, HELPERS_NS::Chrono::Hns offset = 0_hns)
            : pts{ pts }
            , duration{ duration }
            , nextSamplePts{ pts + duration }
            , offset{ offset }
        {}
    };
}