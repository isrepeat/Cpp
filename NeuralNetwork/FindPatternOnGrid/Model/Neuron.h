#pragma once
#include "Activators.h"
#include "NeuralVar.h"

namespace Model {
	template <std::size_t N>
	class Neuron {
	public:
		struct _Data {
			NeuralVector<double, N> weights;
			NeuralScalar<double> bias;
		};

		Neuron(H::Math::Function1D activationFn = Activators::Sigmoid)
			: activationFn(std::move(activationFn)) {
		}

		const _Data& Data() const {
			return this->data;
		}

		void SetData(
			H::Math::Vec<double, N> newWeights,
			double newBias
		) {
			this->data.weights.value = std::move(newWeights);
			this->data.bias.value = newBias;
		}

		void SetDataGradient(
			H::Math::Vec<double, N> newGradWeights,
			double newGradBias
		) {
			this->data.weights.grad = std::move(newGradWeights);
			this->data.bias.grad = newGradBias;
		}

		template <typename TRep>
		void ApplyGradient(H::Rational<TRep> learningRate) {
			this->data.weights.ApplyGradient(learningRate);
			this->data.bias.ApplyGradient(learningRate);
		}

		// z = w·x + b (линейная часть)
		double Preactivate(const H::Math::Vec<double, N>& x) {
			this->last_z = this->data.weights.value * x + this->data.bias.value;
			return this->last_z;
		}

		// y = activationFn(z)
		double Activate(double z) {
			this->last_y = this->activationFn(z);
			return this->last_y;
		}

		// Полный прямой проход
		double Forward(const H::Math::Vec<double, N>& x) {
			return this->Activate(this->Preactivate(x));
		}

		// Для тестирования
		double Predict(const H::Math::Vec<double, N>& x) const {
			const double z = this->data.weights.value.value * x + this->data.bias.value;
			return this->activationFn(z);
		}

	private:
		_Data data;
		H::Math::Function1D activationFn;
		
		// Кэши для будущего backprop
		double last_z{}; // z = w·x + b
		double last_y{}; // y = act(z)
	};
}