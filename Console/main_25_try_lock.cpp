#include <Windows.h>
#include <thread>
#include <mutex>

void main() {
	std::mutex mx;

	auto th1 = std::thread([&mx] {
		Sleep(300);
		if (auto lock = std::unique_lock{ mx, std::try_to_lock }) {
			Sleep(2000);
			printf("thread 1 - owned mx \n");
		}
		Sleep(1);
		});

	auto th2 = std::thread([&mx] {
		Sleep(200);
		if (auto lock = std::unique_lock{ mx, std::try_to_lock }) {
			Sleep(2000);
			printf("thread 2 - owned mx \n");
		}
		Sleep(1);
		});

	th1.join();
	th2.join();

	return;
}