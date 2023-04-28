#include <iostream>

class MyContainer {
public:
	MyContainer(int data)
		: data{new int(data)} 
	{};

	~MyContainer() {
		delete data;
	}

	int* GetData() {
		return data;
	}

private:
	int* data;
};


void PrintNumber(int* number) {
	std::cout << *number << std::endl;
}


int main() {
	PrintNumber(MyContainer(1234).GetData());
	return 0;
}