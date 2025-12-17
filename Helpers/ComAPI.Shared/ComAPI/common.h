#pragma once
#include <winapifamily.h>

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define COMPILE_FOR_DESKTOP 1
#endif

#if (_MANAGED == 1) || (_M_CEE == 1)
// COMPILE_FOR_DESKTOP also == 1
#define COMPILE_FOR_CLR 1
#else
#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define COMPILE_FOR_WINRT 1
#endif
#endif