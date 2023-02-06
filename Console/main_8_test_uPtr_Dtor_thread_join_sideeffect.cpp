#include <Windows.h>
#include <functional>
#include <thread>
#include <Memory>
#include <future>


class Temp {
public:
	Temp(int d = 17) : data{ d } {
		printf("Temp() start");
		Sleep(1);
		printf("Temp() end");
	}

	~Temp() {
		printf("~Temp() start");
		Sleep(1);
		printf("~Temp() end");
	}

private:
	int data;
};



int main() {


	std::unique_ptr<Temp> uPtr;

	uPtr = std::make_unique<Temp>(1488);

	Sleep(10);

	uPtr = std::make_unique<Temp>(911);

	//auto th = std::thread([] {
	//	throw std::exception("hello");

	//	return;
	//	});

	//auto future = std::async(std::launch::async, [] {
	//	OutputDebugStringA("111");
	//	});

	//Sleep(10);

	//if (future.valid()) {
	//	Sleep(10);
	//}


	while (1) {
		Sleep(100);
	}
	return 0;
}
