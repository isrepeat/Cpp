#pragma once
#include "Helpers/common.h"
#include <queue>

namespace HELPERS_NS {
	namespace Collection {
		// https://stackoverflow.com/questions/1259099/stdqueue-iteration
		template<typename T, typename TContainer = std::deque<T> >
		class iterable_queue : public std::queue<T, TContainer> {
		public:
			using iterator = TContainer::iterator;
			using const_iterator = TContainer::const_iterator;

			iterator begin() { return this->c.begin(); }
			iterator end() { return this->c.end(); }
			const_iterator begin() const { return this->c.begin(); }
			const_iterator end() const { return this->c.end(); }
		};
	}
}