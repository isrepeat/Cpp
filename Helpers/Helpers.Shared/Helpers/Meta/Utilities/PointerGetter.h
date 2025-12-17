#pragma once
#include "Helpers/common.h"
#include <utility>
#include <memory>

namespace HELPERS_NS {
	namespace meta {
		class PointerGetter {
		public:
			template<class T>
			static T* Get(T* v) {
				return v;
			}
			template<class T>
			static const T* Get(const T* v) {
				return v;
			}
			template<class T>
			static T* Get(T& v) {
				return std::addressof(v);
			}
			template<class T>
			static const T* Get(const T& v) {
				return std::addressof(v);
			}
			template<class T>
			static T* Get(std::unique_ptr<T>& v) {
				return v.get();
			}
			template<class T>
			static const T* Get(const std::unique_ptr<T>& v) {
				return v.get();
			}
			template<class T>
			static T* Get(std::shared_ptr<T>& v) {
				return v.get();
			}
			template<class T>
			static const T* Get(const std::shared_ptr<T>& v) {
				return v.get();
			}
		};
	}
}