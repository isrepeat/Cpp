#pragma once
#include "Helpers/common.h"

#if __cpp_concepts
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

#else

#include <type_traits>
#include <utility>

namespace HELPERS_NS {
    namespace detail {
        template <typename T, typename = void>
        struct is_dereferenceable_impl : std::false_type {
        };

        template <typename T>
        struct is_dereferenceable_impl<T, std::void_t<decltype(*std::declval<T>())>> : std::true_type {
        };

        template <typename T>
        inline constexpr bool is_dereferenceable_v = is_dereferenceable_impl<T>::value;
    }

    class ValueExtractor {
    private:
        template <typename T, typename Enable = void>
        struct GetFinalType {
            using type = T;
        };

        template <typename T>
        struct GetFinalType<T, std::enable_if_t<detail::is_dereferenceable_v<T>>> {
            using type = typename GetFinalType<
                std::remove_reference_t<decltype(*std::declval<T>())>
            >::type;
        };

    public:
        template <typename T>
        static const typename GetFinalType<T>::type& From(const T& value) {
            if constexpr (detail::is_dereferenceable_v<T>) {
                return ValueExtractor::From(*value);
            }
            else {
                return value;
            }
        }
    };
}
#endif