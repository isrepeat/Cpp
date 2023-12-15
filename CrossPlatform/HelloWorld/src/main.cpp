#include "PlatformProvider.h"
#include <functional>
#include <iostream>

int main() {
	PlatformProvider platformProvider;
	std::cout << "Hello " << platformProvider.GetOsName() << "!" << std::endl;
	return 0;
}