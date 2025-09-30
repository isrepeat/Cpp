#pragma once
#include <Helpers/Math.h>

namespace Model {
	struct Activators {
		static inline const auto Step = H::Math::Function1D::Make(
			[](double z) {
				// Пороговая активация: 1 если z > 0, иначе 0
				return z > 0.0 ? 1.0 : 0.0;
			},
			H::Math::Function1D::MakeDerivative<1>(
				// Производная не используется для перцептрона (но оставим сигнатуру для совместимости)
				[](double /*z*/, double /*y*/) {
					return 0.0;
				}
			)
		);

		static inline const auto Sigmoid = H::Math::Function1D::Make(
			[](double z) {
				return 1.0 / (1.0 + std::exp(-z));
			},
			H::Math::Function1D::MakeDerivative<1>(
				// derivative может принимать либо z, либо уже y=?(z); используем y для численной устойчивости
				[](double /*z*/, double y) {
					return y * (1.0 - y);
				}
			)
		);

		static inline const auto ReLU = H::Math::Function1D::Make(
			[](double z) {
				return z > 0.0 ? z : 0.0;
			},
			H::Math::Function1D::MakeDerivative<1>(
				[](double z, double /*y*/) {
					return z > 0.0 ? 1.0 : 0.0;
				}
			)
		);
	};
}