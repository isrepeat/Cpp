#include <vector>
#include <iostream>

struct Object {
	Object() {
		std::cout << "Hello \n";
	}
	~Object() {
		std::cout << "Bye \n";
	}
};

void main() {
	{
		std::vector<Object*> items;
		items.push_back(new Object());
		items.push_back(new Object());
		items.push_back(new Object());
	}
	return;
}