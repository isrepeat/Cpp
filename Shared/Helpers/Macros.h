#pragma once
#include <debugapi.h>

#ifdef _DEBUG
#define LOG_FIELD_DESTRUCTION(ClassName)																\
    struct ClassName##Field {																			\
        ~ClassName##Field() {																			\
            OutputDebugStringA("===== ~" #ClassName "Field() destroyed ===== \n");						\
        }																								\
    } ClassName##FieldInstance;
#else
#define LOG_FIELD_DESTRUCTION(ClassName)
#endif


#define NO_COPY(className) \
	className(const className &) = delete; \
	className &operator=(const className &) = delete; \

#define NO_MOVE(className) \
    className(className &&) = delete; \
	className &operator=(className &&) = delete;

#define NO_COPY_MOVE(className) \
    NO_COPY(className) \
    className(className &&) = delete; \
	className &operator=(className &&) = delete;

// Expand __VA_ARGS__ with some first explicit arguments:
#define EXPAND_1_VA_ARGS_(arg1, ...) arg1, __VA_ARGS__
#define EXPAND_2_VA_ARGS_(arg1, arg2, ...) arg1, arg2, __VA_ARGS__