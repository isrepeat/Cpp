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
				[](double /*z*/) {
					return 0.0;
				}
			)
		);

		static inline const auto Sigmoid = H::Math::Function1D::Make(
			[](double z) {
				return 1.0 / (1.0 + std::exp(-z));
			},
			H::Math::Function1D::MakeDerivative<1>(
				[](double /*z*/, std::any& metaData) {
					// В производной удобнее зависеть от y = σ(z).
					const double y = std::any_cast<double>(metaData); // y = σ(z)
					return y * (1.0 - y);							  // σ'(z) = σ(z)(1-σ(z))
				}
			)
		);

		static inline const auto ReLU = H::Math::Function1D::Make(
			[](double z) {
				return z > 0.0 ? z : 0.0;
			},
			H::Math::Function1D::MakeDerivative<1>(
				[](double z) {
					// В производной удобнее зависеть от z.
					return z > 0.0 ? 1.0 : 0.0;
				}
			)
		);
	};
}