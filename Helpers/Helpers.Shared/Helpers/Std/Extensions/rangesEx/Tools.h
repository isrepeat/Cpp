#pragma once
#include "Helpers/common.h"
#include "Helpers/Meta/Concepts.h"

#include <algorithm>
#include <utility>
#include <ranges>

namespace STD_EXT_NS {
	namespace ranges {
		namespace tools {
			//
			// ░ view_of
			// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
			// Типовой алиас: вычисляет тип view для произвольного viewable_range TRange.
			//
			template <class TRange>
			using view_of_t = decltype(::std::ranges::views::all(::std::declval<TRange>()));
		}

		namespace views {
			namespace tools {
				//
				// ░ as_view
				// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
				// Функция-обёртка: превращает любой viewable_range в view.
				// Поведение 1:1 с std::ranges::views::all, просто более выразительное имя.
				// 
				// std::ranges::all(...) превращает "что угодно viewable" во view:
				//   - если уже view — вернёт его;
				//   - если lvalue-контейнер — оборачивает в ref_view;
				//   - если rvalue-viewable — материализует совместимое view.
				//
				template <class TRange>
				constexpr auto as_view(TRange&& r) -> decltype(::std::ranges::views::all(::std::forward<TRange>(r))) {
					return ::std::ranges::views::all(::std::forward<TRange>(r));
				}
			}
		}
	}
}