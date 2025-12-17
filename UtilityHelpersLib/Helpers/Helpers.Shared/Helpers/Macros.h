#pragma once
#include "common.h"
#include <debugapi.h>
#include <cassert>

#define H_CONCAT_BASE(a, b) a ## b
#define H_CONCAT(a, b) H_CONCAT_BASE(a,b)

#ifdef _DEBUG
#define NOOP int H_CONCAT(noop__,__LINE__) = 1234

#define Dbreak              \
if (IsDebuggerPresent()) {  \
    __debugbreak();         \
}

#define BEEP(ton, duration) Beep(ton, duration > 500 ? duration : 500) // 500ms - min beep duration

#define assertm(expression, message) (void)(												                          \
            (!!(expression)) ||                                                                                       \
            (_wassert(L" " message L"  {" _CRT_WIDE(#expression) L"}", _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \
        )
#else
#define NOOP
#define Dbreak
#define BEEP(ton, ms) (void)(0)
#define assertm(expression, message) ((void)0)
#endif


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

/* ---------------- */
/*  has_member.hpp  */
/* ---------------- */
// A compile-time method for checking the existence of a class member
// @see https://general-purpose.io/2017/03/10/checking-the-existence-of-a-cpp-class-member-at-compile-time/

// This code uses "decltype" which, according to http://en.cppreference.com/w/cpp/compiler_support
// should be supported by Clang 2.9+, GCC 4.3+ and MSVC 2010+ (if you have an older compiler, please upgrade :)
// As of "constexpr", if not supported by your compiler, you could try "const"
// or use the value as an inner enum value e.g. enum { value = ... }

/// Defines a "has_member_member_name" class template
///
/// This template can be used to check if its "T" argument
/// has a data or function member called "member_name"
#define define_has_member(member_name)                                         \
    template <typename T>                                                      \
    class has_member_##member_name                                             \
    {                                                                          \
        typedef char yes_type;                                                 \
        typedef long no_type;                                                  \
        template <typename U> static yes_type test(decltype(&U::member_name)); \
        template <typename U> static no_type  test(...);                       \
    public:                                                                    \
        static constexpr bool value = sizeof(test<T>(0)) == sizeof(yes_type);  \
    }

/// Shorthand for testing if "class_" has a member called "member_name"
///
/// @note "define_has_member(member_name)" must be used
///       before calling "has_member(class_, member_name)"
#define has_member(class_, member_name)  has_member_##member_name<class_>::value