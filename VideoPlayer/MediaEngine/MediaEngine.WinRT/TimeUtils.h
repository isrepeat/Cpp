#pragma once

#include <cstddef>

namespace MediaEngineWinRT {
	public ref class MFTime sealed {
    private:
        static Platform::String^ PadTimeValue(int64_t timeVal);

	public:
        /* Converts MFTIME (100s of nanosecs) to millisecs, seconds, minutes and hours */
        static int64_t ToMsec(int64_t mfTime);
        static int64_t ToSeconds(int64_t mfTime);
        static int64_t ToMinutes(int64_t mfTime);
        static int64_t ToHours(int64_t mfTime);

        /* Converts milliseconds to MFTIME (100s of nanosecs) */
        static int64_t FromMsec(int64_t msec);

        /* Converts MFTIME (100s of nanosecs) to a human-readable representation in format:
         * `mm:ss` or `hh:mm:ss` */
        static Platform::String^ ToTimeString(int64_t mfTime);
	};
}