#include <type_traits>
#include <iostream>
#include <functional>
#include <vector>
#include <type_traits>
#include "../Shared/Helpers/FunctionTraits.h"



//template <typename T>
//struct function_traits
//{
//    typedef typename function_traits<decltype(&T::operator())>::return_type return_type;
//};
//// For generic types, directly use the result of the signature of its 'operator()'


//template <typename T>
//struct function_traits // matches when T=X or T=lambda
//// As expected, lambda creates a "unique, unnamed, non-union class type" 
//// so it matches here
//{
//    // Here is what you are looking for. The type of the member operator()
//    // of the lambda is taken and mapped again on function_traits.
//    typedef typename function_traits<decltype(&T::operator())>::return_type return_type;
//};


template <typename ReturnType, typename ClassType, typename... Args>
struct function_traits_base {
    enum { arity = sizeof...(Args) };
    // arity is the number of arguments.

    typedef ReturnType return_type;

    typedef ClassType class_type;

    template <size_t i>
    struct arg
    {
        typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
        // the i-th argument is equivalent to the i-th tuple element of a tuple
        // composed of those arguments.
    };
};

// Matches when T=lambda or T=Functor
// For generic types, directly use the result of the signature of its 'operator()'
template <typename T> 
struct function_traits : public function_traits<decltype(&T::operator())> 
{};


// For Functor L-value or R-value
template <typename R, typename C, typename... A>
struct function_traits<R(C::*)(A...)> : public function_traits_base<R, C, A...>
{};

// For lambdas
template <typename R, typename C, typename... A>
struct function_traits<R(C::*)(A...) const> : public function_traits_base<R, C, A...>
{};

// For C-style functions
template <typename R, typename... A>
struct function_traits<R(*)(A...)> : public function_traits_base<R, void, A...>
{};




//// matches for X::operator() but not of lambda::operator()
//template <typename R, typename C, typename... A>
//struct function_traits<R(C::*)(A...)>
//{
//    typedef R return_type;
//};

//// I initially thought the above defined member function specialization of 
//// the trait will match lambdas::operator() because a lambda is a functor.
//// It does not, however. Instead, it matches the one below.
//// I wonder why? implementation defined?
//template <typename R, typename... A>
//struct function_traits<R(*)(A...)> // matches for lambda::operator() 
//{
//    typedef R return_type;
//};




struct Functor {
    double operator() (int i) {
        return 17.0;
    }
};


long CStyleFunc(int i) {
    return 77;
};

template <typename Fn>
typename function_traits<Fn>::return_type Foo(Fn callback) {
    return callback(8);
}

int main() {

    int abcd_efgh = 9;

    Functor functor;
    std::function<long(int)> stdFunc = [abcd_efgh](int i) { return long(i * 10); };
    auto labmda = [](int i) { return long(i * 10); };
    auto labmdaWithCapture = [abcd_efgh](int i) { return long(i * 10); };


    //auto res = Foo([]() {
    //    return 17.0f;
    //    });

    
    auto resCStyleFunc = Foo(&CStyleFunc);
    function_traits<decltype(&CStyleFunc)>::return_type; // NOTE: & - is important
    function_traits<decltype(&CStyleFunc)>::class_type;
    function_traits<decltype(&CStyleFunc)>::arity;
    function_traits<decltype(&CStyleFunc)>::arg<0>::type;

    auto resFunctorLValue = Foo(functor);
    function_traits<decltype(functor)>::return_type;
    function_traits<decltype(functor)>::class_type;
    function_traits<decltype(functor)>::arity;
    function_traits<decltype(functor)>::arg<0>::type;

    auto resFunctorRValue = Foo(Functor{});
    function_traits<decltype(Functor{})>::return_type;
    function_traits<decltype(Functor{})>::class_type;
    function_traits<decltype(Functor{})>::arity;
    function_traits<decltype(Functor{})>::arg<0>::type;

    auto resStdFunc = Foo(stdFunc);
    function_traits<decltype(stdFunc)>::return_type;
    function_traits<decltype(stdFunc)>::class_type;
    function_traits<decltype(stdFunc)>::arity;
    function_traits<decltype(stdFunc)>::arg<0>::type;

    auto resLambda = Foo(labmda);
    function_traits<decltype(labmda)>::return_type;
    function_traits<decltype(labmda)>::class_type;
    function_traits<decltype(labmda)>::arity;
    function_traits<decltype(labmda)>::arg<0>::type;

    auto resLambdaWithCapture = Foo(labmdaWithCapture);
    function_traits<decltype(labmdaWithCapture)>::return_type;
    function_traits<decltype(labmdaWithCapture)>::class_type;
    function_traits<decltype(labmdaWithCapture)>::arity;
    function_traits<decltype(labmdaWithCapture)>::arg<0>::type;

    return 0;
}