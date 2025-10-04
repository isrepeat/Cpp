#pragma once
#include <Helpers/Rational.h>
#include <Helpers/Math.h>

namespace Model {
	template <typename T>
	struct NeuralVar;

	//
	// ░ NeuralVar -> Tensor
	// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
	//
	template <typename T, std::size_t... Dims>
	struct NeuralVar<H::Math::Tensor<T, Dims...>> : H::Math::DifferentialVar<H::Math::Tensor<T, Dims...>> {
		using MyBase_t = H::Math::DifferentialVar<H::Math::Tensor<T, Dims...>>;

		// Сместить this->value в сторону уменьшения ошибки (градиентный спуск).
		template <typename TRep>
		constexpr void ApplyGradient(H::Rational<TRep> learningRate) {
			this->value -= this->grad * learningRate.As<typename MyBase_t::value_type>();
		}
	};


	//
	// ░ NeuralVar -> Scalar
	// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
	//
	template <typename T>
	__requires_expr(
		std::is_arithmetic_v<std::remove_cvref_t<T>>
	) struct NeuralVar<T> : H::Math::DifferentialVar<T> {
		using MyBase_t = H::Math::DifferentialVar<T>;

		template <typename TRep>
		constexpr void ApplyGradient(H::Rational<TRep> learningRate) {
			this->value -= this->grad * learningRate.As<typename MyBase_t::value_type>();
		}
	};


	template <typename T>
	using NeuralScalar = NeuralVar<T>;

	template <typename T, std::size_t N>
	using NeuralVector = NeuralVar<H::Math::Tensor<T, N>>;

	template <typename T, std::size_t R, std::size_t C>
	using NeuralMatrix = NeuralVar<H::Math::Tensor<T, R, C>>;

	template <typename T, std::size_t... Dims>
	using NeuralTensor = NeuralVar<H::Math::Tensor<T, Dims...>>;
}