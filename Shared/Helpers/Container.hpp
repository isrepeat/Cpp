#pragma once
#include <deque>

namespace H {
    template<class T, class ContainerT>
    class fixed_container : private ContainerT {
    public:
        using Base = ContainerT;
        using Base::Base;

        using Base::size;
        using Base::begin;
        using Base::end;
        using Base::operator[];

        fixed_container(ContainerT&& other)
            : ContainerT(std::move(other))
        {}
    };


    template<class T>
    using fixed_deque = fixed_container<T, std::deque<T>>;
}