//#include <type_traits>
//#include <iostream>
//#include <vector>
//
////template<int>
////struct foo {};
////
////template<int arg_N>
////struct val {
////    static constexpr auto N = arg_N;
////};
////
////template<template <int> typename T, int N>
////constexpr val<N> extract(const T<N>&);
////
////template<typename T>
////constexpr auto extract_N = decltype(extract(std::declval<T>()))::N;
//
//
//
//
//template<typename R, int arg_N>
//struct val {
//    static constexpr auto N = arg_N;
//};
//
//template<typename R>
//struct foo : val<R, 911> {
//};
//
////template<template <typename> typename T, typename R, int N>
////constexpr val<R, N> extract(const T<R>&);
//
////template<typename T>
////constexpr auto extract_N = decltype(extract(std::declval<T>()))::N;
//
////template<typename T>
////constexpr auto extract_NN = decltype((val<>)std::declval<T>())::N;
//
//
//
////template <typename T> struct first_param;
////
////template <template <typename, typename...> class C, typename T, typename ...Ts>
////struct first_param<C<T, Ts...>>
////{
////    using type = T;
////};
//
//
//template <typename T> 
//struct first_param {
//    static const int cVal = T::N;
//};
//
////template <template <typename> class C, typename T>
////struct first_param<C<T>>
////{
////    //typedef T type;
////    static const int cVal = C<T>::N;
////};
//
////template <template <typename, typename> class C, typename T, typename T2>
////struct first_param<C<T, T2>>
////{
////    typedef T type;
////};
//
//
////template <template <typename> class C, typename T>
////struct first_param<C<T>>
////{
////    //typedef T type;
////    static const int cVal = C<T>::N;
////};
//
//template<template <typename> class C, typename T>
//constexpr auto extract_N = C<T>::N;
//
//
////struct A {};
////struct B : A {};
////
////A funcA();
////B funcB();
////
////template<typename T>
////using cv = decltype(reinterpret_cast<T(*)()>(&funcB));
////
////template <typename T>
////struct convert_to {
////};
////
////template<template <typename, int> class Base, typename T, int N>
////struct convert_to<Base<T, N>> {
////};
//
//
//int main() {
//    first_param<foo<int>>::cVal;
//
//    //cv<A>;
//
//    //cv<int>;
//
//    //extract_N<foo>;
//
//    //test(foo<int>{});
//
//    //testFunc(reinterpret_cast<A(*)()>(&funcB));
//
//    //extract_N<foo<float>>;
//    
//    //constexpr int N = decltype(std::declval<foo<float>>());
//
//
//    //const foo<float>& ref = std::declval<foo<float>>();
//
//    //std::cout << extract_N<foo<5>>;
//    return 0;
//}