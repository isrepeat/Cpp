#include <iostream>

int counter = 0;

struct Object {
	Object() {
		counter++;
	}
	~Object() {
		counter++;
	}
};

Object Func() {
	Object object;
	return object;
}

void main() {
	{
		Object res;
		res = Func();
	}
	std::cout << counter;
	return;
}