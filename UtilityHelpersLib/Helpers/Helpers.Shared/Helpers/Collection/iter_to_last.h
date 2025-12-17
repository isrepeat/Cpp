#pragma once
#include "Helpers/common.h"
#include <stdexcept>

namespace HELPERS_NS {
	namespace Collection {
		// https://stackoverflow.com/questions/51235181/c-iterator-to-last-element-of-a-linked-list
		template<typename TContainer>
		auto iter_to_last(TContainer&& cont) {
			auto last = std::end(cont);
			if (last == std::begin(cont)) {
				throw std::invalid_argument("container is empty");
			}
			return std::prev(last);
		}
	}
}