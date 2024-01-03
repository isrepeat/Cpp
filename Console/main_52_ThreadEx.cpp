#include <Helpers/ThreadEx.h>
#include <Helpers/Logger.h>


int main() {
	std::unique_ptr<H::ThreadEx> threadEx;

	std::function exceptionHandler = [&] {
		LOG_WARNING_D("Was exception");
	};

	threadEx = std::make_unique<H::ThreadEx>(exceptionHandler, [] {
		Sleep(2000);
		throw 1111;
		});

	Sleep(1000);

	if (threadEx->joinable())
		threadEx->join();

	threadEx = std::make_unique<H::ThreadEx>(exceptionHandler, [] {
		Sleep(2000);
		throw 3.14f;
		});

	if (threadEx->joinable())
		threadEx->join();

	return 0;
}