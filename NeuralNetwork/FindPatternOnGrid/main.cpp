#include <Helpers/Math.h>



// Метка: "Г" на 2x2 — это a=1,b=1 и ровно одна из c,d равна 1
static inline double label_for(unsigned a, unsigned b, unsigned c, unsigned d) {
	return (a && b && (static_cast<unsigned>(c) ^ static_cast<unsigned>(d))) ? 1.0 : 0.0;
}


// Датасет: все 16 комбинаций входов и их метки
struct Sample {
	H::Math::Vec4f x;  // (a,b,c,d)
	double y;
};

std::vector<Sample> make_dataset() {
	std::vector<Sample> ds;
	ds.reserve(16);
	for (unsigned a = 0; a <= 1; ++a)
		for (unsigned b = 0; b <= 1; ++b)
			for (unsigned c = 0; c <= 1; ++c)
				for (unsigned d = 0; d <= 1; ++d) {
					Sample s;
					s.x = H::Math::Vec4f{ double(a), double(b), double(c), double(d) };
					s.y = label_for(a, b, c, d);
					ds.push_back(s);
				}
	return ds;
}

int main() {

	std::function<double(float)> fn = [](char a) {
		return 1;
		};

	fn(1.0);

	auto sigmoidFn = H::Math::Function1D{
		+[](int z) {
			// Чуть более стабильный вариант
			if (z >= 0) {
				double ez = std::exp(-z);
				return 1.0 / (1.0 + ez);
			}
			else {
				double ez = std::exp(z);
				return ez / (1.0 + ez);
			}
		}
	};

	sigmoidFn.SetDerivative<1>(
		[](double a) {
			// a = sigmoid(z) -> a*(1-a)
			return a * (1.0 - a);
		}
	);

	//sigmoidFn(2);
	//sigmoidFn[H::Math::FnForm::Derivative1](2);
}