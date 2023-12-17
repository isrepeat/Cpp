#include <functional>
#include <iostream>
#include <thread>

//#include "../Linux_Test.Static/ClassFromStaticLib.h"
#include "../Linux_Test.Dynamic/ClassFromDynamicLib.h"

int main() {
	std::cout << "Hello from Linux_Test.App" << std::endl;

	//ClassFromStaticLib libraryClass;
	ClassFromDynamicLib libraryClass;
	std::cout << "Library print = \"" << libraryClass.Print() << "\"" << std::endl;

	for (int i = 1; i <= 3; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
		std::cout << i << std::endl;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
	return 0;
}