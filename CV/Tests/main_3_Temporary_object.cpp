#include <iostream>

void PrintText(const char* str) {
	std::cout << str << std::endl;
}

int main() {
	PrintText(std::string("Hello world").c_str());
	return 0;
}