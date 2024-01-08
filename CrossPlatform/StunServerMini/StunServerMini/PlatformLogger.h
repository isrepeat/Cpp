#pragma once
#include "Platform.h"
#include <filesystem>

#if defined(__CMAKE_BUILD__)
#define LOG_DEBUG(msg, ...) printf(msg "\n")
#define LOG_ERROR(msg, ...) printf(msg "\n")
#define LOG_DEBUG_D(msg, ...) printf(msg "\n")
#define LOG_ERROR_D(msg, ...) printf(msg "\n")
#define LOG_FUNCTION_ENTER(msg, ...) printf(msg "\n")
#define LOG_FUNCTION_SCOPE(msg, ...) printf(msg "\n")
#else
#if defined(__WINDOWS_OS__)
	#include <MagicEnum/MagicEnum.h>
	#ifndef LOGGER_NUGET
	#error Logger nuget not defined
#endif
	#include <Logger/Logger.h>
#endif
#endif