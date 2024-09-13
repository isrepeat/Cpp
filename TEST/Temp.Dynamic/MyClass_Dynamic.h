#pragma once
#ifdef __MAKE_DYNAMIC_LIBRARY__
#define CLASS_API __declspec(dllexport)
#define DLL_API __declspec(dllexport)
#else
#define CLASS_API __declspec(dllimport)
#define DLL_API __declspec(dllimport)
#endif


#ifndef NUGET_NS
#define NUGET_NS __Nuget_ns
#define NUGET_NS_DEFINED_INTERNAL 1
#else
#if !NUGET_NS_DEFINED_INTERNAL
#pragma message(PREPROCESSOR_MSG("NUGET_NS already defined outside = '" PP_STRINGIFY(NUGET_NS) "'"))
#endif
#endif

#ifndef NUGET_INLINE_NS
#define NUGET_INLINE_NS Dynamic
#define NUGET_INLINE_NS_DEFINED_INTERNAL 1
#else
#if !NUGET_INLINE_NS_DEFINED_INTERNAL
#pragma message(PREPROCESSOR_MSG("NUGET_INLINE_NS already defined outside = '" PP_STRINGIFY(NUGET_INLINE_NS) "'"))
#endif
#endif


#ifndef NUGET_NS_ALIAS
#define NUGET_NS_ALIAS Nuget
#define NUGET_NS_ALIAS_DEFINED_INTERNAL 1
#else
#if !NUGET_NS_ALIAS_DEFINED_INTERNAL
#pragma message(PREPROCESSOR_MSG("NUGET_NS_ALIAS already defined outside = '" PP_STRINGIFY(NUGET_NS_ALIAS) "'"))
#endif
#endif

namespace NUGET_NS {
	inline namespace NUGET_INLINE_NS {
		// Before set alias need create namespace.
	}
}
namespace NUGET_NS_ALIAS = NUGET_NS::NUGET_INLINE_NS;

//#define HELPERS_INLINE_NS Dynamic
//#include "../Temp.Static/MyClass_Static_copy.h"
//#undef HELPERS_INLINE_NS

// forward declaration with changed ns.
namespace __Helpers_ns {
	inline namespace Static {
		template<typename T>
		struct MyTemplate;
	}
}


// #include "MyClass_Static.h" from nuget.

//#include <string_view>
//namespace __Helpers_ns {
//	inline namespace Dynamic {
//		template<typename T>
//		struct MyTemplate {
//			static constexpr std::string_view templateNotes = "Static template";
//		};
//	}
//}
//namespace NugetHelpers = __NUGET_NS::Nuget;


#include <string_view>
namespace NUGET_NS {
	inline namespace NUGET_INLINE_NS {
		class DLL_API MyClass {
		public:
			//using _Data = __Helpers_ns::Static::MyTemplate<int>;
			MyClass();

			//_Data GetData();

		public:
			__Helpers_ns::Static::MyTemplate<int>* data;
		};

		template<typename T>
		struct MyTemplate {
			//using _Data = __Helpers_ns::Dynamic::MyTemplate<int>;
			static constexpr std::string_view templateNotes = "Dynamic template";
		};

		void DLL_API SomeFunction();
	}
}