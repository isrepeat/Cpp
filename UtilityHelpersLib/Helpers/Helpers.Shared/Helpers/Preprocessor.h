#pragma once
/* ---------------------------------------- */
/*              BASE MACROS                 */
/* ---------------------------------------- */
#define PP_EXPAND(x) x

// In some contexts / cases the comma is not omitted, to fix it use __VA_EXPAND(__VA_ARGS__).
// NOTE: This problem not occured if project enable "/Zc:preprocessor" and use ", ##__VA_ARGS__" construction.
#define __VA_EXPAND(...) , ##__VA_ARGS__

// Expand __VA_ARGS__ with some first explicit arguments:
#define PP_EXPAND_1_VA_ARGS(arg1, ...) arg1, ##__VA_ARGS__
#define PP_EXPAND_2_VA_ARGS(arg1, arg2, ...) arg1, arg2, ##__VA_ARGS__

#define PP_STRINGIFY_BASE(x) #x
#define PP_STRINGIFY(x) PP_STRINGIFY_BASE(x)

#define PP_CONCAT_BASE(a, b) a ## b
#define PP_CONCAT(a, b) PP_CONCAT_BASE(a, b)


/* ---------------------------------------- */
/*                MESSAGING                 */
/* ---------------------------------------- */
#if !defined(DISABLE_PREPROCESSOR_MESSAGES)
#ifdef __PROJECT_NAME__
#	define PREPROCESSOR_MSG(msg)        "  PREPROCESSOR<" PP_STRINGIFY(__PROJECT_NAME__) ">: \"" msg "\""
#	define PREPROCESSOR_ERROR_MSG(msg)  "  PREPROCESSOR<" PP_STRINGIFY(__PROJECT_NAME__) "> [Error]:\n  \"" msg "\""
#else
#	define PREPROCESSOR_MSG(msg)        "  PREPROCESSOR: \"" msg "\""
#	define PREPROCESSOR_ERROR_MSG(msg)  "  PREPROCESSOR [Error]:\n  \"" msg "\""
#endif
#else
#define PREPROCESSOR_MSG(msg) ""
#endif


#define DISABLE_PREPROCESSOR_FILE_INCLUDED
#if !defined(DISABLE_PREPROCESSOR_FILE_INCLUDED)
#define PREPROCESSOR_FILE_INCLUDED(file) PREPROCESSOR_MSG("Included '" file "'")
#else
#define PREPROCESSOR_FILE_INCLUDED(file) ""
#endif


#define PP_MSG_ERROR_REQUIRE_CPP_CONCEPTS  PREPROCESSOR_ERROR_MSG("This file require 'cpp concepts' feature (c++20)")


//
// Define global macros here
//
#ifndef _DEBUG
// UWP projects do not define 'NDEBUG' macro so abort() can be called when use assert(...).
// It is preferable to define this macro at top level, but as workaround it's norm for now
#ifndef NDEBUG
#define NDEBUG
#endif
#endif


/* ---------------------------------------- */
/*               PP_NUM_ARG                 */
/* ---------------------------------------- */
#define PP_NUM_ARG_BASE( \
      _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
     _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
     _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
     _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
     _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
     _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
     _61,_62,_63,N,...) N

#define PP_NUM_ARG_SEQ \
     63,62,61,60,                   \
     59,58,57,56,55,54,53,52,51,50, \
     49,48,47,46,45,44,43,42,41,40, \
     39,38,37,36,35,34,33,32,31,30, \
     29,28,27,26,25,24,23,22,21,20, \
     19,18,17,16,15,14,13,12,11,10, \
     9,8,7,6,5,4,3,2,1,0


#define PP_NUM_ARG_PROXY(...) PP_EXPAND(PP_NUM_ARG_BASE(__VA_ARGS__))
#define PP_NUM_ARG(...)  PP_NUM_ARG_PROXY(__VA_ARGS__, PP_NUM_ARG_SEQ)


/* ---------------------------------------- */
/*               PP_REVERSE                 */
/* ---------------------------------------- */
#define __PP_REVERSE_1(a) a
#define __PP_REVERSE_2(a,b) b,a
#define __PP_REVERSE_3(a,...) PP_EXPAND(__PP_REVERSE_2(__VA_ARGS__)),a
#define __PP_REVERSE_4(a,...) PP_EXPAND(__PP_REVERSE_3(__VA_ARGS__)),a
#define __PP_REVERSE_5(a,...) PP_EXPAND(__PP_REVERSE_4(__VA_ARGS__)),a
#define __PP_REVERSE_6(a,...) PP_EXPAND(__PP_REVERSE_5(__VA_ARGS__)),a
#define __PP_REVERSE_7(a,...) PP_EXPAND(__PP_REVERSE_6(__VA_ARGS__)),a
#define __PP_REVERSE_8(a,...) PP_EXPAND(__PP_REVERSE_7(__VA_ARGS__)),a
#define __PP_REVERSE_9(a,...) PP_EXPAND(__PP_REVERSE_8(__VA_ARGS__)),a
#define __PP_REVERSE_10(a,...) PP_EXPAND(__PP_REVERSE_9(__VA_ARGS__)),a

#define PP_REVERSE_BASE(N, ...) PP_EXPAND(__PP_REVERSE_ ## N(__VA_ARGS__))
#define PP_REVERSE_PROXY(N, ...) PP_REVERSE_BASE(N, ##__VA_ARGS__)
#define PP_REVERSE(...) PP_REVERSE_PROXY(PP_NUM_ARG(__VA_ARGS__), ##__VA_ARGS__)


/* ---------------------------------------- */
/*             PP_VARIADIC_FUNC             */
/* ---------------------------------------- */
#define PP_VARIADIC_FUNC(Func, ...) PP_CONCAT(Func, PP_NUM_ARG(__VA_ARGS__)) (__VA_ARGS__)


/* ---------------------------------------- */
/*         PP_GET_XXX / PP_DROP_XXX         */
/* ---------------------------------------- */
#define PP_GET_ELEMENT_N_BASE(_1, _2, _3, _4, _5, N, ...) N
#define PP_GET_ELEMENT_N(...) PP_EXPAND(PP_GET_ELEMENT_N_BASE(__VA_ARGS__))


#define PP_GET_FIRST_BASE(a1, ...) a1
#define PP_GET_FIRST_PROXY(...) PP_EXPAND(PP_GET_FIRST_BASE(__VA_ARGS__))
#define PP_GET_FIRST(...) PP_GET_FIRST_PROXY(__VA_ARGS__)

#define PP_GET_SECOND_BASE(a1, a2, ...) a2
#define PP_GET_SECOND_PROXY(...) PP_EXPAND(PP_GET_SECOND_BASE(__VA_ARGS__))
#define PP_GET_SECOND(...) PP_GET_SECOND_PROXY(__VA_ARGS__)

#define PP_GET_LAST(...) PP_GET_FIRST(PP_REVERSE(__VA_ARGS__))


#define PP_DROP_FIRST_BASE(a1, ...) __VA_ARGS__
#define PP_DROP_FIRST_PROXY(...) PP_EXPAND(PP_DROP_FIRST_BASE(__VA_ARGS__))
#define PP_DROP_FIRST(...) PP_DROP_FIRST_PROXY(__VA_ARGS__)

#define PP_DROP_LAST(...) PP_REVERSE(PP_DROP_FIRST(PP_REVERSE(__VA_ARGS__)))

/* ---------------------------------------- */
/*          IF_VA_OPT_SUPPORTED             */
/* ---------------------------------------- */
// WARNING: Some macros which uses __VA_OPT()__ require enabled "/Zc:preprocessor", 
//          you can enable it through project property or define "UseStandardPreprocessor == true"
//          in Directory.Build.targets:
// 
//           <ItemDefinitionGroup>
//             <ClCompile>
//               <UseStandardPreprocessor>true</UseStandardPreprocessor>
//           ...
//
#define IF_VA_OPT_SUPPORTED_BASE(...) PP_GET_SECOND(__VA_OPT__(,) 1, 0)
#define IF_VA_OPT_SUPPORTED IF_VA_OPT_SUPPORTED_BASE(_)


/* ---------------------------------------- */
/*               PP_FOR_EACH                */
/* ---------------------------------------- */
// TODO: Rename some macros more readable.

// Source: https://gist.github.com/thwarted/8ce47e1897a578f4e80a
/* because gcc cpp doesn't recursively expand macros, so a single CALLIT
 * macro can't be used in all the FE_n macros below
 */
#define FE_CALLITn01(MacroFn, parenthesis) MacroFn parenthesis
#define FE_CALLITn02(MacroFn, parenthesis) MacroFn parenthesis
#define FE_CALLITn03(MacroFn, parenthesis) MacroFn parenthesis
#define FE_CALLITn04(MacroFn, parenthesis) MacroFn parenthesis
#define FE_CALLITn05(MacroFn, parenthesis) MacroFn parenthesis
#define FE_CALLITn06(MacroFn, parenthesis) MacroFn parenthesis
#define FE_CALLITn07(MacroFn, parenthesis) MacroFn parenthesis
#define FE_CALLITn08(MacroFn, parenthesis) MacroFn parenthesis
#define FE_CALLITn09(MacroFn, parenthesis) MacroFn parenthesis


 /* the MSVC preprocessor expands __VA_ARGS__ as a single argument, so it needs
  * to be expanded indirectly through the CALLIT macros.
  * http://connect.microsoft.com/VisualStudio/feedback/details/380090/variadic-macro-replacement
  * http://stackoverflow.com/questions/21869917/visual-studio-va-args-issue
  */
#define FE_n00(...)
#define FE_n01(MacroFn, arg1, ...)  MacroFn(arg1)
#define FE_n02(MacroFn, arg1, ...)  MacroFn(arg1) FE_CALLITn02(FE_n01, (MacroFn, ##__VA_ARGS__))
#define FE_n03(MacroFn, arg1, ...)  MacroFn(arg1) FE_CALLITn03(FE_n02, (MacroFn, ##__VA_ARGS__))
#define FE_n04(MacroFn, arg1, ...)  MacroFn(arg1) FE_CALLITn04(FE_n03, (MacroFn, ##__VA_ARGS__))
#define FE_n05(MacroFn, arg1, ...)  MacroFn(arg1) FE_CALLITn05(FE_n04, (MacroFn, ##__VA_ARGS__))
#define FE_n06(MacroFn, arg1, ...)  MacroFn(arg1) FE_CALLITn06(FE_n05, (MacroFn, ##__VA_ARGS__))
#define FE_n07(MacroFn, arg1, ...)  MacroFn(arg1) FE_CALLITn07(FE_n06, (MacroFn, ##__VA_ARGS__))
#define FE_n08(MacroFn, arg1, ...)  MacroFn(arg1) FE_CALLITn08(FE_n07, (MacroFn, ##__VA_ARGS__))
#define FE_n09(MacroFn, arg1, ...)  MacroFn(arg1) FE_CALLITn09(FE_n08, (MacroFn, ##__VA_ARGS__))
#define FE_n10(...) ERROR: FOR_EACH only supports up to 9 arguments

#define FE_GET_MACRO_BASE(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, NAME, ...) NAME
#define FE_GET_MACRO(...) PP_EXPAND(FE_GET_MACRO_BASE(__VA_ARGS__))

#define PP_FOR_EACH(MacroFn, ...) \
    FE_CALLITn01( \
		FE_GET_MACRO( \
				 PP_EXPAND_1_VA_ARGS(_0, __VA_ARGS__), \
                 FE_n10, FE_n09, FE_n08, FE_n07, FE_n06, FE_n05, FE_n04,\
                 FE_n03, FE_n02, FE_n01, FE_n00), (PP_EXPAND_1_VA_ARGS(MacroFn, __VA_ARGS__) \
		) \
	)


/* ---------------------------------------- */
/*   PP_INLINE_TEMPLATE_SPECIALIZATION      */
/* ---------------------------------------- */
#define __PP_CLOSE_NAMESPACE(value) }
#define __PP_OPEN_NAMESPACE(value) namespace value {

#define PP_INLINE_TEMPLATE_SPECIALIZATION_INTERNAL(code, ...) \
    PP_FOR_EACH(__PP_CLOSE_NAMESPACE, ##__VA_ARGS__) \
    code \
    PP_FOR_EACH(__PP_OPEN_NAMESPACE, ##__VA_ARGS__) 

#define PP_INLINE_TEMPLATE_SPECIALIZATION(...) \
    PP_INLINE_TEMPLATE_SPECIALIZATION_INTERNAL(PP_GET_LAST(__VA_ARGS__), PP_DROP_LAST(__VA_ARGS__))

// Usage:
// Redefine __NAMESPACE__ macro according to current namespace before call _BEGIN().
//
// For example: 
// namespace A {
//   namespace B::C {
//      ...
//      #define __NAMESPACE__ A, B::C
//      PP_INLINE_TEMPLATE_SPECIALIZATION_BEGIN()
//      ...
//      PP_INLINE_TEMPLATE_SPECIALIZATION_END()
#define PP_INLINE_TEMPLATE_SPECIALIZATION_BEGIN() \
    PP_FOR_EACH(__PP_CLOSE_NAMESPACE, __NAMESPACE__)

#define PP_INLINE_TEMPLATE_SPECIALIZATION_END() \
    PP_FOR_EACH(__PP_OPEN_NAMESPACE, __NAMESPACE__)



#define __PP_JOIN_NAMESPACE(value) ::value

// Helper to get full namespace from __NAMESPACE__'s macro args
#define PP_LAST_NAMESPACE \
	PP_GET_FIRST(__NAMESPACE__) PP_FOR_EACH(__PP_JOIN_NAMESPACE, PP_DROP_FIRST(__NAMESPACE__))


/* ---------------------------------------- */
/*               Class helpers              */
/* ---------------------------------------- */
// This is an alternative to "using _MyBase::_MyBase"
#define PP_FORWARD_CTOR(From, To) \
	template <typename... _Args> \
	From(_Args&&... args) \
		: To(::std::forward<_Args&&>(args)...) \
	{}