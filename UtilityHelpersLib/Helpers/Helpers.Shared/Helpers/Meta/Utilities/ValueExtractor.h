#pragma once
#include "Helpers/common.h"
#include "../Concepts.h"

namespace HELPERS_NS {
	namespace meta {
		class ValueExtractor {
		private:
			template<typename T>
			struct GetFinalType {
				using type = T;
			};

			template<typename T>
			__requires_expr(
				meta::concepts::is_dereferenceable<T>
			) struct GetFinalType<T> {
				using type = typename GetFinalType<
					std::remove_reference_t<decltype(*std::declval<T>())>
				>::type;
			};

		public:
			template<typename T>
			static const typename GetFinalType<T>::type& From(const T& value) {
				if constexpr (concepts::is_dereferenceable<T>) {
					return ValueExtractor::From(*value);
				}
				else {
					return value;
				}
			}
		};
	}
}