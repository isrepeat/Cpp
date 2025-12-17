#pragma once

#include <memory>
#include <msclr\auto_gcroot.h>

template<class T>
struct shared_auto_gcroot {
	typedef std::shared_ptr<msclr::auto_gcroot<T>> type;
};

template<class D, class S, class... Args>
typename shared_auto_gcroot<D ^>::type make_shared_auto_gcroot(Args ...args) {
	typename shared_auto_gcroot<D ^>::type res;
	auto tmp = msclr::auto_gcroot<D ^>(gcnew S(args...));

	res = std::make_shared<msclr::auto_gcroot<D ^>>(tmp);

	return res;
}

template<class D, class S>
typename shared_auto_gcroot<D ^>::type wrap_shared_auto_gcroot(S ^arg) {
	typename shared_auto_gcroot<D ^>::type res;
	auto tmp = msclr::auto_gcroot<D ^>(arg);

	res = std::make_shared<msclr::auto_gcroot<D ^>>(tmp);

	return res;
}