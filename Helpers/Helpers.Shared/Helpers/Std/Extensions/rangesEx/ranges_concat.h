#pragma once
#include "Helpers/common.h"
#include "Helpers/Meta/Concepts.h"
#include "Tools.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <optional>
#include <utility>
#include <ranges>

namespace STD_EXT_NS {
	namespace ranges {
		namespace views {
			//
			// ░ concat
			// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
			//
			// Идея:
			//   Лениво склеиваем ДВА произвольных viewable_range так, чтобы можно было писать:
			//     rng1 | std::ex::ranges::views::concat(rng2)
			//   Адаптер ничего не копирует из элементов, просто "прочитав" первый диапазон,
			//   продолжает итерацию вторым. Жизненный цикл обоих исходных представлений (views)
			//   хранится внутри concat_view, поэтому итераторы безопасны.
			//
			// Ограничения (requires):
			//   - Оба аргумента — input_range и уже "представления" (view), чтобы их можно было хранить по значению.
			//   - Ссылочные типы элементов совместимы через common_reference_with: у concat_view должен быть единый
			//     type reference, который подходит и для TView1::reference, и для TView2::reference.
			//
			// Замечания по реализации:
			//   - Категория итератора: input_iterator (достаточно для большинства наших сценариев).
			//     При желании можно нарастить поддержку forward_range и т.п.
			//   - Конец диапазона — default_sentinel. Итератор == end тогда и только тогда, когда
			//     мы уже перешли ко второй части И дошли до её конца.
			//   - Если первая часть пустая, begin() сразу начинает со второй.
			//
			// ░ concat_view
			//
			template <
				typename TView1,
				typename TView2
			>
			__requires_expr(
				::std::ranges::input_range<TView1>&&
				::std::ranges::input_range<TView2>&&
				::std::ranges::view<TView1>&&
				::std::ranges::view<TView2>&&
				::std::common_reference_with<
				::std::ranges::range_reference_t<TView1>,
				::std::ranges::range_reference_t<TView2>
				>
			) class concat_view : public ::std::ranges::view_interface<concat_view<TView1, TView2>> {
			public:
				// Держит ссылку на родителя (для доступа к viewFirst/viewSecond) и состояние:
				//   - inFirst          : мы сейчас в первой части?
				//   - iterFirstOpt     : активный итератор первой части (если inFirst == true)
				//   - iterSecondOpt    : активный итератор второй части (если inFirst == false)
				//
				// Ленивая "стыковка":
				//   - ++ на первой части двигает iterFirst; при достижении end(first) — переключаемся
				//     на начало второй части (inFirst = false, iterSecondOpt = begin(second)).
				//   - ++ на второй части просто двигает iterSecond.
				//
				class iterator {
				public:
					// Соглашения итераторов для STL
					using iterator_concept = ::std::input_iterator_tag;
					using iterator_category = ::std::input_iterator_tag;
					using difference_type = ::std::ptrdiff_t;
					using reference = ::std::common_reference_t<
						::std::ranges::range_reference_t<TView1>,
						::std::ranges::range_reference_t<TView2>
					>;
					using value_type = ::std::common_type_t<
						::std::ranges::range_value_t<TView1>,
						::std::ranges::range_value_t<TView2>
					>;

					using It1_t = ::std::ranges::iterator_t<TView1>;
					using It2_t = ::std::ranges::iterator_t<TView2>;

					iterator() = default;

					explicit iterator(
						concat_view* parentPtr,
						bool isInFirst,
						::std::optional<It1_t> iterFirstOptInit,
						::std::optional<It2_t> iterSecondOptInit
					)
						: parent{ parentPtr }
						, inFirst{ isInFirst }
						, iterFirstOpt{ ::std::move(iterFirstOptInit) }
						, iterSecondOpt{ ::std::move(iterSecondOptInit) } {
					}

					reference operator*() const {
						// Разыменование транслируется либо к итератору первой части, либо ко второй.
						if (this->inFirst) {
							return **this->iterFirstOpt;
						}
						else {
							return **this->iterSecondOpt;
						}
					}

					iterator& operator++() {
						// Шаг итератора:
						//   - если мы в первой части — пытаемся сдвинуться;
						//     при достижении конца первой части переключаемся на начало второй.
						//   - если во второй — сдвигаем её итератор.
						if (this->inFirst) {
							auto& it1 = *this->iterFirstOpt;
							++it1;

							if (it1 == ::std::ranges::end(this->parent->viewFirst)) {
								this->inFirst = false;
								this->iterSecondOpt = ::std::ranges::begin(this->parent->viewSecond);
							}
						}
						else {
							auto& it2 = *this->iterSecondOpt;
							++it2;
						}

						return *this;
					}

					iterator operator++(int) {
						auto iteratorCopy = *this;
						++(*this);
						return iteratorCopy;
					}

					// Методная форма сравнения: it == default_sentinel
					bool operator==(::std::default_sentinel_t) const {
						// Конец достигается ТОЛЬКО когда мы уже во второй части и дошли до её end().
						if (this->inFirst) {
							return false;
						}

						return *(this->iterSecondOpt) == ::std::ranges::end(this->parent->viewSecond);
					}

					// Симметричная свободная форма: default_sentinel == it
					friend bool operator==(::std::default_sentinel_t s, const iterator& it) {
						return it == s;
					}

				private:
					concat_view* parent;
					bool inFirst;
					::std::optional<It1_t> iterFirstOpt;
					::std::optional<It2_t> iterSecondOpt;
				}; // class iterator

				friend class iterator; // доступ к приватным полям viewFirst / viewSecond.


				concat_view() = default;

				constexpr concat_view(
					TView1 v1,
					TView2 v2
				)
					: viewFirst{ ::std::move(v1) }
					, viewSecond{ ::std::move(v2) } {
				}

				iterator begin() {
					auto it1 = ::std::ranges::begin(this->viewFirst);
					auto e1 = ::std::ranges::end(this->viewFirst);

					// Если первая часть пуста — начинаем сразу со второй.
					// Иначе — начинаем с первой.
					if (it1 == e1) {
						auto it2 = ::std::ranges::begin(this->viewSecond);

						return iterator{
							this,
							false,
							::std::nullopt,
							::std::optional{ it2 }
						};
					}
					else {
						return iterator{
							this,
							true,
							::std::optional{ it1 },
							::std::nullopt
						};
					}
				}

				::std::default_sentinel_t end() {
					return ::std::default_sentinel;
				}

			private:
				// Храним оба представления ПО ЗНАЧЕНИЮ — так мы контролируем их lifetime
				// на всём протяжении итерирования результирующего view.
				TView1 viewFirst;
				TView2 viewSecond;
			};


			//
			// ░ concat_losure
			//
			// Объект-замыкание для пайп-синтаксиса.
			//   v1 | concat(v2)
			// храним второй операнд (как view), а первый придёт в operator|.
			// Гарантируем lifetime второго за счёт хранения внутри closure.
			//
			template <typename TRange2>
			__requires_expr(
				::std::ranges::viewable_range<TRange2>
			) struct concat_closure {
				ranges::tools::view_of_t<TRange2> viewSecond;

				template <typename TRange1>
				__requires_expr(
					::std::ranges::viewable_range<TRange1>
				) friend auto operator|(
					TRange1&& viewFirst,
					const concat_closure& closure
					) {
					using View1_t = ranges::tools::view_of_t<TRange1>;
					using View2_t = ranges::tools::view_of_t<TRange2>;
					return concat_view<View1_t, View2_t>{
						views::tools::as_view(::std::forward<TRange1>(viewFirst)),
							closure.viewSecond
					};
				}
			};


			//
			// ░ concat_fn
			//
			struct concat_fn {
				template <typename TRange2>
				__requires_expr(
					::std::ranges::viewable_range<TRange2>
				) constexpr auto operator()(
					TRange2&& r2
					) const {
					return concat_closure<TRange2>{
						views::tools::as_view(::std::forward<TRange2>(r2))
					};
				}

				template <
					typename TRange1,
					typename TRange2
				>
				__requires_expr(
					::std::ranges::viewable_range<TRange1>&&
					::std::ranges::viewable_range<TRange2>
				) constexpr auto operator()(
					TRange1&& r1,
					TRange2&& r2
					) const {
					using View1_t = ranges::tools::view_of_t<TRange1>;
					using View2_t = ranges::tools::view_of_t<TRange2>;
					return concat_view<View1_t, View2_t>{
						views::tools::as_view(::std::forward<TRange1>(r1)),
							views::tools::as_view(::std::forward<TRange2>(r2))
					};
				}
			};

			inline constexpr concat_fn concat{};
		}
	}

	namespace views = ranges::views;
}