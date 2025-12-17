#pragma once
#include "Helpers/common.h"
#include <type_traits>

namespace HELPERS_NS {
	namespace meta {
		template <typename T>
		struct ExtractUnderlyingType {
			using type = T;
		};

		template <template<typename> typename ContainerT, typename ItemT>
		struct ExtractUnderlyingType<ContainerT<ItemT>> {
			using type = ItemT;
		};
	}
}