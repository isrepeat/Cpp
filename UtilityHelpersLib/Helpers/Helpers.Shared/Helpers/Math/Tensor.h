#pragma once
#include "Helpers/common.h"
#include "Helpers/Meta/Concepts.h"

#if _HAS_CXX20
#include <type_traits>
#include <cassert>
#include <cstddef>
#include <utility>
#include <array>

namespace HELPERS_NS {
	namespace Math {
		//
		// ░ Tensor
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		template <typename TValue, std::size_t... Dims>
		class Tensor {
		public:
			using value_type = std::remove_cvref_t<TValue>;

			static constexpr std::size_t kRank = sizeof...(Dims);
			static constexpr std::size_t kSize = (1 * ... * Dims);

		private:
			// Extents: размеры по осям.
			static consteval std::array<std::size_t, kRank> MakeExtents() {
				return { Dims... };
			}

			// Strides: шаги по осям (row-major).
			static consteval std::array<std::size_t, kRank> MakeStrides() {
				std::array<std::size_t, kRank> strides{};

				if constexpr (kRank == 0) {
					// Ранг 0: пустые extents/strides; размер = 1, данных 1 элемент.
					return strides;
				}
				else {
					const auto& extentsRef = Tensor::kExtents;

					strides[kRank - 1] = 1;
					for (std::size_t i = kRank - 1; i > 0; --i) {
						strides[i - 1] = strides[i] * extentsRef[i];
					}

					return strides;
				}
			}

		public:
			static constexpr std::array<std::size_t, kRank> kExtents = Tensor::MakeExtents();
			static constexpr std::array<std::size_t, kRank> kStrides = Tensor::MakeStrides();

			static constexpr std::size_t Rank() { return Tensor::kRank; }
			static constexpr std::size_t Size() { return Tensor::kSize; }
			static constexpr const std::array<std::size_t, kRank>& Extents() { return Tensor::kExtents; }
			static constexpr const std::array<std::size_t, kRank>& Strides() { return Tensor::kStrides; }

			constexpr Tensor() = default;

			template <typename... TVals>
			__requires_expr(
				(sizeof...(TVals) == kSize) &&
				(std::conjunction_v<std::is_convertible<TVals, value_type>...>)
			) constexpr /*explicit*/ Tensor(TVals&&... vals)
				: dataArray{
					static_cast<value_type>(std::forward<TVals>(vals))...
				} {
			}

			// Смещение по массиву индексов: offset = Σ idx[d] * stride[d]
			static constexpr std::size_t IndexToOffset(
				const std::array<std::size_t, kRank>& indicesArray
			) {
				std::size_t offset = 0;
				for (std::size_t d = 0; d < kRank; ++d) {
					offset += indicesArray[d] * Tensor::kStrides[d];
				}
				return offset;
			}

			// Удобный вариадик-оверлоад: IndexToOffset(i,j,k,...)
			template <typename... TIdx>
			static constexpr std::size_t IndexToOffset(TIdx... idx) {
				static_assert(sizeof...(idx) == kRank, "rank mismatch");
				std::array<std::size_t, kRank> indicesArray{ static_cast<std::size_t>(idx)... };
				return Tensor::IndexToOffset(indicesArray);
			}

			constexpr value_type* Data() {
				return this->dataArray.data();
			}

			constexpr const value_type* Data() const {
				return this->dataArray.data();
			}

			constexpr void Fill(const value_type& value) {
				for (std::size_t i = 0; i < Tensor::kSize; ++i) {
					this->dataArray[i] = value;
				}
			}


			// Прокси для промежуточных уровней индексации.
			template <bool IsConst, std::size_t Level>
			class IndexProxy {
			public:
				using elem_t = std::conditional_t<IsConst, const value_type, value_type>;

				constexpr IndexProxy(
					std::conditional_t<IsConst, const value_type*, value_type*> basePtr,
					std::size_t offsetFlat
				)
					: basePtr{ basePtr }
					, offsetFlat{ offsetFlat } {
				}

				// Ещё не на последнем уровне → вернуть следующий прокси
				constexpr auto operator[](std::size_t i) const {
					// шаг по текущему измерению (row-major)
					const std::size_t stride = Tensor::kStrides[Level];
					const std::size_t next = this->offsetFlat + i * stride;

					if constexpr (Level + 1 == Tensor::kRank) {
						// последний уровень → вернуть ссылку на элемент
						return const_cast<elem_t&>(this->basePtr[next]);
					}
					else {
						// промежуточный уровень
						using next_proxy = IndexProxy<IsConst, Level + 1>;
						return next_proxy{ this->basePtr, next };
					}
				}

			private:
				std::conditional_t<IsConst, const value_type*, value_type*> basePtr;
				std::size_t offsetFlat;
			}; // class IndexProxy


			// rank == 1: возвращаем ссылки на элементы
			template <std::size_t Rank = Tensor::kRank>
			__requires_expr(
				Rank == 1
			) constexpr value_type& operator[](std::size_t i) {
				const std::size_t off = i * this->kStrides[0];
				return this->dataArray[off];
			}

			template <std::size_t Rank = Tensor::kRank>
			__requires_expr(
				Rank == 1
			) constexpr const value_type& operator[](std::size_t i) const {
				const std::size_t off = i * this->kStrides[0];
				return this->dataArray[off];
			}

			// rank != 1: возвращаем прокси-объект
			template <std::size_t Rank = Tensor::kRank>
			__requires_expr(
				Rank != 1
			) constexpr IndexProxy<false, 1> operator[](std::size_t i) {
				const std::size_t off = i * this->kStrides[0];
				return IndexProxy<false, 1>{ this->dataArray.data(), off};
			}

			template <std::size_t Rank = Tensor::kRank>
			__requires_expr(
				Rank != 1
			) constexpr IndexProxy<true, 1> operator[](std::size_t i) const {
				const std::size_t off = i * this->kStrides[0];
				return IndexProxy<true, 1>{ this->dataArray.data(), off};
			}

			constexpr bool operator==(const Tensor& other) const {
				for (std::size_t i = 0; i < kSize; ++i) {
					if (this->dataArray[i] != other.dataArray[i]) {
						return false;
					}
				}
				return true;
			}

			constexpr bool operator!=(const Tensor& other) const {
				return !(*this == other);
			}

		private:
			std::array<value_type, kSize> dataArray{};
		};


		//
		// ░ Aliases
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		template <typename T, std::size_t N>
		using Vec = Tensor<T, N>;

		using Vec2f = Tensor<double, 2>;
		using Vec3f = Tensor<double, 3>;
		using Vec4f = Tensor<double, 4>;

		template <typename T, std::size_t R, std::size_t C>
		using Mat = Tensor<T, R, C>;

		using Mat1x1f = Tensor<double, 1, 1>;
		using Mat2x2f = Tensor<double, 2, 2>;
		using Mat3x3f = Tensor<double, 3, 3>;
		using Mat4x4f = Tensor<double, 4, 4>;


		//
		// ░ Details
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		namespace details {
			// ---------- вспомогательные мета-типы над пакетами размеров ----------
			template <std::size_t... Dims>
			struct dims_pack {};


			template <typename A, typename B>
			struct pack_concat;

			template <std::size_t... L, std::size_t... R>
			struct pack_concat<dims_pack<L...>, dims_pack<R...>> {
				using type = dims_pack<L..., R...>;
			};


			template <typename P>
			struct pack_pop_first;

			template <std::size_t H, std::size_t... T>
			struct pack_pop_first<dims_pack<H, T...>> {
				using type = dims_pack<T...>;
			};


			template <typename P>
			struct pack_pop_last;

			template <std::size_t Last>
			struct pack_pop_last<dims_pack<Last>> {
				using type = dims_pack<>;
			};

			template <std::size_t H, std::size_t... T>
			struct pack_pop_last<dims_pack<H, T...>> {
				using type = typename pack_concat<dims_pack<H>, typename pack_pop_last<dims_pack<T...>>::type>::type;
			};


			template <typename T, typename P>
			struct tensor_from_pack;

			template <typename T, std::size_t... D>
			struct tensor_from_pack<T, dims_pack<D...>> {
				using type = Tensor<T, D...>;
			};


			// --- мета-константы: первый и последний элемент пакета ---
			template <typename P>
			struct pack_size;

			template <std::size_t... Dims>
			struct pack_size<dims_pack<Dims...>> {
				static constexpr std::size_t value = sizeof...(Dims);
			};


			template <typename P>
			struct pack_first;

			template <std::size_t H, std::size_t... T>
			struct pack_first<dims_pack<H, T...>> {
				static constexpr std::size_t value = H;
			};


			template <typename P>
			struct pack_last;

			template <std::size_t X>
			struct pack_last<dims_pack<X>> {
				static constexpr std::size_t value = X;
			};

			template <std::size_t H, std::size_t... T>
			struct pack_last<dims_pack<H, T...>> {
				static constexpr std::size_t value = pack_last<dims_pack<T...>>::value;
			};


			// --- concept: размеры совместимы для «последняя × первая» ---
			template <typename LP, typename RP>
			concept ContractibleDimsPacks =
				(pack_size<LP>::value >= 1) &&
				(pack_size<RP>::value >= 1) &&
				(pack_last<LP>::value == pack_first<RP>::value);


			// --------------------------------------------------------------------
			//  Контракция «последняя ось lhs × первая ось rhs».
			//  Для матриц это Mat<R,C>*Mat<C,K> -> Mat<R,K>.
			//  Для общих тензоров: Tensor<T, L0..L{m-2},L{m-1}> * Tensor<T, R0..R{n-1}>
			//  при L{m-1} == R0  ⇒  Tensor<T, L0..L{m-2}, R1..R{n-1}>
			// --------------------------------------------------------------------
			template <
				typename T,
				std::size_t... L,
				std::size_t... R,
				// Тип результата: L без последней  +  R без первой
				typename TOut = typename tensor_from_pack<
				T,
				typename pack_concat<
				typename pack_pop_last<dims_pack<L...>>::type,
				typename pack_pop_first<dims_pack<R...>>::type
				>::type
				>::type
			>
			constexpr auto contract_last_first(
				const Tensor<T, L...>& lhs,
				const Tensor<T, R...>& rhs
			) {
				static_assert(sizeof...(L) >= 1 && sizeof...(R) >= 1, "rank must be >= 1 on both sides");

				// Длины «свёрнутых» осей (последняя слева и первая справа) должны совпадать.
				// Берём их из compile-time массивов с размерами пакетов.
				constexpr std::array<std::size_t, sizeof...(L)> LExt{ L... };
				constexpr std::array<std::size_t, sizeof...(R)> RExt{ R... };
				constexpr std::size_t Kleft = LExt[sizeof...(L) - 1];
				constexpr std::size_t Kright = RExt[0];
				static_assert(Kleft == Kright, "contracted dimensions must be equal");

				TOut out{};
				out.Fill(static_cast<T>(0));

				// Вспом. функция: раскодировать линейный индекс -> многомерный индекс результата.
				const auto unflatten = [](
					std::size_t linear
					) {
						std::array<std::size_t, TOut::kRank> idx{};
						for (std::size_t d = TOut::kRank; d > 0; --d) {
							const std::size_t dim = TOut::kExtents[d - 1];
							idx[d - 1] = linear % dim;
							linear /= dim;
						}
						return idx;
					};

				// Главный цикл по всем позициям результата
				for (std::size_t lin = 0; lin < TOut::kSize; ++lin) {
					const auto idxOut = unflatten(lin);

					// Суммирование по контрактуемой оси k
					T acc = static_cast<T>(0);

					for (std::size_t k = 0; k < Kleft; ++k) {
						// idxL: [ i0, i1, ..., i_{m-2}, k ]
						std::array<std::size_t, sizeof...(L)> idxL{};
						{
							// m = ранг lhs, n = ранг rhs
							constexpr std::size_t m = sizeof...(L);
							constexpr std::size_t n = sizeof...(R);
							static_assert(m >= 1 && n >= 1);

							// сколько координат lhs переносим из начала idxOut
							constexpr std::size_t leftChunk = (m > 0 ? m - 1 : 0);

							// первые (m-1) координат берём из начала idxOut
							for (std::size_t i = 0; i < leftChunk; ++i) {
								idxL[i] = idxOut[i];
							}

							// последняя — k (позиция m-1)
							idxL[leftChunk] = k; // т.к. idxL.size() == m
						}

						// idxR: [ k, j1, j2, ..., j_{n-1} ]
						std::array<std::size_t, sizeof...(R)> idxR{};
						{
							idxR[0] = k;

							// остальное берём из конца idxOut
							// в idxOut сначала идут (m-1) координат lhs, затем (n-1) координат rhs
							constexpr std::size_t m = sizeof...(L);
							constexpr std::size_t n = sizeof...(R);
							static_assert(m >= 1 && n >= 1);

							constexpr std::size_t base = m - 1;     // где в idxOut начинаются координаты rhs
							constexpr std::size_t rhsCount = n - 1; // сколько координат rhs кладём
							constexpr std::size_t outCount = (m - 1) + (n - 1);
							static_assert(outCount == TOut::kRank);

							// идём параллельно по j и позиции в idxOut
							for (std::size_t j = 1, pos = base; j < n; ++j, ++pos) {
								idxR[j] = idxOut[pos];
							}
						}

						const auto offL = Tensor<T, L...>::IndexToOffset(idxL);
						const auto offR = Tensor<T, R...>::IndexToOffset(idxR);

						acc += lhs.Data()[offL] * rhs.Data()[offR];
					}

					out.Data()[lin] = acc;
				}

				return out;
			}
		} // namespace details


		//
		// ░ Operators
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		// ░ Tensor multiplication
		//
		template <typename T, std::size_t... L, std::size_t... R>
		__requires_expr(
			(sizeof...(L) != 1 || sizeof...(R) != 1) && // Исключаем "вектор × вектор", чтобы не конкурировать с dot-версией.
			details::ContractibleDimsPacks<details::dims_pack<L...>, details::dims_pack<R...>>
		) constexpr auto operator*(
			const Tensor<T, L...>& lhs,
			const Tensor<T, R...>& rhs
			) {
			return details::contract_last_first(lhs, rhs);
		}

		//
		// ░ Dot product for vectors (rank==1): Vec<T,N> * Vec<T,N> -> T
		//
		template <typename T, std::size_t N>
		constexpr T operator*(
			const Tensor<T, N>& lhs,
			const Tensor<T, N>& rhs
			) {
			T acc{ static_cast<T>(0) };
			for (std::size_t i = 0; i < Tensor<T, N>::kSize; ++i) {
				acc += lhs.Data()[i] * rhs.Data()[i];
			}
			return acc;
		}


		//
		// ░ Elementwise operators
		//
		template <typename T, std::size_t... Dims>
		constexpr Tensor<T, Dims...> operator+(
			const Tensor<T, Dims...>& lhs,
			const Tensor<T, Dims...>& rhs
			) {
			Tensor<T, Dims...> out{};
			for (std::size_t i = 0; i < Tensor<T, Dims...>::kSize; ++i) {
				out.Data()[i] = lhs.Data()[i] + rhs.Data()[i];
			}
			return out;
		}

		template <typename T, std::size_t... Dims>
		constexpr Tensor<T, Dims...>& operator+=(
			Tensor<T, Dims...>& lhs,
			const Tensor<T, Dims...>& rhs
			) {
			for (std::size_t i = 0; i < Tensor<T, Dims...>::kSize; ++i) {
				lhs.Data()[i] += rhs.Data()[i];
			}
			return lhs;
		}

		template <typename T, std::size_t... Dims>
		constexpr Tensor<T, Dims...> operator-(
			const Tensor<T, Dims...>& lhs,
			const Tensor<T, Dims...>& rhs
			) {
			Tensor<T, Dims...> out{};
			for (std::size_t i = 0; i < Tensor<T, Dims...>::kSize; ++i) {
				out.Data()[i] = lhs.Data()[i] - rhs.Data()[i];
			}
			return out;
		}

		template <typename T, std::size_t... Dims>
		constexpr Tensor<T, Dims...>& operator-=(
			Tensor<T, Dims...>& lhs,
			const Tensor<T, Dims...>& rhs
			) {
			for (std::size_t i = 0; i < Tensor<T, Dims...>::kSize; ++i) {
				lhs.Data()[i] -= rhs.Data()[i];
			}
			return lhs;
		}

		//
		// ░ Scalar multiplication
		//
		template <typename T, std::size_t... Dims, typename S>
		__requires_expr(
			std::is_convertible_v<S, T>
		) constexpr Tensor<T, Dims...> operator*(
			const Tensor<T, Dims...>& lhs,
			S scalar
			) {
			Tensor<T, Dims...> out{};
			for (std::size_t i = 0; i < Tensor<T, Dims...>::kSize; ++i) {
				out.Data()[i] = lhs.Data()[i] * static_cast<T>(scalar);
			}
			return out;
		}

		template <typename T, std::size_t... Dims, typename S>
		__requires_expr(
			std::is_convertible_v<S, T>
		) constexpr Tensor<T, Dims...> operator*(
			S scalar,
			const Tensor<T, Dims...>& rhs
			) {
			return rhs * scalar;
		}

		template <typename T, std::size_t... Dims, typename S>
		__requires_expr(
			std::is_convertible_v<S, T>
		) constexpr Tensor<T, Dims...>& operator*=(
			Tensor<T, Dims...>& lhs,
			S scalar
			) {
			for (std::size_t i = 0; i < Tensor<T, Dims...>::kSize; ++i) {
				lhs.Data()[i] *= static_cast<T>(scalar);
			}
			return lhs;
		}


#if HELPERS_ENABLE_COMPILETIME_TESTS == 0
		//
		// ░ Tests
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		namespace TensorTests {
			namespace concepts {
				// Проверка наличия оператора перемнжения тензоров
				template <typename A, typename B>
				concept Multipliable = requires (A a, B b) {
					a* b;
				};

				// Проверка наличия element-wise операторов для одинаковых размеров
				template <typename A, typename B>
				concept Addable = requires (A a, B b) {
					a + b;
					a - b;
					a += b;
					a -= b;
				};

				// Проверка наличия скалярного умножения (оба порядка) и *=
				template <typename A, typename S>
				concept ScalarMultipliable = requires (A a, S s) {
					a* s;
					s* a;
					a *= s;
				};
			}

			consteval void TestMeta() {
				using T = Tensor<int, 2, 3, 4>;

				static_assert(T::Rank() == 3);
				static_assert(T::Size() == 24);
				static_assert(T::Strides()[0] == 12);
				static_assert(T::Strides()[1] == 4);
				static_assert(T::Strides()[2] == 1);
				static_assert(T::IndexToOffset(1, 2, 3) == 23);
			}

			consteval void TestVectorMul() {
				// Vec*Vec must use dot-product operator*
				static_assert(std::same_as<
					decltype(std::declval<Vec<double, 4>>() * std::declval<Vec<double, 4>>()),
					double
				>);

				// Mat*Vec must use universal operator*
				static_assert(std::same_as<
					decltype(std::declval<Mat<float, 2, 3>>() * std::declval<Vec<float, 3>>()),
					Vec<float, 2>
				>);

				// Vec*Mat must use universal operator*
				static_assert(std::same_as<
					decltype(std::declval<Vec<float, 2>>() * std::declval<Mat<float, 2, 3>>()),
					Vec<float, 3>
				>);

				constexpr Vec<int, 3> a{ 1, 2, 3 };
				constexpr Vec<int, 3> b{ 4, 5, 6 };

				static_assert(a * b == 32);
			}

			consteval void TestMatrixMul() {
				using MatA = Tensor<int, 2, 3>;
				using MatB = Tensor<int, 3, 2>;
				using MatC = Tensor<int, 2, 2>;

				constexpr MatA matA{
					1, 2, 0,
					0, 1, 3
				};
				constexpr MatB matB{
					1, 0,
					2, 1,
					1, 2
				};
				constexpr auto matC = matA * matB;
				constexpr MatC matC_exp{
					5, 2,
					5, 7,
				};

				static_assert(matC == matC_exp);
				static_assert(matC[1][1] == matC_exp.Data()[MatC::IndexToOffset(1, 1)]);

				using M2x3 = Mat<int, 2, 3>;
				using M3x4 = Mat<int, 3, 4>;
				using M4x5 = Mat<int, 4, 5>;

				static_assert(concepts::Multipliable<M2x3, M3x4>);
				static_assert(!concepts::Multipliable<M2x3, M4x5>);
			}

			consteval void TestElementwise() {
				using T = Tensor<int, 2, 3>;
				using M2x3 = Mat<int, 2, 3>;
				using M3x2 = Mat<int, 3, 2>;

				// Доступность/недоступность операторов
				static_assert(concepts::Addable<M2x3, M2x3>);
				static_assert(!concepts::Addable<M2x3, M3x2>);

				static_assert(concepts::ScalarMultipliable<M2x3, int>);
				static_assert(concepts::ScalarMultipliable<M2x3, double>);

				// 1) Возврат ссылки для rank==1 (non-const и const)
				static_assert(
					std::is_lvalue_reference_v<decltype(std::declval<Vec3f&>()[0])>,
					"operator[] on rank-1 must return T&"
					);
				static_assert(
					std::is_lvalue_reference_v<decltype(std::declval<const Vec3f&>()[0])>,
					"operator[] on const rank-1 must return const T&"
					);
				static_assert(
					std::is_const_v<std::remove_reference_t<decltype(std::declval<const Vec3f&>()[0])>>,
					"const Vec: operator[] element must be const"
					);

				// Данные для проверки
				constexpr T a{
					1, 2, 3,
					4, 5, 6
				};
				constexpr T b{
					6, 5, 4,
					3, 2, 1
				};

				// Ожидаемые результаты
				constexpr T sum_exp{
					7, 7, 7,
					7, 7, 7
				};
				constexpr T diff_exp{
					-5, -3, -1,
					1, 3, 5
				};
				constexpr T mul3_exp{
					3, 6, 9,
					12, 15, 18
				};

				// +, -
				{
					constexpr auto sum = a + b;
					constexpr auto diff = a - b;

					static_assert(sum == sum_exp);
					static_assert(diff == diff_exp);
				}

				// +=, -= (мутация в constexpr-контексте допустима, т.к. операторы constexpr)
				{
					constexpr auto check_plus_assign = [=]() {
						auto x = a;
						x += b;
						return x == sum_exp;
						};
					static_assert(check_plus_assign());

					constexpr auto check_minus_assign = [=]() {
						auto x = a;
						x -= b;
						return x == diff_exp;
						};
					static_assert(check_minus_assign());

					//constexpr auto add_assign_ok = []() {
					//	Vec3f a{
					//		1.0,
					//		2.0,
					//		3.0
					//	};
					//	Vec3f b{
					//		0.5,
					//		0.5,
					//		0.5
					//	};

					//	a[1] += b[1]; // должно компилироваться и менять a[1]
					//	return a[1] == 2.5;
					//	};
					//static_assert(add_assign_ok(), "a[i] += b[i] must work for rank-1 tensor.");
				}

				// Скалярное умножение (оба порядка) и *=
				{
					// tensor * scalar
					constexpr auto p1 = a * 3;
					static_assert(p1 == mul3_exp);

					// scalar * tensor (зеркальный)
					constexpr auto p2 = 3 * a;
					static_assert(p2 == mul3_exp);

					// *= scalar
					constexpr auto check_mul_assign = [=]() {
						auto x = a;
						x *= 3;
						return x == mul3_exp;
						};
					static_assert(check_mul_assign());

					// Скаляр, конвертируемый к T (double -> int)
					constexpr auto p3 = a * 3.0;
					static_assert(p3 == mul3_exp);
				}
			}

			consteval void TestAll() {
				TestMeta();
				TestVectorMul();
				TestMatrixMul();
				TestElementwise();
			}

			constexpr int __tensor_tests_anchor = (TestAll(), 0);
		}
#endif // HELPERS_ENABLE_COMPILETIME_TESTS
	}
}
#endif