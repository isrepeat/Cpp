#pragma once
#include "Helpers/common.h"
#include "Helpers/Meta/Concepts.h"

#include <algorithm>
#include <iterator>
#include <ranges>

namespace STD_EXT_NS {
	namespace ranges {
		namespace views {
			//
			// ░ to
			// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
			//
			// ░ to_closure
			//
			template <typename TContainer>
			struct to_closure {
				template <typename TRange>
				__requires_expr(
					::std::ranges::input_range<TRange>
				) friend auto operator|(
					TRange&& range,
					to_closure /*self*/
					) {
					TContainer container;

					if constexpr (requires { container.reserve(::std::ranges::size(range)); }) {
						container.reserve(::std::ranges::size(range));
					}

					// универсально для map/set и т.п.
					// inserter чуть медленнее для vector, зато компилируется везде
					::std::ranges::copy(range, ::std::inserter(container, container.end()));

					return container;
				}
			};


			//
			// ░ to_fn
			//
			template <typename TContainer>
			struct to_fn {
			public:
				constexpr auto operator()() const noexcept {
					return to_closure<TContainer>{};
				}
			};


			template <typename TContainer>
			inline constexpr to_fn<TContainer> to{};
		}
	}

	namespace views = ranges::views;
}