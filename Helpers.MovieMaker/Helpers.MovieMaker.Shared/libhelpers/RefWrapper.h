#pragma once
#include "config.h"

#if HAVE_WINRT == 1
template<class T>
struct RefWrapper {
	T ^obj;

	RefWrapper() {}
	RefWrapper(T ^obj)
		: obj(obj)
	{}
};
#endif // HAVE_WINRT == 1
