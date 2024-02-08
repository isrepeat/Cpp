#include <Windows.h>
#include <vector>
#include <iostream>
#include <functional>
#include <Helpers/FunctionTraits.hpp>




long CStyleFunc(int i, float f = 0.0f) {
    return 77;
};

struct Functor {
    int x;
    double operator() (int i, float f = 0.0f) {
        return 17.0;
    }
};

struct FunctorConst {
    double operator() (int i, float f = 0.0f) const {
        return 17.0;
    }
};

template <typename Fn>
constexpr bool IsCStyleFunc() {
    if constexpr (H::FunctionTraits<Fn>::Kind == H::FuncKind::CstyleFunc) {
        return true;
    }
    return false;
}
template <typename Fn>
constexpr bool IsCStyleFunc_(Fn fn) {
    if constexpr (H::FunctionTraits<Fn>::Kind == H::FuncKind::CstyleFunc) {
        return true;
    }   
    return false;
}

template <typename Fn>
constexpr bool IsClassMember() {
    if constexpr (H::FunctionTraits<Fn>::Kind == H::FuncKind::ClassMember) {
        return true;
    }
    return false;
}
template <typename Fn>
constexpr bool IsClassMember_(Fn fn) {
    if constexpr (H::FunctionTraits<Fn>::Kind == H::FuncKind::ClassMember) {
        return true;
    }
    return false;
}

template <typename Fn>
constexpr bool IsFunctor() {
    if constexpr (H::FunctionTraits<Fn>::Kind == H::FuncKind::Functor) {
        return true;
    }
    return false;
}
template <typename Fn>
constexpr bool IsFunctor_(Fn fn) {
    if constexpr (H::FunctionTraits<Fn>::Kind == H::FuncKind::Functor) {
        return true;
    }
    return false;
}

template <typename Fn>
constexpr bool IsLambda() {
    if constexpr (H::FunctionTraits<Fn>::Kind == H::FuncKind::Lambda) {
        return true;
    }
    return false;
}
template <typename Fn>
constexpr bool IsLambda_(Fn fn) {
    if constexpr (H::FunctionTraits<Fn>::Kind == H::FuncKind::Lambda) {
        return true;
    }
    return false;
}



class Temp {
public:
    int xxx = 1488;
    std::string sss = "Hello";

public:
    float Foo(char) const {
        static const std::function<void()> staticConstLambda = [this] { // this captured once
            Sleep(1);
            if (xxx == 1488) {
                int xx = 9;
            }
            Sleep(1);
            if (sss == "Hello") {
                int xx = 9;
            }
            Sleep(1);
        };
        static std::function<void()> staticLambda = [this] { // this will captured every time
            Sleep(1);
            if (xxx == 1488) {
                int xx = 9;
            }
            Sleep(1);
            if (sss == "Hello") {
                int xx = 9;
            }
            Sleep(1);
        };

        staticConstLambda();
        staticLambda();

        return 3.14f;
    }
};



template <typename Fn, typename Rt = typename H::FunctionTraits<Fn>::Ret>
Rt GetResult(Fn callback) {
    if constexpr (std::is_same_v<Rt, void>) {
        callback(8, 2.0f);
        return;
    }
    return callback(8, 2.0f);
}


std::shared_ptr<double> MyFunc(std::wstring, int) {
    return {};
}


template <typename>
struct Invokation;

template <std::size_t... I>
struct Invokation<std::index_sequence<I...>> {
    template <typename Fn>
    static constexpr int Method(Fn fn) {
        return sizeof...(I);
    }
};

//template <typename Fn>
//constexpr int Invoke(Fn fn) {
//    return Invokation<std::make_index_sequence<H::FunctionTraits<Fn>::ArgumentCount>>::Method(fn);
//}



int main() {

    Temp temp;
    //temp.xxx = 911;
    //temp.sss = "World";
    //temp.Foo('0');

    int localValue = 9;

    Functor functor;
    FunctorConst functorConst;

    //std::function<void(int)> stdFunc = nullptr;
    std::function<void(int, float)> stdFunc = [localValue](int i, float f = 0.0f) {
        long(i * 10);
        return;
    };
    auto lambda = [](int i, float f = 0.0f) { 
        return long(i * 10); 
    };
    auto lambdaWithCapture = [localValue](int i, float f = 0.0f) {
        return long(i * 10); 
    };


    H::FunctionTraits<decltype(&MyFunc)>::Ret;
    H::FunctionTraits<decltype(&MyFunc)>::Function;
    H::FunctionTraits<decltype(&MyFunc)>::Arguments;
    H::FunctionTraits<decltype(&MyFunc)>::ArgumentCount;

    
    //H::FunctionTraits<,>::Ret;
    //H::FunctionTraits<long(*)(int, float)>::Ret;
    //H::FunctionTraits<long(*)(int, float)>::Function;

    ////InvokeInner(&MyFunc, std::wstring{}, 2);
    //InvokeInner(&MyFunc, L"", 2);
    //InvokeInner(&MyFunc, std::wstring{}, 2);
    //InvokeInner(std::make_index_sequence<3>{});
    
    constexpr int res = Invokation<std::make_index_sequence<4>>::Method(&MyFunc);

    auto resCStyleFunc = GetResult(&CStyleFunc);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(&CStyleFunc)>::Ret, long>); // NOTE: & - is important
    //static_assert(std::is_same_v<H::FunctionTraits<decltype(&CStyleFunc)>::Class, void>);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(&CStyleFunc)>::arg<0>::type, int>);
    static_assert(H::FunctionTraits<decltype(&CStyleFunc)>::ArgumentCount == 2);
    static_assert(H::FunctionTraits<decltype(&CStyleFunc)>::Kind == H::FuncKind::CstyleFunc);
    static_assert(H::FunctionTraits<decltype(&CStyleFunc)>::IsPointerToMemberFunction == false);
    static_assert(IsLambda_(&CStyleFunc) == false);
    static_assert(IsFunctor_(&CStyleFunc) == false);
    static_assert(IsCStyleFunc_(&CStyleFunc) == true); // <--
    static_assert(IsClassMember_(&CStyleFunc) == false);
    static_assert(IsLambda<decltype(&CStyleFunc)>() == false);
    static_assert(IsFunctor<decltype(&CStyleFunc)>() == false);
    static_assert(IsCStyleFunc<decltype(&CStyleFunc)>() == true); // <--
    static_assert(IsClassMember<decltype(&CStyleFunc)>() == false);

    static_assert(std::is_same_v<H::FunctionTraits<decltype(&Temp::Foo)>::Ret, float>);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(&Temp::Foo)>::Class, Temp>);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(&Temp::Foo)>::arg<0>::type, char>);
    static_assert(H::FunctionTraits<decltype(&Temp::Foo)>::ArgumentCount == 1);
    static_assert(H::FunctionTraits<decltype(&Temp::Foo)>::Kind == H::FuncKind::ClassMember);
    static_assert(H::FunctionTraits<decltype(&Temp::Foo)>::IsPointerToMemberFunction == true);
    static_assert(IsLambda_(&Temp::Foo) == false);
    static_assert(IsFunctor_(&Temp::Foo) == false);
    static_assert(IsCStyleFunc_(&Temp::Foo) == false);
    static_assert(IsClassMember_(&Temp::Foo) == true); // <--
    static_assert(IsLambda<decltype(&Temp::Foo)>() == false);
    static_assert(IsFunctor<decltype(&Temp::Foo)>() == false);
    static_assert(IsCStyleFunc<decltype(&Temp::Foo)>() == false);
    static_assert(IsClassMember<decltype(&Temp::Foo)>() == true); // <--

    auto resFunctorLValue = GetResult(functor);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(functor)>::Ret, double>);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(functor)>::Class, Functor>);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(functor)>::arg<0>::type, int>);
    static_assert(H::FunctionTraits<decltype(functor)>::ArgumentCount == 2);
    static_assert(H::FunctionTraits<decltype(functor)>::Kind == H::FuncKind::Functor);
    static_assert(H::FunctionTraits<decltype(functor)>::IsPointerToMemberFunction == true);
    static_assert(IsLambda_(functor) == false);
    static_assert(IsFunctor_(functor) == true); // <--, Will fails if Functor class have non-contexpr members 
    static_assert(IsCStyleFunc_(functor) == false);
    static_assert(IsClassMember_(functor) == false);
    static_assert(IsLambda<decltype(functor)>() == false);
    static_assert(IsFunctor<decltype(functor)>() == true); // <--
    static_assert(IsCStyleFunc<decltype(functor)>() == false);
    static_assert(IsClassMember<decltype(functor)>() == false);

    auto resFunctorRValue = GetResult(Functor{});
    static_assert(std::is_same_v<H::FunctionTraits<decltype(Functor{})>::Ret, double>);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(Functor{})>::Class, Functor>);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(Functor{})>::arg<0>::type, int>);
    static_assert(H::FunctionTraits<decltype(Functor{})>::ArgumentCount == 2);
    static_assert(H::FunctionTraits<decltype(Functor{})>::Kind == H::FuncKind::Functor);
    static_assert(H::FunctionTraits<decltype(Functor{})>::IsPointerToMemberFunction == true);
    static_assert(IsLambda_(Functor{}) == false);
    static_assert(IsFunctor_(Functor{}) == true); // <-- Ok, even if Functor have non-constexpr members
    static_assert(IsCStyleFunc_(Functor{}) == false);
    static_assert(IsClassMember_(Functor{}) == false);
    static_assert(IsLambda<decltype(Functor{})>() == false);
    static_assert(IsFunctor<decltype(Functor{})>() == true); // <--
    static_assert(IsCStyleFunc<decltype(Functor{})>() == false);
    static_assert(IsClassMember<decltype(Functor{})>() == false);

    static_assert(std::is_same_v<H::FunctionTraits<decltype(functorConst)>::Ret, double>);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(functorConst)>::Class, FunctorConst>);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(functorConst)>::arg<0>::type, int>);
    static_assert(H::FunctionTraits<decltype(functorConst)>::ArgumentCount == 2);
    static_assert(H::FunctionTraits<decltype(functorConst)>::Kind == H::FuncKind::Lambda);
    static_assert(H::FunctionTraits<decltype(functorConst)>::IsPointerToMemberFunction == true);
    static_assert(IsLambda_(functorConst) == true); // <--, Will fails if Functor class have non-contexpr members 
    static_assert(IsFunctor_(functorConst) == false);
    static_assert(IsCStyleFunc_(functorConst) == false);
    static_assert(IsClassMember_(functorConst) == false);
    static_assert(IsLambda<decltype(functorConst)>() == true); // <--
    static_assert(IsFunctor<decltype(functorConst)>() == false);
    static_assert(IsCStyleFunc<decltype(functorConst)>() == false);
    static_assert(IsClassMember<decltype(functorConst)>() == false);

    static_assert(std::is_same_v < H::FunctionTraits<decltype(FunctorConst{})> ::Ret, double>);
    static_assert(std::is_same_v < H::FunctionTraits<decltype(FunctorConst{})> ::Class, FunctorConst>);
    static_assert(std::is_same_v < H::FunctionTraits<decltype(FunctorConst{})> ::arg<0>::type,int>);
    static_assert(H::FunctionTraits<decltype(FunctorConst{})>::ArgumentCount == 2);
    static_assert(H::FunctionTraits<decltype(FunctorConst{})>::Kind == H::FuncKind::Lambda);
    static_assert(H::FunctionTraits<decltype(FunctorConst{})>::IsPointerToMemberFunction == true);
    static_assert(IsLambda_(FunctorConst{}) == true); // <-- Ok, even if Functor have non-constexpr members
    static_assert(IsFunctor_(FunctorConst{}) == false);
    static_assert(IsCStyleFunc_(FunctorConst{}) == false);
    static_assert(IsClassMember_(FunctorConst{}) == false);
    static_assert(IsLambda<decltype(FunctorConst{}) > () == true); // <--
    static_assert(IsFunctor<decltype(FunctorConst{}) > () == false);
    static_assert(IsCStyleFunc<decltype(FunctorConst{}) > () == false);
    static_assert(IsClassMember<decltype(FunctorConst{}) > () == false);

    GetResult(stdFunc);
    //Invoke([]() { return 12; });
    //Invoke([] {});
    //auto resStdFunc = GetResult(stdFunc);
    //Invoke(stdFunc);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(stdFunc)>::Ret, void>);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(stdFunc)>::Class, std::_Func_class<void, int, float>>);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(stdFunc)>::arg<0>::type, int>);
    static_assert(H::FunctionTraits<decltype(stdFunc)>::ArgumentCount == 2);
    static_assert(H::FunctionTraits<decltype(stdFunc)>::Kind == H::FuncKind::Lambda);
    static_assert(H::FunctionTraits<decltype(stdFunc)>::IsPointerToMemberFunction == true);
    //static_assert(IsLambda_(stdFunc) == true); // Fails because std::function have non-contexpr members
    //static_assert(IsFunctor_(stdFunc) == false);
    //static_assert(IsCStyleFunc_(stdFunc) == false);
    //static_assert(IsClassMember_(stdFunc) == false);
    static_assert(IsLambda<decltype(stdFunc)>() == true); // <--
    static_assert(IsFunctor<decltype(stdFunc)>() == false);
    static_assert(IsCStyleFunc<decltype(stdFunc)>() == false);
    static_assert(IsClassMember<decltype(stdFunc)>() == false);

    auto resLambda = GetResult(lambda);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(lambda)>::Ret, long>);
    //static_assert(std::is_same_v<H::FunctionTraits<decltype(lambda)>::Class, ???>);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(lambda)>::arg<0>::type, int>);
    static_assert(H::FunctionTraits<decltype(lambda)>::ArgumentCount == 2);
    static_assert(H::FunctionTraits<decltype(lambda)>::Kind == H::FuncKind::Lambda);
    static_assert(H::FunctionTraits<decltype(lambda)>::IsPointerToMemberFunction == true);
    static_assert(IsLambda_(lambda) == true); // <--
    static_assert(IsFunctor_(lambda) == false);
    static_assert(IsCStyleFunc_(lambda) == false);
    static_assert(IsClassMember_(lambda) == false);
    static_assert(IsLambda<decltype(lambda)>() == true); // <--
    static_assert(IsFunctor<decltype(lambda)>() == false);
    static_assert(IsCStyleFunc<decltype(lambda)>() == false);
    static_assert(IsClassMember<decltype(lambda)>() == false);

    auto resLambdaWithCapture = GetResult(lambdaWithCapture);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(lambdaWithCapture)>::Ret, long>);
    //static_assert(std::is_same_v<H::FunctionTraits<decltype(lambdaWithCapture)>::Class, ???>);
    static_assert(std::is_same_v<H::FunctionTraits<decltype(lambdaWithCapture)>::arg<0>::type, int>);
    static_assert(H::FunctionTraits<decltype(lambdaWithCapture)>::ArgumentCount == 2);
    static_assert(H::FunctionTraits<decltype(lambdaWithCapture)>::Kind == H::FuncKind::Lambda);
    static_assert(H::FunctionTraits<decltype(lambdaWithCapture)>::IsPointerToMemberFunction == true);
    //static_assert(IsLambda_(lambdaWithCapture) == true); // Fails because lambda have captured values
    //static_assert(IsFunctor_(lambdaWithCapture) == false);
    //static_assert(IsCStyleFunc_(lambdaWithCapture) == false);
    //static_assert(IsClassMember_(lambdaWithCapture) == false);
    static_assert(IsLambda<decltype(lambdaWithCapture)>() == true); // <--
    static_assert(IsFunctor<decltype(lambdaWithCapture)>() == false);
    static_assert(IsCStyleFunc<decltype(lambdaWithCapture)>() == false);
    static_assert(IsClassMember<decltype(lambdaWithCapture)>() == false);
    return 0;
}