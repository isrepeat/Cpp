#include "pch.h"
#include "TimeUtils.h"

namespace MediaEngineWinRT {
    int64_t MFTime::ToMsec(int64_t mfTime) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(H::Chrono::Hns(mfTime)).count();
    }

    int64_t MFTime::ToSeconds(int64_t mfTime) {
        return std::chrono::duration_cast<std::chrono::seconds>(H::Chrono::Hns(mfTime)).count();
    }

    int64_t MFTime::ToMinutes(int64_t mfTime) {
        return std::chrono::duration_cast<std::chrono::minutes>(H::Chrono::Hns(mfTime)).count();
    }

    int64_t MFTime::ToHours(int64_t mfTime) {
        return std::chrono::duration_cast<std::chrono::hours>(H::Chrono::Hns(mfTime)).count();
    }

    int64_t MFTime::FromMsec(int64_t msec) {
        return std::chrono::duration_cast<H::Chrono::Hns::_MyBase>(std::chrono::milliseconds(msec)).count();
    }

    Platform::String^ MFTime::ToTimeString(int64_t mfTime) {
        auto s = ToMsec(mfTime) / 1000;
        auto m = s / 60;
        auto h = m / 60;
        auto timeStr = PadTimeValue(m) + ":" + PadTimeValue(s % 60);

        if (h > 0) {
            return PadTimeValue(h) + ":" + timeStr;
        }
        return timeStr;
    }

    Platform::String^ MFTime::PadTimeValue(int64_t timeVal) {
        auto timeStr = std::to_wstring(timeVal);
        if (timeStr.length() < 2) {
            timeStr = L"0" + timeStr;
        }
        return ref new Platform::String(timeStr.c_str());
    }
}