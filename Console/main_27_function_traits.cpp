#include <Helpers/Meta/FunctionTraits.h>
#include <type_traits>
#include <functional>
#include <utility>
#include <tuple>


namespace test {
	using namespace H::meta;

	template <typename T, typename U>
	constexpr bool same = std::is_same_v<T, U>;

	namespace Free {
		long Fn(int, float) {
			return 1;
		}

		long FnNoexcept(int, float) noexcept {
			return 2;
		}

		struct Class {
			static int FnStaticMember(int, double) noexcept {
				return 0;
			}
		};
	} // namespace Free


	static_assert(same<FunctionTraits<decltype(&Free::Fn)>::Ret, long>);
	static_assert(FunctionTraits<decltype(&Free::Fn)>::ArgumentCount == 2);
	static_assert(same<FunctionTraits<decltype(&Free::Fn)>::arg<0>::type, int>);
	static_assert(same<FunctionTraits<decltype(&Free::Fn)>::arg<1>::type, float>);
	static_assert(FunctionTraits<decltype(&Free::Fn)>::Kind == FuncKind::CstyleFunc);
	static_assert(FunctionTraits<decltype(&Free::Fn)>::IsNoexcept == false);
	static_assert(FunctionTraits<decltype(&Free::Fn)>::IsConst == false);
	static_assert(FunctionTraits<decltype(&Free::Fn)>::Ref == RefQual::None);

	static_assert(same<FunctionTraits<decltype(&Free::FnNoexcept)>::Ret, long>);
	static_assert(FunctionTraits<decltype(&Free::FnNoexcept)>::ArgumentCount == 2);
	static_assert(FunctionTraits<decltype(&Free::FnNoexcept)>::Kind == FuncKind::CstyleFunc);
	static_assert(FunctionTraits<decltype(&Free::FnNoexcept)>::IsNoexcept == true);

	static_assert(same<FunctionTraits<decltype(&Free::Class::FnStaticMember)>::Ret, int>);
	static_assert(FunctionTraits<decltype(&Free::Class::FnStaticMember)>::Kind == FuncKind::CstyleFunc);
	static_assert(FunctionTraits<decltype(&Free::Class::FnStaticMember)>::IsNoexcept == true);


	namespace Member {
		struct Quals {
			// cv / noexcept 
			int Fn(double) { return 0; }
			int FnConst(double) const { return 0; }
			int FnNoexcept(double) noexcept { return 0; }
			int FnConstNoexcept(double) const noexcept { return 0; }

			// cv / ref / noexcept
			int FnLRef(double)& { return 0; }
			int FnRRef(double)&& { return 0; }
			int FnConstLRef(double) const& { return 0; }
			int FnConstRRef(double) const&& { return 0; }
			int FnLRefNoexcept(double) & noexcept { return 0; }
			int FnRRefNoexcept(double) && noexcept { return 0; }
			int FnConstLRefNoexcept(double) const& noexcept { return 0; }
			int FnConstRRefNoexcept(double) const&& noexcept { return 0; }
		};
	} // namespace Member


	static_assert(same<FunctionTraits<decltype(&Member::Quals::Fn)>::Class, Member::Quals>);
	static_assert(same<FunctionTraits<decltype(&Member::Quals::Fn)>::Signature, int(Member::Quals::*)(double)>);
	static_assert(FunctionTraits<decltype(&Member::Quals::Fn)>::Kind == FuncKind::ClassMember);
	static_assert(FunctionTraits<decltype(&Member::Quals::Fn)>::Ref == RefQual::None);
	static_assert(FunctionTraits<decltype(&Member::Quals::Fn)>::IsConst == false);
	static_assert(FunctionTraits<decltype(&Member::Quals::Fn)>::IsNoexcept == false);

	static_assert(FunctionTraits<decltype(&Member::Quals::FnConst)>::IsConst == true);
	static_assert(FunctionTraits<decltype(&Member::Quals::FnConst)>::Ref == RefQual::None);

	static_assert(FunctionTraits<decltype(&Member::Quals::FnLRef)>::Ref == RefQual::LValue);
	static_assert(FunctionTraits<decltype(&Member::Quals::FnRRef)>::Ref == RefQual::RValue);

	static_assert(FunctionTraits<decltype(&Member::Quals::FnConstLRef)>::IsConst == true);
	static_assert(FunctionTraits<decltype(&Member::Quals::FnConstLRef)>::Ref == RefQual::LValue);

	static_assert(FunctionTraits<decltype(&Member::Quals::FnConstRRef)>::IsConst == true);
	static_assert(FunctionTraits<decltype(&Member::Quals::FnConstRRef)>::Ref == RefQual::RValue);

	static_assert(FunctionTraits<decltype(&Member::Quals::FnNoexcept)>::IsNoexcept == true);

	static_assert(FunctionTraits<decltype(&Member::Quals::FnConstNoexcept)>::IsConst == true);
	static_assert(FunctionTraits<decltype(&Member::Quals::FnConstNoexcept)>::IsNoexcept == true);
	
	static_assert(FunctionTraits<decltype(&Member::Quals::FnLRefNoexcept)>::Ref == RefQual::LValue);
	static_assert(FunctionTraits<decltype(&Member::Quals::FnLRefNoexcept)>::IsNoexcept == true);

	static_assert(FunctionTraits<decltype(&Member::Quals::FnRRefNoexcept)>::Ref == RefQual::RValue);
	static_assert(FunctionTraits<decltype(&Member::Quals::FnRRefNoexcept)>::IsNoexcept == true);

	static_assert(FunctionTraits<decltype(&Member::Quals::FnConstLRefNoexcept)>::Ref == RefQual::LValue);
	static_assert(FunctionTraits<decltype(&Member::Quals::FnConstLRefNoexcept)>::IsConst == true);
	static_assert(FunctionTraits<decltype(&Member::Quals::FnConstLRefNoexcept)>::IsNoexcept == true);

	static_assert(same<
		FunctionTraits<decltype(&Member::Quals::FnConstRRefNoexcept)>::Signature,
		int(Member::Quals::*)(double) const && noexcept
	>);
	static_assert(FunctionTraits<decltype(&Member::Quals::FnConstRRefNoexcept)>::Ref == RefQual::RValue);
	static_assert(FunctionTraits<decltype(&Member::Quals::FnConstRRefNoexcept)>::IsConst == true);
	static_assert(FunctionTraits<decltype(&Member::Quals::FnConstRRefNoexcept)>::IsNoexcept == true);
	

	namespace Callable {
		//
		// Callable-типы с одиночным operator(): Functor vs Lambda, ref-квалификаторы
		//
		struct Functor {
			int operator()(char) { return 1; }
		};

		struct FunctorConst {
			int operator()(char) const { return 1; }
		};

		// union как callable Ч корректно, HasNonOverloadedCallOp пропустит
		union UnionConst {
			int operator()(char) const { return 1; }
		};

		// NOTE: 
		// ” этого класса несколько operator(), поэтому нельз€ использовать FunctionTraits<FunctorRefQual>
		// напр€мую, нужно €вно указать, какую именно перегрузку мы хотим, через FunctionTraitsWithSignature.
		struct FunctorOverloadedQual {
			int operator()(char) { return 1; }
			int operator()(char) const { return 2; }
			int operator()(char) noexcept { return 3; }
			int operator()(char) const noexcept { return 4; }
		};

		struct FunctorOverloadedRefQual {
			int operator()(char)& { return 1; }
			int operator()(char)&& { return 2; }
			int operator()(char) const& { return 3; }
			int operator()(char) const&& { return 4; }
			int operator()(char) & noexcept { return 5; }
			int operator()(char) && noexcept { return 6; }
			int operator()(char) const& noexcept { return 7; }
			int operator()(char) const&& noexcept { return 8; }
		};

		using Sig = int(char);
		using SigConst = int(char) const;
		using SigNoexcept = int(char) noexcept;
		using SigConstNoexcept = int(char) const noexcept;

		using SigLRef = int(char)&;
		using SigRRef = int(char)&&;
		using SigConstLRef = int(char) const&;
		using SigConstRRef = int(char) const&&;
		using SigLRefNoexcept = int(char) & noexcept;
		using SigRRefNoexcept = int(char) && noexcept;
		using SigConstLRefNoexcept = int(char) const& noexcept;
		using SigConstRRefNoexcept = int(char) const&& noexcept;

		// Ћ€мбды (без/с захватами) Ч это просто анонимные callable-типы.
		constexpr auto Lambda = [](short, long) noexcept -> int { return 0; };
		auto LambdaWithCapture = [x = 42](short, long) -> int { return x; };
	} // namespace Callable


	static_assert(same<FunctionTraits<Callable::Functor>::Ret, int>);
	static_assert(FunctionTraits<Callable::Functor>::ArgumentCount == 1);
	static_assert(FunctionTraits<Callable::Functor>::Kind == FuncKind::Functor);
	static_assert(FunctionTraits<Callable::Functor>::Ref == RefQual::None);
	static_assert(FunctionTraits<Callable::Functor>::IsConst == false);

	static_assert(same<FunctionTraits<Callable::FunctorConst>::Ret, int>);
	static_assert(FunctionTraits<Callable::FunctorConst>::ArgumentCount == 1);
	static_assert(FunctionTraits<Callable::FunctorConst>::Kind == FuncKind::Functor);
	static_assert(FunctionTraits<Callable::FunctorConst>::IsConst == true);

	static_assert(same<FunctionTraits<Callable::UnionConst>::Ret, int>);
	static_assert(FunctionTraits<Callable::UnionConst>::ArgumentCount == 1);
	static_assert(FunctionTraits<Callable::UnionConst>::Kind == FuncKind::Functor);
	static_assert(FunctionTraits<Callable::UnionConst>::IsConst == true);

	// ” std::function ровно один operator() с нужной сигнатурой Ч он должен анализироватьс€ как callable-тип:
	using StdFn = std::function<void(int, float)>;

	static_assert(FunctionTraits<StdFn>::Kind == FuncKind::Functor);
	static_assert(same<FunctionTraits<StdFn>::Ret, void>);
	static_assert(FunctionTraits<StdFn>::ArgumentCount == 2);

	// NOTE: FunctionTraitsWithSignature маршрутизирует в pointer-to-member, значит Kind будет ClassMember
	static_assert(FunctionTraitsWithSignature<Callable::FunctorOverloadedQual, Callable::Sig>::Kind == FuncKind::ClassMember);
	static_assert(FunctionTraitsWithSignature<Callable::FunctorOverloadedQual, Callable::Sig>::Ref == RefQual::None);
	static_assert(FunctionTraitsWithSignature<Callable::FunctorOverloadedQual, Callable::Sig>::IsConst == false);
	static_assert(FunctionTraitsWithSignature<Callable::FunctorOverloadedQual, Callable::Sig>::IsNoexcept == false);

	static_assert(same<
		FunctionTraitsWithSignature<Callable::FunctorOverloadedRefQual, Callable::SigConstRRefNoexcept>::Class,
		Callable::FunctorOverloadedRefQual
	>);
	static_assert(same<
		FunctionTraitsWithSignature<Callable::FunctorOverloadedRefQual, Callable::SigConstRRefNoexcept>::Signature,
		int(Callable::FunctorOverloadedRefQual::*)(char) const&& noexcept
	>);
	static_assert(FunctionTraitsWithSignature<Callable::FunctorOverloadedRefQual, Callable::SigConstRRefNoexcept>::Ref == RefQual::RValue);
	static_assert(FunctionTraitsWithSignature<Callable::FunctorOverloadedRefQual, Callable::SigConstRRefNoexcept>::IsConst == true);
	static_assert(FunctionTraitsWithSignature<Callable::FunctorOverloadedRefQual, Callable::SigConstRRefNoexcept>::IsNoexcept == true);

	static_assert(same<FunctionTraits<decltype(Callable::Lambda)>::Ret, int>);
	static_assert(FunctionTraits<decltype(Callable::Lambda)>::ArgumentCount == 2);
	static_assert(FunctionTraits<decltype(Callable::Lambda)>::Kind == FuncKind::Functor);
	static_assert(FunctionTraits<decltype(Callable::Lambda)>::IsNoexcept == true);

	static_assert(same<FunctionTraits<decltype(Callable::LambdaWithCapture)>::Ret, int>);
	static_assert(FunctionTraits<decltype(Callable::LambdaWithCapture)>::Kind == FuncKind::Functor);

	//
	// arg<i>::type и arg_val<i>::type
	//
	int FreePtrRef(int*&, const double*);

	static_assert(same<FunctionTraits<decltype(&FreePtrRef)>::arg<0>::type, int*&>);
	static_assert(same<FunctionTraits<decltype(&FreePtrRef)>::arg<1>::type, const double*>);
	static_assert(same<FunctionTraits<decltype(&FreePtrRef)>::arg_val<0>::type, int>); // remove_ref + remove_ptr
	static_assert(same<FunctionTraits<decltype(&FreePtrRef)>::arg_val<1>::type, const double>);

	//
	// Concepts
	//
	using CStyleFn = decltype(&Free::Fn);
	using MemberFn = decltype(&Member::Quals::FnConstRRefNoexcept);
	using CallableFn = Callable::FunctorConst;

	static_assert(concepts::has_static_function<CStyleFn>);
	static_assert(!concepts::has_method<CStyleFn>);

	static_assert(concepts::has_method<MemberFn>);
	static_assert(!concepts::has_static_function<MemberFn>);

	// ƒл€ callable-типа с одиночным operator() Ч HasMethod == false, это именно callable-класс:
	static_assert(!concepts::has_method<CallableFn>);
	static_assert(!concepts::has_static_function<CallableFn>);

	// Has*WithSignature Ч провер€ют точное совпадение Signature:
	static_assert(concepts::has_static_function_with_signature<CStyleFn, long (*)(int, float)>);
	static_assert(!concepts::has_static_function_with_signature<CStyleFn, long (*)(float, int)>);

	// ƒл€ member pointer сигнатуры:
	static_assert(concepts::has_method_with_signature<MemberFn, int (Member::Quals::*)(double) const&& noexcept > );

} // namespace test


int main() {
	return 0;
}