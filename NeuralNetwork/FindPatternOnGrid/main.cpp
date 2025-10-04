#include <Helpers/Logger.h>
#include <Helpers/Math.h>
#include "Model/Neuron.h"

#include <iostream>
#include <random>
#include <array>



struct TrainConfig {
	int epochs = 2000;						// сколько раз обойти весь датасет
	H::Rational<double> learningRate{1, 2}; // скорость обучения (η)
	uint32_t seed = 123;					// для воспроизводимости
	bool shuffle_each_epoch = true;
	double l2 = 0.0;						// L2-регуляризация (0 = выкл)
	double label_smooth = 0.0;				// 0.05 => 1→0.95, 0→0.05
};

using Vec4 = H::Math::Vec<double, 4>;

struct Sample { 
	Vec4 x;
	double t;
};

static std::vector<Sample> BuildDataset() {
	std::vector<Sample> ds;
	ds.reserve(16);
	for (int a = 0; a < 2; ++a) for (int b = 0; b < 2; ++b)
		for (int c = 0; c < 2; ++c) for (int d = 0; d < 2; ++d) {
			Vec4 x{ double(a), double(b), double(c), double(d) };
			double t = (a == 1 && b == 1 && c == 1 && d == 0) ? 1.0 : 0.0; // «Г»
			ds.push_back({ x, t });
		}
	return ds;
}

static inline double Smooth(double t, double eps) {
	if (eps <= 0.0) {
		return t;
	}
	// простое сглаживание: 1→1−eps, 0→eps
	return t > 0.5 ? (1.0 - eps) : eps;
}

// Функция потерь
double Loss(double y, double t) {
	const double eps = 1e-12;
	return -(t * std::log(y + eps) + (1.0 - t) * std::log(1.0 - y + eps));
}

// один проход по датасету (1 эпоха)
static double TrainOneEpoch(Model::Neuron<4>& n,
	std::vector<Sample>& ds,
	const TrainConfig& cfg,
	std::mt19937& rng
) {
	if (cfg.shuffle_each_epoch) {
		std::shuffle(ds.begin(), ds.end(), rng);
	}

	//const double eps = 1e-12;
	double loss_sum = 0.0;

	for (const auto& s : ds) {
		const double t = Smooth(s.t, cfg.label_smooth);

		// forward
		const double y = n.Forward(s.x);

		// BCE
		const double loss = Loss(y, t);
		loss_sum += loss;

		// dL/dz = y - t  (для σ+BCE)
		const double delta = (y - t);

		// градиенты
		H::Math::Vec<double, 4> gradW{
			delta * s.x[0],
			delta * s.x[1],
			delta * s.x[2],
			delta * s.x[3]
		};
		double gradB = delta;

		// L2: ∂L/∂w += λ w
		if (cfg.l2 > 0.0) {
			for (size_t i = 0; i < 4; ++i) {
				gradW[i] += cfg.l2 * n.Data().weights.value[i];
			}
		}

		// прокинуть градиенты внутрь и обновиться
		n.SetDataGradient(std::move(gradW), gradB);
		n.ApplyGradient(cfg.learningRate);
	}

	return loss_sum / double(ds.size());
}

// быстрая валидация на всех 16 паттернах
static std::pair<int, double> EvalAll(const Model::Neuron<4>& n_in) {
	auto ds = BuildDataset();
	int correct = 0;
	double loss_sum = 0.0;
	const double eps = 1e-12;

	for (const auto& s : ds) {
		auto n = n_in;                 // Forward у тебя не const — берём копию
		double y = n.Forward(s.x);
		if ((y >= 0.5) == (s.t > 0.5)) ++correct;
		loss_sum += -(s.t * std::log(y + eps) + (1.0 - s.t) * std::log(1.0 - y + eps));
	}
	return { correct, loss_sum / double(ds.size()) };
}

// ВАЖНО: здесь явный цикл по epochs
static void Train(Model::Neuron<4>& n, const TrainConfig& cfg) {
	auto ds = BuildDataset();
	std::mt19937 rng(cfg.seed);

	for (int e = 1; e <= cfg.epochs; ++e) {
		double train_loss = TrainOneEpoch(n, ds, cfg, rng);

		// логируем редко, чтобы не засорять вывод
		if (e == 1 || e % 100 == 0 || e == cfg.epochs) {
			auto [acc, avg_bce] = EvalAll(n);
			std::printf("epoch %4d | train_loss=%.6f | acc=%2d/16 | avgBCE=%.6f\n",
				e, train_loss, acc, avg_bce);
		}
	}
}


int main() {
	Model::Neuron<4> n(Model::Activators::Sigmoid);
	TrainConfig cfg;
	cfg.epochs = 2000;
	cfg.learningRate = {1, 2};

	Train(n, cfg);

	// Проверка трёх кейсов:
	auto y_pos = n.Predict({ 1, 1, 1, 0 });
	auto y_neg1 = n.Predict({ 1, 1, 1, 1 });
	auto y_neg2 = n.Predict({ 0, 1, 1, 0 });

	LOG_DEBUG_D("\n"
		"y_pos = {}\n"
		"y_neg1 = {}\n"
		"y_neg2 = {}\n"
		, y_pos
		, y_neg1
		, y_neg2
	);
	return 0;
}



//struct G2x2Trainer {
//	using Vec4 = H::Math::Vec<double, 4>;
//
//	// Паттерны 2x2 раскладываем в вектор (r0c0, r0c1, r1c0, r1c1), значения {0,1}.
//	// Определим «Г» как:
//	// [1 1]
//	// [1 0]
//	static constexpr Vec4 kG = Vec4{ 1, 1, 1, 0 };
//
//	struct Sample { 
//		Vec4 x;   // вход
//		double t; // целевая метка
//	}; 
//
//	// Полный датасет: все 16 конфигураций 2x2, «1» только для точного совпадения с Г
//	static auto BuildDataset() {
//		std::array<Sample, 16> ds{};
//		int idx = 0;
//		for (int a = 0; a <= 1; ++a) {
//			for (int b = 0; b <= 1; ++b) {
//				for (int c = 0; c <= 1; ++c) {
//					for (int d = 0; d <= 1; ++d) {
//						Vec4 x{ (double)a, (double)b, (double)c, (double)d };
//						double t = (a == 1 && b == 1 && c == 1 && d == 0) ? 1.0 : 0.0;
//						ds[idx++] = Sample{ x, t };
//					}
//				}
//			}
//		}
//		return ds;
//	}
//
//	// Инициализация весов небольшим шумом
//	static void InitSmallRandom(Model::Neuron<4>& n, uint32_t seed = 42) {
//		std::mt19937 rng(seed);
//		std::uniform_real_distribution<double> U(-0.5, 0.5);
//		H::Math::Vec<double, 4> w{ U(rng), U(rng), U(rng), U(rng) };
//		double b = U(rng) * 0.1;
//		n.SetData(std::move(w), b);
//	}
//
//	// Один шаг обучения по одному сэмплу (SGD)
//	template <typename TRep>
//	static double TrainStep(
//		Model::Neuron<4>& n,
//		const Vec4& x,
//		double t,
//		H::Rational<TRep> learningRate
//	) {
//		// Forward
//		const double y = n.Forward(x);
//
//		// BCE loss (для логирования/контроля)
//		const double eps = 1e-12;
//		const double loss = -(t * std::log(y + eps) + (1.0 - t) * std::log(1.0 - y + eps));
//
//		// Для σ+BCE: dL/dz = (y - t). Градиенты по параметрам:
//		const double delta = (y - t);
//		H::Math::Vec<double, 4> gradW{
//			delta * x[0],
//			delta * x[1],
//			delta * x[2],
//			delta * x[3]
//		};
//		const double gradB = delta;
//
//		// Прокинем в твои NeuralVar градиенты
//		n.SetDataGradient(std::move(gradW), gradB);
//
//		// Обновление параметров через твою обёртку (градиентный спуск)
//		// (ApplyGradient реализован в NeuralVar)
//		// Замечание: тут мы используем один и тот же learningRate для w и b.
//		// При желании можно сделать разные скорости.
//		// Важно: знак "минус" уже внутри ApplyGradient.
//		// Если у тебя Rational<int> — ок, As<double>() всё сведёт к double.
//		n.ApplyGradient(learningRate);
//
//		return loss;
//	}
//
//	// Полноценный цикл обучения
//	template <typename TRep>
//	static void Train(
//		Model::Neuron<4>& n,
//		int epochs,
//		H::Rational<TRep> learningRate
//	) {
//		auto ds = BuildDataset();
//
//		for (int e = 1; e <= epochs; ++e) {
//			double epochLoss = 0.0;
//			for (const auto& s : ds) {
//				epochLoss += TrainStep(n, s.x, s.t, learningRate);
//			}
//			epochLoss /= ds.size();
//
//			// Простейший мониторинг
//			// (можно заменить на твой логгер)
//			// printf("epoch %d | loss=%.6f\n", e, epochLoss);
//		}
//	}
//
//	static double Predict(const Model::Neuron<4>& n, const Vec4& x) {
//		// Нужен небольшой доступ: Forward не const (из-за кэшей).
//		// Обойтись можно, создав копию n, либо добавить const-вариант.
//		// Здесь создадим копию:
//		auto tmp = n;
//		return tmp.Forward(x);
//	}
//};
//
//
//int main() {
//	Model::Neuron<4> n(Model::Activators::Sigmoid);
//	
//	G2x2Trainer::InitSmallRandom(n, /*seed*/ 123);
//
//	// Обучим 2000 эпох с lr = 0.5 (подбери по вкусу)
//	auto learningRate = H::Rational<double>{ 1, 2 };
//	G2x2Trainer::Train(n, /*epochs*/ 2000, learningRate);
//
//	// Проверим несколько кейсов
//	auto y_pos = G2x2Trainer::Predict(n, G2x2Trainer::kG);						// ожидание: близко к 1
//	auto y_neg1 = G2x2Trainer::Predict(n, H::Math::Vec<double, 4>{1, 1, 1, 1}); // всё единицы → 0
//	auto y_neg2 = G2x2Trainer::Predict(n, H::Math::Vec<double, 4>{0, 1, 1, 0}); // «угадали» 3 клетки иначе → 0
//
//	LOG_DEBUG_D("\n"
//		"y_pos = {}\n"
//		"y_neg1 = {}\n"
//		"y_neg2 = {}\n"
//		, y_pos
//		, y_neg1
//		, y_neg2
//	);
//	return 0;
//}