#pragma once
#include "Helpers/common.h"
#include <deque>

namespace HELPERS_NS {
	namespace Collection {
		template<typename T, typename TContainer>
		class fixed_container : private TContainer {
		public:
			using Base_t = TContainer;
			using Base_t::Base_t;

			using Base_t::size;
			using Base_t::begin;
			using Base_t::end;
			using Base_t::operator[];

			fixed_container(TContainer&& other)
				: TContainer(std::move(other)) {
			}
		};

		template<typename T>
		using fixed_deque = fixed_container<T, std::deque<T>>;
	}
}