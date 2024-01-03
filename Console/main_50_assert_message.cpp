#include <cassert>

#ifdef _DEBUG
#define assertm(expression, message) (void)(												                          \
            (!!(expression)) ||                                                                                       \
            (_wassert(L" " message L"  {" _CRT_WIDE(#expression) L"}", _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \
        )
#else
#define assertm(expression, message) ((void)0)
#endif


constexpr bool expression = 2 > 3;

constexpr void foo() {
	return;
}

constexpr void bar() {
	return;
}


int main() {
	assertm(2 > 3);
	//constexpr bool res = !(!!(expression) || (foo(), false) || (bar(), false));
	return 0;
}