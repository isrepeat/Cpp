#pragma once
#pragma message(PREPROCESSOR_MSG("Deprecated 'FunctionTraits.hpp'"))
#include "common.h"
#include "Meta/Tags.h"
#include <tuple>

//namespace HELPERS_NS {
//    namespace details {
//        template <bool If, typename T, T ThenValue, T ElseValue>
//        struct IF_value {
//            static constexpr T value = ThenValue;
//        };
//
//        template <typename T, T ThenValue, T ElseValue>
//        struct IF_value<false, T, ThenValue, ElseValue> {
//            static constexpr T value = ElseValue;
//        };
//    }
//
//    enum class FuncKind {
//        Lambda, // for lambdas, std::function and custom Functor with "operator() const"
//        Functor, // for custom Functor with non const "operator()"
//        CstyleFunc,
//        ClassMember
//    };
//
//    template <typename ReturnType, typename... Args>
//    struct _FunctionTraitsBaseArgs {
//        enum {
//            ArgumentCount = sizeof...(Args)
//        };
//
//        using Ret = ReturnType;
//        using Arguments = std::tuple<Args...>;
//
//        template <size_t i>
//        struct arg {
//            using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
//            // the i-th argument is equivalent to the i-th tuple element of a tuple
//            // composed of those arguments.
//        };
//
//        template <size_t i>
//        struct arg_val {
//            using type = std::remove_pointer_t<std::remove_reference_t<typename arg<i>::type>>;
//        };
//    };
//
//
//    template <typename ReturnType, typename ClassType, typename... Args>
//    struct _FunctionTraitsBase : _FunctionTraitsBaseArgs<ReturnType, Args...> {
//        using Class = ClassType;
//        using Function = ReturnType(Class::*) (Args...);
//        static constexpr bool IsPointerToMemberFunction = true;
//    };
//
//    template <typename ReturnType, typename... Args>
//    struct _FunctionTraitsBase<ReturnType, void, Args...> : _FunctionTraitsBaseArgs<ReturnType, Args...> {
//        using Class = void;
//        using Function = ReturnType(*)(Args...);
//        static constexpr bool IsPointerToMemberFunction = false;
//    };
//
//
//    // Matches when T=lambda or T=Functor
//    // For generic types, directly use the result of the signature of its 'operator()'
//    template <typename T, bool IsGeneric = false>
//    struct FunctionTraits : public FunctionTraits<decltype(&T::operator()), true> 
//    {};
//
//    // For Functor L-value or R-value with non const "operator()"
//    template <typename R, typename C, typename... A, bool IsGeneric>
//    struct FunctionTraits<R(C::*)(A...), IsGeneric> : public _FunctionTraitsBase<R, C, A...> {
//        static constexpr FuncKind Kind = details::IF_value<IsGeneric, FuncKind, FuncKind::Functor, FuncKind::ClassMember>::value;
//    };
//
//    // For lambdas or Functor with "operator() const"
//    template <typename R, typename C, typename... A, bool IsGeneric>
//    struct FunctionTraits<R(C::*)(A...) const, IsGeneric> : public _FunctionTraitsBase<R, C, A...> {
//        static constexpr FuncKind Kind = details::IF_value<IsGeneric, FuncKind, FuncKind::Lambda, FuncKind::ClassMember>::value;
//    };
//
//    // For C-style functions
//    template <typename R, typename... A, bool IsGeneric>
//    struct FunctionTraits<R(*)(A...), IsGeneric> : public _FunctionTraitsBase<R, void, A...> {
//        static constexpr FuncKind Kind = FuncKind::CstyleFunc;
//    };
//
//#ifdef __CLR__
//    // For C-style functions CLR
//    template <typename R, typename... A, bool IsGeneric>
//    struct FunctionTraits<R(__clrcall*)(A...)> : public _FunctionTraitsBase<R, void, A...> {
//        static constexpr FuncKind Kind = FuncKind::CstyleFunc;
//    };
//#endif
//
//    template <typename Fn>
//    static constexpr bool IsLambda = FunctionTraits<Fn>::Kind == FuncKind::Lambda;
//    
//    template <typename Fn>
//    static constexpr bool IsFunctor = FunctionTraits<Fn>::Kind == FuncKind::Functor;
//
//    template <typename Fn>
//    static constexpr bool IsCStyleFn = FunctionTraits<Fn>::Kind == FuncKind::CstyleFunc;
//
//    template <typename Fn>
//    static constexpr bool IsClassFn = FunctionTraits<Fn>::Kind == FuncKind::ClassMember;
//
//
//    template <typename T>
//    struct Result {
//        using type = T;
//    };
//
//    template <>
//    struct Result<void> {
//        using type = nothing;
//    };
//
//    template <typename T>
//    using Result_t = typename Result<T>::type;
//
//    template<typename Fn, typename... Args>
//    Result_t<typename FunctionTraits<Fn>::Ret> InvokeHelper(Fn fn, Args&&... args) {
//        if constexpr (std::is_same_v<typename FunctionTraits<Fn>::Ret, void>) {
//            fn(std::forward<Args&&>(args)...);
//            return Result_t<void>{};
//        }
//        else {
//            return fn(std::forward<Args&&>(args)...);
//        }
//    }
//
//    template <typename T, size_t i>
//    using FunctionArgT = typename FunctionTraits<T>::template arg<i>::type;
//
//    template <typename T, size_t i>
//    using FunctionArgValT = typename FunctionTraits<T>::template arg_val<i>::type;
//
//
//#if __cpp_concepts
//	namespace concepts {
//		template<typename Fn>
//		concept HasMethod =
//			H::FunctionTraits<Fn>::Kind == H::FuncKind::ClassMember &&
//			H::FunctionTraits<Fn>::IsPointerToMemberFunction == true;
//
//		template<typename Fn, typename Signature>
//		concept HasMethodWithSignature =
//			H::FunctionTraits<Fn>::Kind == H::FuncKind::ClassMember &&
//			H::FunctionTraits<Fn>::IsPointerToMemberFunction == true &&
//			std::is_same_v<typename H::FunctionTraits<Fn>::Function, Signature>;
//
//		template<typename Fn>
//		concept HasStaticMethod =
//			H::FunctionTraits<Fn>::IsPointerToMemberFunction == false;
//
//		template<typename Fn, typename Signature>
//		concept HasStaticMethodWithSignature =
//			H::FunctionTraits<Fn>::IsPointerToMemberFunction == false &&
//			std::is_same_v<typename H::FunctionTraits<Fn>::Function, Signature>;
//	}
//#endif
//}