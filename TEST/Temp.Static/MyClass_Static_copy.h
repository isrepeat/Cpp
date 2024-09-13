#pragma once
#include <string_view>

#ifndef HELPERS_NS
#define HELPERS_NS __Helpers_ns
#define HELPERS_NS_DEFINED_INTERNAL 1
#else
#if !HELPERS_NS_DEFINED_INTERNAL
#pragma message(PREPROCESSOR_MSG("HELPERS_NS already defined outside = '" PP_STRINGIFY(HELPERS_NS) "'"))
#endif
#endif

#ifndef HELPERS_INLINE_NS
#define HELPERS_INLINE_NS Static
#define HELPERS_INLINE_NS_DEFINED_INTERNAL 1
#else
#if !HELPERS_INLINE_NS_DEFINED_INTERNAL
#pragma message(PREPROCESSOR_MSG("HELPERS_INLINE_NS already defined outside = '" PP_STRINGIFY(HELPERS_INLINE_NS) "'"))
#endif
#endif


#ifndef HELPERS_NS_ALIAS
#define HELPERS_NS_ALIAS Helpers
#define HELPERS_NS_ALIAS_DEFINED_INTERNAL 1
#else
#if !HELPERS_NS_ALIAS_DEFINED_INTERNAL
#pragma message(PREPROCESSOR_MSG("HELPERS_NS_ALIAS already defined outside = '" PP_STRINGIFY(HELPERS_NS_ALIAS) "'"))
#endif
#endif

namespace HELPERS_NS { 
	inline namespace HELPERS_INLINE_NS {
		// Before set alias need create namespace.
	}
} 
namespace HELPERS_NS_ALIAS = HELPERS_NS::HELPERS_INLINE_NS;


namespace HELPERS_NS {
	inline namespace HELPERS_INLINE_NS {
		class MyClass {
		public:
			MyClass();
		};

		template<typename T>
		struct MyTemplate {
			static constexpr std::string_view templateNotes = "Static template [old]";
		};
	}
}