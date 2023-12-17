#include <functional>
#include <iostream>
#include <thread>

int main() {
	std::cout << "Hello Linux!" << std::endl;

	for (int i = 1; i <= 5; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1'000));
		std::cout << i << std::endl;
	}
	return 0;
}