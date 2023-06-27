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

int main() {
	Child child;
	Parent  a = child;
	Parent& b = child;
	Parent* c = &child;

	std::cout << a.GetName() << std::endl;
	std::cout << b.GetName() << std::endl;
	std::cout << c->GetName() << std::endl;
	return 0;
}