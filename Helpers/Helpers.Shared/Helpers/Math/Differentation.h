#pragma once
#include "Helpers/common.h"
#include "Helpers/Meta/Concepts.h"
#include "Tensor.h"

#if _HAS_CXX20
#include <type_traits>
#include <stdexcept>
#include <utility>

namespace HELPERS_NS {
	namespace Math {
		namespace details {
			template <class T>
			struct is_tensor : std::false_type {};

			template <typename T, std::size_t... Dims>
			struct is_tensor<HELPERS_NS::Math::Tensor<T, Dims...>> : std::true_type {
				using value_type = T;
			};

			template <class T>
			inline constexpr bool is_tensor_v = is_tensor<std::remove_cv_t<std::remove_reference_t<T>>>::value;

			//template <class T>
			//using tensor_value_t = typename is_tensor<std::remove_cv_t<std::remove_reference_t<T>>>::value_type;
		} // namespace details

		
		template <typename T>
		struct DifferentialVar;

		//
		// ░ DifferentialVar -> Tensor
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		template <typename T, std::size_t... Dims>
		struct DifferentialVar<HELPERS_NS::Math::Tensor<T, Dims...>> {
			using Tensor_t = HELPERS_NS::Math::Tensor<T, Dims...>;
			using value_type = typename Tensor_t::value_type;

			Tensor_t value{};
			Tensor_t grad{};

			constexpr void Reset() {
				this->value.Fill(value_type{ 0 });
				this->grad.Fill(value_type{ 0 });
			}

			constexpr void ResetGrad() {
				this->grad.Fill(value_type{ 0 });
			}

			constexpr void AccumulateGrad(const Tensor_t& g) {
				this->grad += g;
			}
		};


		//
		// ░ DifferentialVar -> Scalar
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		template <typename T>
		__requires_expr(
			// По умолчанию разрешим любой арифметический тип как скаляр.
			std::is_arithmetic_v<std::remove_cvref_t<T>>
		) struct DifferentialVar<T> {
			using value_type = std::remove_cvref_t<T>;

			value_type value{};
			value_type grad{};

			constexpr void Reset() {
				this->value = value_type{ 0 };
				this->grad = value_type{ 0 };
			}

			constexpr void ResetGrad() {
				this->grad = value_type{ 0 };
			}

			constexpr void AccumulateGrad(value_type g) {
				this->grad += g;
			}
		};


		//
		// ░ Aliases
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		template <typename T>
		using DifferentialScalar = DifferentialVar<T>;

		template <typename T, std::size_t N>
		using DifferentialVector = DifferentialVar<H::Math::Tensor<T, N>>;

		template <typename T, std::size_t R, std::size_t C>
		using DifferentialMatrix = DifferentialVar<H::Math::Tensor<T, R, C>>;

		template <typename T, std::size_t... Dims>
		using DifferentialTensor = DifferentialVar<H::Math::Tensor<T, Dims...>>;


#if HELPERS_ENABLE_COMPILETIME_TESTS // == 0
		//
		// ░ Tests
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		namespace DifferentationTests {
			consteval void TestMeta() {
			}

			consteval void TestAll() {
				TestMeta();
			}

			constexpr int __differentation_tests_anchor = (TestAll(), 0);
		}
#endif // HELPERS_ENABLE_COMPILETIME_TESTS
	}
}
#endif