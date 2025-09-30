#pragma once
#include <Helpers/Math.h>
#include "Activators.h"

namespace Model {
	template <std::size_t N>
	class Neuron {
	public:
		Neuron(
			H::Math::Function1D activationFn = Activators::Sigmoid
		)
			: activationFn(std::move(activationFn)) {
		}

		std::size_t Size() const {
			return this->weights.Size();
		}

		void SetWeights(
			H::Math::Vec<double, N> newWeights,
			double newBias
		) {
			this->weights = std::move(newWeights);
			this->bias = newBias;
		}

		void SetGrads(
			H::Math::Vec<double, N> newGradWeights,
			double newGradBias
		) {
			this->gradWeights = std::move(newGradWeights);
			this->gradBias = newGradBias;
		}

		// z = w·x + b (линейная часть)
		double Preactivate(const H::Math::Vec<double, N>& x) {
			this->last_z = this->weights * x + this->bias;
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

	private:
		H::Math::Function1D activationFn;
		H::Math::Vec<double, N> weights;
		double bias;

		// Буферы градиентов (будут заполнены в обучении)
		H::Math::Vec<double, N> gradWeights; // dL/dw
		double gradBias{}; // dL/db

		// Кэши для будущего backprop
		double last_z{}; // z = w·x + b
		double last_y{}; // y = act(z)
	};
}