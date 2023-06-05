//#include <Windows.h>
//#include <vector>
//#include <iostream>
//#include <functional>
//#include "../Shared/Helpers/FunctionTraits.h"
//
//
//
//
//long CStyleFunc(int i) {
//    return 77;
//};
//
//struct Functor {
//    double operator() (int i) {
//        return 17.0;
//    }
//};
//
//
//template <typename Fn, typename Rt = typename FunctionTraits<Fn>::Ret>
//Rt GetResult(Fn callback) {
//    if constexpr (std::is_same_v<Rt, void>) {
//        callback(8);
//        return;
//    }
//    else {
//        return callback(8);
//    }
//}
//
//
//class Temp {
//public:
//    int xxx = 1488;
//    std::string sss = "Hello";
//
//public:
//    void Foo() {
//        static const std::function<void()> staticConstLambda = [this] {
//            Sleep(1);
//            if (xxx == 1488) {
//                int xx = 9;
//            }
//            Sleep(1);
//            if (sss == "Hello") {
//                int xx = 9;
//            }
//            Sleep(1);
//        };
//        static std::function<void()> staticLambda = [this] {
//            Sleep(1);
//            if (xxx == 1488) {
//                int xx = 9;
//            }
//            Sleep(1);
//            if (sss == "Hello") {
//                int xx = 9;
//            }
//            Sleep(1);
//        };
//
//        staticConstLambda();
//        staticLambda();
//    }
//};
//
//
////template <typename Fn>
////typename FunctionTraits<Fn>::Ret Invoke( callback) {
////    return callback();
////}
//
//int main() {
//
//    Temp temp;
//
//    temp.xxx = 911;
//    temp.sss = "World";
//
//    temp.Foo();
//
//    int abcd_efgh = 9;
//
//    Functor functor;
//    std::function<void(int)> stdFunc = nullptr;
//    //std::function<void(int)> stdFunc = [abcd_efgh](int i) {
//    //    long(i * 10);
//    //    return;
//    //};
//    auto lambda = [](int i) { 
//        return long(i * 10); 
//    };
//    auto lambdaWithCapture = [abcd_efgh](int i) {
//        return long(i * 10); 
//    };
//
//
//
//    auto resCStyleFunc = GetResult(&CStyleFunc);
//    FunctionTraits<decltype(&CStyleFunc)>::Ret; // NOTE: & - is important
//    FunctionTraits<decltype(&CStyleFunc)>::Class;
//    FunctionTraits<decltype(&CStyleFunc)>::arity;
//    FunctionTraits<decltype(&CStyleFunc)>::arg<0>::type;
//
//    auto resFunctorLValue = GetResult(functor);
//    FunctionTraits<decltype(functor)>::Ret;
//    FunctionTraits<decltype(functor)>::Class;
//    FunctionTraits<decltype(functor)>::arity;
//    FunctionTraits<decltype(functor)>::arg<0>::type;
//
//    auto resFunctorRValue = GetResult(Functor{});
//    FunctionTraits<decltype(Functor{})>::Ret;
//    FunctionTraits<decltype(Functor{})>::Class;
//    FunctionTraits<decltype(Functor{})>::arity;
//    FunctionTraits<decltype(Functor{})>::arg<0>::type;
//
//    GetResult(stdFunc);
//    //Invoke([]() { return 12; });
//    //Invoke([] {});
//
//    //auto resStdFunc = GetResult(stdFunc);
//    //Invoke(stdFunc);
//    FunctionTraits<decltype(stdFunc)>::Ret;
//    FunctionTraits<decltype(stdFunc)>::Class;
//    FunctionTraits<decltype(stdFunc)>::arity;
//    FunctionTraits<decltype(stdFunc)>::arg<0>::type;
//
//    auto resLambda = GetResult(lambda);
//    FunctionTraits<decltype(lambda)>::Ret;
//    FunctionTraits<decltype(lambda)>::Class;
//    FunctionTraits<decltype(lambda)>::arity;
//    FunctionTraits<decltype(lambda)>::arg<0>::type;
//
//    auto resLambdaWithCapture = GetResult(lambdaWithCapture);
//    FunctionTraits<decltype(lambdaWithCapture)>::Ret;
//    FunctionTraits<decltype(lambdaWithCapture)>::Class;
//    FunctionTraits<decltype(lambdaWithCapture)>::arity;
//    FunctionTraits<decltype(lambdaWithCapture)>::arg<0>::type;
//
//    return 0;
//}