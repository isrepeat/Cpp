#pragma once
#include "Helpers/common.h"

namespace HELPERS_NS {
	namespace meta {
		//
		// ░ Conditional "IF"
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		namespace v1 {
			template <bool If, typename T, T ThenValue, T ElseValue>
			struct IF_value {
				static constexpr T value = ThenValue;
			};

			template <typename T, T ThenValue, T ElseValue>
			struct IF_value<false, T, ThenValue, ElseValue> {
				static constexpr T value = ElseValue;
			};
		} // namespace v1

		namespace vLatest {
			template <bool Cond, auto VTrue, auto VFalse>
			struct IF_value_c {
				static constexpr auto value = VTrue;
			};

			template <auto VTrue, auto VFalse>
			struct IF_value_c<false, VTrue, VFalse> {
				static constexpr auto value = VFalse;
			};
		} // namespace vLatest

		// Use latest version by default
		inline namespace vLatest {};
	}
}