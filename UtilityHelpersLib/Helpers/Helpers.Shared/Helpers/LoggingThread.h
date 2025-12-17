#pragma once
#include "common.h"
#if SPDLOG_SUPPORT
#include <Spdlog/LogHelpers.h>
#include "Thread.h"
#else
#define DISABLE_THREAD_LOGGING
#endif


#if !defined(DISABLE_THREAD_LOGGING)
#define LOG_THREAD(name) \
	LOG_DEBUG_D(L"Thread START '{}'", std::wstring(name)); \
	HELPERS_NS::ThreadNameHelper::SetThreadName(name); \
	\
	auto threadFinishLogScoped = HELPERS_NS::MakeScope([&] { \
		LOG_DEBUG_D(L"Thread END '{}'", std::wstring(name)); \
		});
#else
#define LOG_THREAD(name)
#endif