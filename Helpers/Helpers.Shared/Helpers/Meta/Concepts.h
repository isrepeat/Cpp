#pragma once
#include "Helpers/common.h"
#include <concepts>

#if __cpp_concepts

// Пока Microsoft не пофиксит дефолтный стиль (отступы) написания requires - используй следуюий стиль:
// __requires_expr(
//   Condition1 &&
//   Condition2 &&
//   ...
//   ConditionN
// ) 
// 
#define __requires_expr(...) requires (__VA_ARGS__)

namespace HELPERS_NS {
	namespace meta {
		namespace concepts {
			template<typename T>
			concept has_type = requires {
				typename T;
			};


			template<typename T>
			concept is_dereferenceable = requires(const T & val) {
				{ *val };
			};


			// Iter — это iterator или const_iterator контейнера
			template<class Iter, class Container>
			concept iterator_of =
				std::same_as<Iter, typename Container::iterator> ||
				std::same_as<Iter, typename Container::const_iterator>;


			// У контейнера есть erase(first,last) и он возвращает iterator
			template <typename TContainer>
			concept is_erasable_container = requires (
				TContainer c,
				typename TContainer::iterator f,
				typename TContainer::iterator l
				) {
					{ c.erase(f, l) } -> std::same_as<typename TContainer::iterator>;
			};
		}
	}
}
#else
#define __requires_expr(...)
#endif