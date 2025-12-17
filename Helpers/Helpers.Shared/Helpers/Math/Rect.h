#pragma once
#include "Helpers/common.h"

namespace HELPERS_NS {
	namespace Math {
		namespace details {
			template <typename T>
			struct Rect {
				T left;
				T top;
				T right;
				T bottom;

				Rect()
					: left{ 0 }
					, top{ 0 }
					, right{ 0 }
					, bottom{ 0 } {
				}

				Rect(T left, T top, T right, T bottom)
					: left{ left }
					, top{ top }
					, right{ right }
					, bottom{ bottom } {
				}

				template <typename T>
				explicit operator Rect<T>() {
					return Rect<T>{
						static_cast<T>(this->left),
							static_cast<T>(this->top),
							static_cast<T>(this->right),
							static_cast<T>(this->bottom)
					};
				}
			};
		} // namespace details
	}

	using Rect = Math::details::Rect<unsigned int>;
	using Rect_f = Math::details::Rect<float>;
}