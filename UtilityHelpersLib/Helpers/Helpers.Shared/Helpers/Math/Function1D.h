#pragma once
#include "Helpers/common.h"
#include "Helpers/Meta/Concepts.h"

#if _HAS_CXX20
#include <unordered_map>
#include <type_traits>
#include <functional>
#include <stdexcept>
#include <utility>
#include <any>

namespace HELPERS_NS {
	namespace Math {
		enum class FnForm {
			Src,
			Derivative1,
			Derivative2,
			Primitive1,
			Primitive2,
		};

		//
		// ░ Function1D
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		class Function1D {
		public:
			using MetaData_t = std::any;
			using Fn_t = std::function<double(double, MetaData_t&)>;

		private:
			template <int N, typename TDerivativeFn>
			struct Derivative {
				static_assert(N >= 1, "Derivative order must be >= 1.");
				Fn_t fn;

				explicit Derivative(TDerivativeFn&& fn)
					// Конвертируем любой callable в std::function.
					// Здесь можно передать как rvalue-лямбду, так и std::function.
					: fn{ std::forward<TDerivativeFn>(fn) } {
				}
			};

			template <int N, typename TPrimitiveFn>
			struct Primitive {
				static_assert(N >= 1, "Primitive order must be >= 1.");
				Fn_t fn;

				explicit Primitive(TPrimitiveFn&& fn)
					: fn{ std::forward<TPrimitiveFn>(fn) } {
				}
			};

			struct traits {
				// f(x) -> double
				template <typename TFn>
				struct is_invocable_plain_func : std::bool_constant<
					std::is_invocable_r_v<double, TFn&, double>
				> {
				};

				// f(x, meta) -> double
				template <typename TFn>
				struct is_invocable_meta_func : std::bool_constant<
					std::is_invocable_r_v<double, TFn&, double, MetaData_t&>
				> {
				};

				// Trait который будет true если TFn это Derivative / Primitive.
				template <typename TFn>
				struct is_derivative_or_primitive_form : std::false_type {};

				template <int N, typename TFn>
				struct is_derivative_or_primitive_form<Derivative<N, TFn>> : std::true_type {};

				template <int N, typename TFn>
				struct is_derivative_or_primitive_form<Primitive<N, TFn>> : std::true_type {};
			};

			// std::decay_t<TFn> нужен, чтобы нормализовать тип:
			// - убрать ссылки и cv-квалификаторы (если передадут lvalue-лямбду или const std::function&),
			// - превратить массивы/функции в указатели.
			// Это гарантирует, что шаблон Derivative/Primitive хранит "чистый" копируемый тип,
			// а не ссылку на временный объект.

			// f(x) -> f(x, meta)
			template <typename TFn>
			__requires_expr(
				std::is_invocable_r_v<double, std::decay_t<TFn>, double>
			) static Fn_t MakeMetaWrapperFn(TFn&& fn) {
				auto fn_decayed = std::decay_t<TFn>{ std::forward<TFn>(fn) };
				return [fn_decayed](double x, MetaData_t& /*metaData*/) {
					return fn_decayed(x);
					};
			}

			// Нормализация callable -> Fn_t
			template <typename TFn>
			static Fn_t ToFn(TFn&& fn) {
				using TFn_decayed_t = std::decay_t<TFn>;

				if constexpr (std::is_same_v<TFn_decayed_t, Fn_t>) {
					return std::forward<TFn>(fn);
				}
				else if constexpr (traits::is_invocable_meta_func<TFn_decayed_t>::value) {
					return Fn_t{ std::forward<TFn>(fn) };
				}
				else {
					static_assert(
						traits::is_invocable_plain_func<TFn_decayed_t>::value,
						"'fn' must be double(double) or double(double, MetaData_t&)."
						);
					return Function1D::MakeMetaWrapperFn(std::forward<TFn>(fn));
				}
			}

			Function1D(Fn_t srcFunction)
				: srcFunction{ std::move(srcFunction) } {
			}

		public:
			template <int N, typename TFn>
			__requires_expr(
				traits::is_invocable_meta_func<std::remove_reference_t<TFn>>::value
			) static Derivative<N, Fn_t> MakeDerivative(TFn&& fn) {
				return Derivative<N, Fn_t>{ Fn_t{ std::forward<TFn>(fn) } };
			}

			template <int N, typename TFn>
			__requires_expr(
				traits::is_invocable_plain_func<std::remove_reference_t<TFn>>::value
			) static Derivative<N, Fn_t> MakeDerivative(TFn&& fn) {
				return Derivative<N, Fn_t>{ Function1D::MakeMetaWrapperFn(std::forward<TFn>(fn)) };
			}

			template <int N, typename TFn>
			__requires_expr(
				traits::is_invocable_meta_func<std::remove_reference_t<TFn>>::value
			) static Primitive<N, Fn_t> MakePrimitive(TFn&& fn) {
				return Primitive<N, Fn_t>{ Fn_t{ std::forward<TFn>(fn) } };
			}

			template <int N, typename TFn>
			__requires_expr(
				traits::is_invocable_plain_func<std::remove_reference_t<TFn>>::value
			) static Primitive<N, Fn_t> MakePrimitive(TFn&& fn) {
				return Primitive<N, Fn_t>{ Function1D::MakeMetaWrapperFn(std::forward<TFn>(fn)) };
			}

			// Builder с requires: принимает только формы (Derivative<N> / Primitive<N>).
			template <typename TSrcFn, typename... TFunctionForms>
			__requires_expr(
				traits::is_derivative_or_primitive_form<std::decay_t<TFunctionForms>>::value && ...
			) static Function1D Make(
				TSrcFn&& srcFunction,
				TFunctionForms&&... functionForms
			) {
				Function1D fn{ Function1D::ToFn(std::forward<TSrcFn>(srcFunction)) };
				(fn.AddFunctionForm(std::forward<TFunctionForms>(functionForms)), ...);
				return fn;
			}


			template <int N, typename TFn>
			__requires_expr(
				traits::is_invocable_meta_func<std::remove_reference_t<TFn>>::value ||
				traits::is_invocable_plain_func<std::remove_reference_t<TFn>>::value
			) Function1D& SetDerivative(TFn&& derivativeFn) {
				static_assert(N >= 1, "Derivative order must be >= 1.");
				this->derivativesMap[N] = Function1D::ToFn(std::forward<TFn>(derivativeFn));
				return *this;
			}

			template <int N, typename TFn>
			__requires_expr(
				traits::is_invocable_meta_func<std::remove_reference_t<TFn>>::value ||
				traits::is_invocable_plain_func<std::remove_reference_t<TFn>>::value
			) Function1D& SetPrimitive(TFn&& primitiveFn) {
				static_assert(N >= 1, "Primitive order must be >= 1.");
				this->primitivesMap[N] = Function1D::ToFn(std::forward<TFn>(primitiveFn));
				return *this;
			}

			template <int N>
			bool HasDerivative() const {
				static_assert(N >= 1, "Derivative order must be >= 1.");
				return this->derivativesMap.find(N) != this->derivativesMap.end();
			}

			template <int N>
			bool HasPrimitive() const {
				static_assert(N >= 1, "Primitive order must be >= 1.");
				return this->primitivesMap.find(N) != this->primitivesMap.end();
			}

			//
			// Вызов с мета данными
			//
			double Invoke(double x, MetaData_t& metaData) const {
				return this->srcFunction(x, metaData);
			}

			template <int N>
			double InvokeDerivative(double x, MetaData_t& metaData) const {
				static_assert(N >= 1, "Derivative order must be >= 1.");
				const auto it = this->derivativesMap.find(N);

				if (it == this->derivativesMap.end()) {
					throw std::logic_error{ "Derivative<N> is not set." };
				}
				return it->second(x, metaData);
			}

			template <int N>
			double InvokePrimitive(double x, MetaData_t& metaData) const {
				static_assert(N >= 1, "Primitive order must be >= 1.");
				const auto it = this->primitivesMap.find(N);

				if (it == this->primitivesMap.end()) {
					throw std::logic_error{ "Primitive<N> is not set." };
				}
				return it->second(x, metaData);
			}

			//
			// Вызов без мета данных
			//
			double Invoke(double x) const {
				MetaData_t metaData{};
				return this->Invoke(x, metaData);
			}

			template <int N>
			double InvokeDerivative(double x) const {
				MetaData_t metaData{};
				return this->InvokeDerivative<N>(x, metaData);
			}

			template <int N>
			double InvokePrimitive(double x) const {
				MetaData_t metaData{};
				return this->InvokePrimitive<N>(x, metaData);
			}


			class CallProxy {
			public:
				explicit CallProxy(
					const Function1D& functionOwner,
					const FnForm fnForm
				)
					: functionOwner{ &functionOwner }
					, fnForm{ fnForm } {
				}

				double operator()(double x, MetaData_t& metaData) const {
					switch (this->fnForm) {
					case FnForm::Src:			return this->functionOwner->Invoke(x, metaData);
					case FnForm::Derivative1:	return this->functionOwner->InvokeDerivative<1>(x, metaData);
					case FnForm::Derivative2:	return this->functionOwner->InvokeDerivative<2>(x, metaData);
					case FnForm::Primitive1:	return this->functionOwner->InvokePrimitive<1>(x, metaData);
					case FnForm::Primitive2:	return this->functionOwner->InvokePrimitive<2>(x, metaData);
					default:					throw std::logic_error("could not find fnForm");
					};
				}

				double operator()(double x) const {
					MetaData_t metaData{};
					return this->operator()(x, metaData);
				}

			private:
				const Function1D* functionOwner;
				FnForm fnForm;
			}; // class CallProxy


			double operator()(double x, MetaData_t& metaData) const {
				return this->Invoke(x, metaData);
			}

			double operator()(double x) const {
				return this->Invoke(x);
			}

			CallProxy operator[](const FnForm fnForm) const {
				return Function1D::CallProxy{ *this, fnForm };
			}

		private:
			template <int N, typename TDerivative>
			void AddFunctionForm(Derivative<N, TDerivative>&& derivative) {
				this->SetDerivative<N>(std::move(derivative.fn));
			}

			template <int N, typename TPrimitive>
			void AddFunctionForm(Primitive<N, TPrimitive>&& primitive) {
				this->SetPrimitive<N>(std::move(primitive.fn));
			}

		private:
			Fn_t srcFunction;
			std::unordered_map<int, Fn_t> derivativesMap;
			std::unordered_map<int, Fn_t> primitivesMap;
		};


#if HELPERS_ENABLE_COMPILETIME_TESTS // == 0
		//
		// ░ Tests
		// ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ 
		//
		namespace Function1DTests {
			consteval void TestMeta() {
			}

			consteval void TestAll() {
				TestMeta();
			}

			constexpr int __function1D_tests_anchor = (TestAll(), 0);
		}
#endif // HELPERS_ENABLE_COMPILETIME_TESTS
	}
}
#endif