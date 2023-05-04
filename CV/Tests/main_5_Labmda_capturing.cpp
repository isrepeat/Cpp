#include <iostream>
#include <functional>

std::function<int()> CreatePrintLambda()  {
	int value = 123;

	std::function<int()> lambda = [&value] () {
		return value;
	};

	return lambda;
}

void main()  {
	auto printLambda = CreatePrintLambda();
	std::cout << printLambda();
	return;
};