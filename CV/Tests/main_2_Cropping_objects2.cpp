#include <iostream>

class Parent {
public:
	virtual std::string GetName() {
		return "Parent";
	};
};

class Child : public Parent {
public:
	std::string GetName() override {
		return "Child";
	};
};


void PrintName1(Parent* object) {
	std::cout << object->GetName() << std::endl;
}

void PrintName2(Parent object) {
	std::cout << object.GetName() << std::endl;
}

int main() {
	Child child;

	PrintName1(&child);
	PrintName2(child);
	return 0;
}