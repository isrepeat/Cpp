#pragma once
#include "Helpers/common.h"
#include "Helpers/Meta/Concepts.h"
#include "Tools.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <optional>
#include <utility>
#include <vector>
#include <ranges>

namespace STD_EXT_NS {
	namespace ranges {
		namespace views {
			namespace details {
				namespace concepts {
					template <typename TGetChildrenFn, typename TNode>
					concept children_fn_return_input_range = requires(
						TGetChildrenFn getChildrenFn,
						TNode node
						) {
							{ getChildrenFn(node) } -> ::std::ranges::input_range;
					};

					// TGetChildrenFn соответствует узлу TNode и возвращает input_range
					template <typename TGetChildrenFn, typename TNode>
					concept children_fn_valid =
						::std::same_as<
						::std::ranges::range_value_t<::std::invoke_result_t<TGetChildrenFn, TNode>>,
						TNode
						>
						&&
						children_fn_return_input_range<TGetChildrenFn, TNode>;
				}
			}

			//
			// ░ flatten_tree
			// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
			//
			// ░ flatten_tree_view
			//
			// Почему внутри этого класса мы используем decayed-тип коллэбла (TGetChildrenFn_decayed_t)?
			// 1) Без нормализации тип TGetChildrenFn может выводиться как ссылочный (например, const Lambda&
			//    из-за perfect-forwarding). Тогда при хранении указателя на такой тип получится «указатель на
			//    ссылку», что в C++ запрещено и приводит к ошибке вида "pointer to reference is illegal".
			//    Decay гарантирует, что мы храним ЗНАЧЕНИЕ, а итератор держит обычный указатель на это значение.
			// 2) Хранение по значению даёт стабильное время жизни и предсказуемость: ни висячих ссылок, ни
			//    зависимости от того, был ли исходный коллэбл lvalue/rvalue/const и т.п.
			// 3) Унификация типов и чище ошибки компиляции: независимо от cv/ref-квалификаций на входе внутри
			//    view всегда один «канонический» тип. Это сокращает число инстанциаций и делает диагностические
			//    сообщения короче.
			// 4) Хотя для лямбд/функторов обычно достаточно remove_cvref_t, мы используем decay_t как более
			//    универсальную нормализацию: помимо снятия cv/ref, она обрабатывает «функция → указатель на
			//    функцию» и «массив → указатель на элемент». Это делает код устойчивее, если когда-нибудь
			//    коллэбл окажется типом функции.
			template <
				typename TView,
				typename TGetChildrenFn
			>
			__requires_expr(
				::std::ranges::view<TView>
			) class flatten_tree_view : public ::std::ranges::view_interface<flatten_tree_view<TView, TGetChildrenFn>> {
			public:
				using Node_t = ::std::ranges::range_value_t<TView>;
				using TGetChildrenFn_decayed_t = ::std::decay_t<TGetChildrenFn>; // нормализуем тип коллэбла

				class iterator {
				public:
					// Соглашения итераторов для STL
					using iterator_concept = ::std::input_iterator_tag;
					using iterator_category = ::std::input_iterator_tag;
					using difference_type = ::std::ptrdiff_t;
					using value_type = Node_t;
					using reference = Node_t;

				public:
					iterator() = default;

					explicit iterator(
						const TGetChildrenFn_decayed_t* getChildrenFnPtr,
						::std::vector<Node_t>&& roots
					)
						: getChildrenFnPtr{ getChildrenFnPtr }
						, stack{ ::std::move(roots) } {
						// stack изначально содержит только корни, причем в прямом порядке.
						// Для правильного обхода слева-напрво разворачиваем корни.
						::std::reverse(this->stack.begin(), this->stack.end());
						this->advance_to_next();
					}

					reference operator*() const {
						return this->currentNode;
					}

					const Node_t* operator->() const {
						return ::std::addressof(this->currentNode);
					}

					iterator& operator++() {
						this->advance_to_next();
						return *this;
					}

					void operator++(int) {
						++(*this);
					}

					bool operator==(::std::default_sentinel_t) const {
						return this->atEnd;
					}

					friend bool operator==(::std::default_sentinel_t s, const iterator& it) {
						return it == s;
					}

				private:
					void advance_to_next() {
						// Реализует обход дерева слева-направо.
						// К примру есть дерево:
						// Root
						//  ├─ A
						//  │  ├─ A1
						//  │  └─ A2
						//  └─ B
						//     └─ B1
						// 
						// тогда результат обхода будет: [Root, A, A1, A2, B, B1, B2].

						if (this->stack.empty()) {
							this->atEnd = true;
							return;
						}

						this->currentNode = ::std::move(this->stack.back());
						this->stack.pop_back();

						auto currentNodeСhildrenView = (*this->getChildrenFnPtr)(this->currentNode);

						// Собираем детей «слева направо»:
						::std::vector<Node_t> currentNodeСhildren;
						for (auto&& child : currentNodeСhildrenView) {
							if (static_cast<bool>(child)) {
								currentNodeСhildren.push_back(child);
							}
						}

						// ...и затем кладём их в стек в ОБРАТНОМ порядке (right => left),
						// чтобы при LIFO-извлечении порядок обхода стал left => right.
						// 
						// К примеру если сейчас stack = [B, A*, Root*] == [B]
						// то после цикла станет stack = [B, A*, Root*, A2, A1] == [B, A2, A1]
						// (* - означает что элемент пройден и удален из стека)
						for (auto it = currentNodeСhildren.rbegin(); it != currentNodeСhildren.rend(); ++it) {
							this->stack.push_back(::std::move(*it));
						}
					}

				private:
					const TGetChildrenFn_decayed_t* getChildrenFnPtr{ nullptr };
					::std::vector<Node_t> stack; // LIFO-стек для обхода слева-направо
					Node_t currentNode;
					bool atEnd = false;
				}; // class iterator


				flatten_tree_view(
					TView rootsView,
					const TGetChildrenFn_decayed_t& getChildrenFn
				)
					: rootsView{ ::std::move(rootsView) }
					, getChildrenFn{ getChildrenFn } {
					static_assert(
						details::concepts::children_fn_valid<TGetChildrenFn_decayed_t, Node_t>,
						"TGetChildrenFn must return an input_range with range_value_t == Node_t"
						);
				}

				auto begin() {
					::std::vector<Node_t> roots;

					// Собираем корни «слева направо»:
					for (auto&& r : this->rootsView) {
						if (static_cast<bool>(r)) {
							roots.push_back(r);
						}
					}

					return iterator{
						::std::addressof(this->getChildrenFn),
						::std::move(roots)
					};
				}

				auto end() {
					return ::std::default_sentinel;
				}

			private:
				TView rootsView;
				TGetChildrenFn_decayed_t getChildrenFn;
			};


			//
			// ░ flatten_tree_closure
			//
			template <typename TGetChildrenFn>
			struct flatten_tree_closure {
			public:
				TGetChildrenFn getChildrenFn;

			public:
				template <typename TRange>
				__requires_expr(
					::std::ranges::viewable_range<TRange>
				) friend auto operator|(
					TRange&& roots,
					const flatten_tree_closure& self
					) {
					using TView = ranges::tools::view_of_t<TRange>;
					using Node_t = ::std::ranges::range_value_t<TView>;

					static_assert(
						details::concepts::children_fn_valid<TGetChildrenFn, Node_t>,
						"TGetChildrenFn(node) must return an input_range with range_value_t == Node_t"
						);

					return flatten_tree_view<TView, TGetChildrenFn>{
						views::tools::as_view(::std::forward<TRange>(roots)),
							self.getChildrenFn
					};
				}
			};


			//
			// ░ flatten_tree_fn
			//
			struct flatten_tree_fn {
			public:
				template <typename TRange, typename TGetChildrenFn>
				__requires_expr(
					::std::ranges::viewable_range<TRange>
				) constexpr auto operator()(
					TRange&& roots,
					TGetChildrenFn&& getChildrenFn
					) const {
					using TView = ranges::tools::view_of_t<TRange>;
					using Node_t = ::std::ranges::range_value_t<TView>;

					static_assert(
						details::concepts::children_fn_valid<TGetChildrenFn, Node_t>,
						"get_children(node) must return an input_range with range_value_t == Node_t"
						);

					return flatten_tree_view<TView, TGetChildrenFn>{
						views::tools::as_view(::std::forward<TRange>(roots)),
							::std::forward<TGetChildrenFn>(getChildrenFn)
					};
				}

				template <typename TGetChildrenFn>
				constexpr auto operator()(
					TGetChildrenFn&& getChildrenFn
					) const {
					return flatten_tree_closure<TGetChildrenFn>{
						.getChildrenFn = ::std::forward<TGetChildrenFn>(getChildrenFn)
					};
				}
			};


			inline constexpr flatten_tree_fn flatten_tree{};
		}
	}

	namespace views = ranges::views;
}


//
// ░ Export concepts
// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
//
namespace HELPERS_NS {
	namespace meta {
		namespace concepts {
			// ...
		}
	}
}