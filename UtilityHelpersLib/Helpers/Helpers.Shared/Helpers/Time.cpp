#include "Time.h"
#include <sstream>
#include <Windows.h>

namespace HELPERS_NS {
	MeasureTime::MeasureTime()
		: start(std::chrono::high_resolution_clock::now())
	{
	}
	MeasureTime::~MeasureTime() {
		auto stop = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> duration = stop - start;
		OutputDebugStringA((std::stringstream() << "[elapsed time = " << duration.count() * 1000 << "ms] \n").str().c_str());
	}

	MeasureTimeScoped::MeasureTimeScoped(std::function<void(std::chrono::duration<double, std::milli> dt)> completedCallback)
		: start{ std::chrono::high_resolution_clock::now() }
		, completedCallback{ completedCallback }
	{
	}
	MeasureTimeScoped::~MeasureTimeScoped() {
		auto stop = std::chrono::high_resolution_clock::now();
		if (completedCallback) {
			std::chrono::duration<double, std::milli> duration = stop - start;
			completedCallback(duration);
		}
	}

	std::string GetTimeNow(TimeFormat timeFormat) {
		std::string format;
		switch (timeFormat) {
		case TimeFormat::None:
			 format = "%Y%m%d%H%M%S";
			 break;
		case TimeFormat::Ymdhms_with_separators:
			format = "%Y-%m-%dT%H:%M:%SZ";
			break;
		};

		struct tm gt;
		char output[100];
		auto time = std::time(nullptr);
		errno_t err = ::gmtime_s(&gt, &time);
		if (!err) {
			if (std::strftime(output, sizeof(output), format.c_str(), &gt)) {
				return std::string{ output };
			}
		}
		return "";
	}

	std::string GetTimezone() {
		// Get the local system time.
		SYSTEMTIME LocalTime = { 0 };
		GetSystemTime(&LocalTime);

		// Get the timezone info.
		TIME_ZONE_INFORMATION TimeZoneInfo;
		GetTimeZoneInformation(&TimeZoneInfo);

		// Convert local time to UTC.
		SYSTEMTIME GmtTime = { 0 };
		TzSpecificLocalTimeToSystemTime(&TimeZoneInfo, &LocalTime, &GmtTime);

		// GMT = LocalTime + TimeZoneInfo.Bias
		// TimeZoneInfo.Bias is the difference between local time and GMT in minutes.

		// Local time expressed in terms of GMT bias.
		int TimeZoneDifference = -(float(TimeZoneInfo.Bias) / 60);
		return std::to_string(TimeZoneDifference);
	}
}