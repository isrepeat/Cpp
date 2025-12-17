#pragma once
#include "common.h"
#include <functional>
#include <cassert>
#include <memory>

namespace HELPERS_NS {
    // https://stackoverflow.com/questions/22822836/type-switch-construct-in-c11

    template<typename T> struct remove_class { };
    template<typename C, typename R, typename... A>
    struct remove_class<R(C::*)(A...)> { using type = R(A...); };
    template<typename C, typename R, typename... A>
    struct remove_class<R(C::*)(A...) const> { using type = R(A...); };
    template<typename C, typename R, typename... A>
    struct remove_class<R(C::*)(A...) volatile> { using type = R(A...); };
    template<typename C, typename R, typename... A>
    struct remove_class<R(C::*)(A...) const volatile> { using type = R(A...); };

    template<typename T>
    struct get_signature_impl {
        using type = typename remove_class<
            decltype(&std::remove_reference<T>::type::operator())>::type;
    };
    template<typename R, typename... A>
    struct get_signature_impl<R(A...)> { using type = R(A...); };
    template<typename R, typename... A>
    struct get_signature_impl<R(&)(A...)> { using type = R(A...); };
    template<typename R, typename... A>
    struct get_signature_impl<R(*)(A...)> { using type = R(A...); };
    template<typename T> using get_signature = typename get_signature_impl<T>::type;


    namespace details {
        template<typename Base, typename T>
        bool TypeCase(Base* base, std::function<void(T&)> typeHandler) {
            try {
                typeHandler(dynamic_cast<T&>(*base));
                return true;
            }
            catch (std::bad_cast&) {
                return false;
            }
        }
    }

    template<typename Base>
    bool TypeSwitch(Base*) {
        return false;
    }

    template<typename Base, typename FirstSubclassHandler, typename... RestSubclassesHandlers>
    bool TypeSwitch(Base* base, FirstSubclassHandler&& firstHandler, RestSubclassesHandlers&&... restHandlers) {
        using Signature = get_signature<FirstSubclassHandler>;
        using Function = std::function<Signature>;

        if (details::TypeCase(base, (Function)firstHandler)) {
            return true;
        }
        else {
            return TypeSwitch(base, restHandlers...);
        }
    }
}