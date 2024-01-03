#include <Windows.h>
#include <iostream>
#include <thread>
#include <future>

int main() {
	//auto th = std::thread([] {
	//	throw std::exception("thread exception");
	//	});

	std::future<void> f;
	if (f.valid()) {
		int xxx = 9;
	}

	f = std::async(std::launch::async, [] {
		Sleep(100);
		throw std::exception("async exception");
		});

	try {
		//th.join();

		if (f.valid()) {
			f.get();
		}

		int xx = 9;
	}
	catch (std::exception ex) {
		auto msg = ex.what();
	}
	catch (...) {
		int xxx = 9;
	}
}