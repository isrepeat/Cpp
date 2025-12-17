#pragma once
#include "..\config.h"

#if HAVE_WINRT
#define LoggerDisabled 1
#endif

#if !LoggerDisabled
#include <Logger/Logger.h>
#endif
