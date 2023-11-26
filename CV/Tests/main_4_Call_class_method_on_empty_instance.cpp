#include <iostream>

struct A {
    int data;
    int foo() {
        return 1234; 
    }
};

void main() {
    A* p = nullptr;

    std::cout << p->foo();
    return;
}