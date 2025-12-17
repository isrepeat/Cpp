#pragma once
#include "Helpers/common.h"

namespace HELPERS_NS {
	namespace Math {
		namespace details {
			template <typename T>
			struct Size {
				T width;
				T height;

				Size()
					: width{ 0 }
					, height{ 0 } {
				}

				Size(T width, T height)
					: width{ width }
					, height{ height } {
				}

				bool operator == (const Size& other) const {
					return this->width == other.width && this->height == other.height;
				}
				bool operator != (const Size& other) const {
					return !(*this == other);
				}

				template <typename T>
				explicit operator Size<T>() {
					return Size<T>{
						static_cast<T>(this->width),
							static_cast<T>(this->height)
					};
				}
			};

			template <typename T>
			inline Size<T> operator*(Size<T> size, float value) {
				return Size<T>{
					static_cast<T>(size.width* value),
						static_cast<T>(size.height* value)
				};
			}

			template <typename T>
			inline Size<T> operator/(Size<T> size, float value) {
				return Size<T>{
					static_cast<T>(size.width / value),
						static_cast<T>(size.height / value)
				};
			}
		} // namespace details
	}

	using Size = Math::details::Size<unsigned int>;
	using Size_f = Math::details::Size<float>;
}