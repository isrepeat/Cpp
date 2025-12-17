#pragma once
#include "Helpers/common.h"
#include "Helpers/Meta/Concepts.h"

#include <algorithm>
#include <ranges>

namespace STD_EXT_NS {
	namespace ranges {
		namespace views {
			//
			// ░ drop_last
			// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
			//
			// ░ drop_last_view
			//
			template <typename TView>
			__requires_expr(
				::std::ranges::forward_range<TView>&&
				::std::ranges::view<TView>
			) class drop_last_view : public ::std::ranges::view_interface<drop_last_view<TView>> {
			public:
				drop_last_view() = default;

				constexpr drop_last_view(
					TView baseView,
					::std::size_t countToDrop
				)
					: baseView{ ::std::move(baseView) }
					, countToDrop{ countToDrop }
					, takeCountCached{ 0 } {
					const auto total = ::std::ranges::distance(this->baseView);
					const auto safeDrop = static_cast<::std::size_t>(::std::min<::std::ptrdiff_t>(
						total,
						static_cast<::std::ptrdiff_t>(this->countToDrop)
					));
					this->takeCountCached = static_cast<::std::size_t>(::std::max<::std::ptrdiff_t>(
						total - static_cast<::std::ptrdiff_t>(safeDrop),
						0
					));;
				}

				// при желании можно сделать и const-версии
				constexpr auto begin() {
					return ::std::ranges::begin(
						::std::ranges::views::take(this->baseView, this->takeCountCached)
					);
				}

				constexpr auto end() {
					return ::std::ranges::end(
						::std::ranges::views::take(this->baseView, this->takeCountCached)
					);
				}

				// бонус: size(), если базовый — sized_range
				constexpr auto size() requires ::std::ranges::sized_range<TView> {
					return this->takeCountCached;
				}

			private:
				TView baseView;
				::std::size_t countToDrop{ 0 };
				::std::size_t takeCountCached{ 0 };
			};


			//
			// ░ drop_last_closure
			//
			struct drop_last_closure {
			public:
				::std::size_t countToDrop{ 0 };

			public:
				template <typename TRange>
				__requires_expr(
					::std::ranges::forward_range<TRange>
				) friend auto operator|(
					TRange&& range,
					const drop_last_closure& self
					) {
					using View_t = ranges::tools::view_of_t<TRange>;
					return drop_last_view<View_t>{
						views::tools::as_view(::std::forward<TRange>(range)),
							self.countToDrop
					};
				}
			};

			//
			// ░ drop_last_fn
			//
			struct drop_last_fn {
				constexpr auto operator()(
					::std::size_t n
					) const {
					return drop_last_closure{ .countToDrop = n };
				}

				template <typename TRange>
				__requires_expr(
					::std::ranges::forward_range<TRange>
				) constexpr auto operator()(
					TRange&& range,
					::std::size_t n
					) const {
					using View_t = ranges::tools::view_of_t<TRange>;
					return drop_last_view<View_t>{
						views::tools::as_view(::std::forward<TRange>(range)),
							n
					};
				}
			};


			inline constexpr drop_last_fn drop_last{};
		}
	}

	namespace views = ranges::views;
}