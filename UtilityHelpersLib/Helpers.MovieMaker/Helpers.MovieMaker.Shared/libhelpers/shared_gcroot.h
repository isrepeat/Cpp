#pragma once

#include <memory>
#include <msclr\gcroot.h>

template<class T>
struct shared_gcroot {
    typedef std::shared_ptr<msclr::gcroot<T>> type;
};

template<class D, class S, class... Args>
typename shared_gcroot<D ^>::type make_shared_gcroot(Args ...args) {
    typename shared_gcroot<D ^>::type res;
    auto tmp = msclr::gcroot<D ^>(gcnew S(args...));

    res = std::make_shared<msclr::gcroot<D ^>>(tmp);

    return res;
}

template<class D, class S>
typename shared_gcroot<D ^>::type wrap_shared_gcroot(S ^arg) {
    typename shared_gcroot<D ^>::type res;
    auto tmp = msclr::gcroot<D ^>(arg);

    res = std::make_shared<msclr::gcroot<D ^>>(tmp);

    return res;
}