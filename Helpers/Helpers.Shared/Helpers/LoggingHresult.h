#pragma once
#include "common.h"
#if SPDLOG_SUPPORT
#include <Spdlog/LogHelpers.h>
#else
#define DISABLE_ERROR_LOGGING
#endif


#if !defined(DISABLE_ERROR_LOGGING)
#include "Helpers.h"
#define LOG_FAILED(hr) \
	if (FAILED(hr)) { \
		LOG_ERROR_D(L"FAILED hr = [{:#10x}]: {}", static_cast<unsigned int>(hr), HELPERS_NS::GetFormatedErrorMessage(hr)); \
	}

#else
#define LOG_FAILED(hr)
#endif