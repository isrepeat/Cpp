#pragma once
#include "Helpers/common.h"
#include <type_traits>

namespace HELPERS_NS {
	namespace meta {
		// Всегда false, но зависит от шаблонных параметров
		// Удобно для static_assert в зависимом контексте
		template <typename...>
		struct dependent_false : std::false_type {};

		template <typename... Ts>
		inline constexpr bool dependent_false_v = dependent_false<Ts...>::value;
	}
}