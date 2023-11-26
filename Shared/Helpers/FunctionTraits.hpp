#pragma once
#include <tuple>
#include <type_traits>

template <typename ReturnType, typename ClassType, typename... Args>
struct _FunctionTraitsBase {
    enum { arity = sizeof...(Args) };
    // arity is the number of arguments.

    using Ret = ReturnType;
    using Class = ClassType;

    template <size_t i>
    struct arg {
        using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
        // the i-th argument is equivalent to the i-th tuple element of a tuple
        // composed of those arguments.
    };
};

// Matches when T=lambda or T=Functor
// For generic types, directly use the result of the signature of its 'operator()'
template <typename T>
struct FunctionTraits : public FunctionTraits<decltype(&T::operator())>
{};

// For Functor L-value or R-value
template <typename R, typename C, typename... A>
struct FunctionTraits<R(C::*)(A...)> : public _FunctionTraitsBase<R, C, A...>
{};

// For lambdas (need const)
template <typename R, typename C, typename... A>
struct FunctionTraits<R(C::*)(A...) const> : public _FunctionTraitsBase<R, C, A...>
{};

// For C-style functions
template <typename R, typename... A>
struct FunctionTraits<R(*)(A...)> : public _FunctionTraitsBase<R, void, A...>
{};