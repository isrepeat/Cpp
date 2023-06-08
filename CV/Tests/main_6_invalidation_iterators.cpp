#include <iostream>
#include <vector>

void main() {
	std::vector<int> vec;
	vec.reserve(3);
	vec.push_back(111);
	vec.push_back(222);
	vec.push_back(333);

	auto it = vec.begin();
	std::cout << "element [0] = " << *it;

	vec.push_back(444);
	std::cout << "element [0] = " << *it;

	return;
}