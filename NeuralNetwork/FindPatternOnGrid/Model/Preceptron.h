#pragma once
#include <Helpers/Math.h>
#include "Activators.h"

namespace Model {
	template<std::size_t N>
	class Perceptron {
	public:
		Perceptron() {
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

		// z = w·x + b (линейная часть)
		double Preactivate(const H::Math::Vec<double, N>& x) {
			this->last_z = this->weights * x + this->bias;
			return this->last_z;
		}

		// y = step(z)
		double Activate(double z) {
			this->last_y = Activators::Step(z);
			return this->last_y;
		}

		// Полный прямой проход
		double Forward(const H::Math::Vec<double, N>& x) {
			return this->Activate(this->Preactivate(x));
		}		
		
	private:
		H::Math::Vec<double, N> weights;
		double bias;

		// Кэши для отладки/будущего обучения перцептрона по алгоритму Розенблатта
		double last_z{}; // w·x + b
		double last_y{}; // step(last_z)
	};
}