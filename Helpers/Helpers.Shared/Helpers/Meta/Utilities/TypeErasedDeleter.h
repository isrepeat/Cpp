#pragma once
#include "Helpers/common.h"

namespace HELPERS_NS {
	namespace meta {
		struct TypeErasedDeleter {		
			using Fn_t = void(*)(void*);

			TypeErasedDeleter() = default;

			explicit TypeErasedDeleter(Fn_t deleterFn)
				: deleterFn{ deleterFn } {
			}

			void operator()(void* p) const {
				if (this->deleterFn != nullptr) {
					this->deleterFn(p);
				}
			}

			template <typename T>
			static TypeErasedDeleter Make() {
				return TypeErasedDeleter{
					[](void* p) {
						delete static_cast<T*>(p);
					}
				};
			}

		private:
			Fn_t deleterFn{ nullptr };
		};
	}
}