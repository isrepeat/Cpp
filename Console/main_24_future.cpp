#include <Windows.h>
#include <functional>
#include <thread>
#include <Memory>
#include <future>

std::future<void> routine;
std::future<void> routine2;

int main() {
	{
		routine = std::async(std::launch::async, [] {
			OutputDebugStringA("111 \n");
			Sleep(200);
			//throw 111;
			OutputDebugStringA("222 \n");
			Sleep(500);
			//OutputDebugStringA("333 \n");
			//Sleep(100);
			//OutputDebugStringA("444 \n");
			//Sleep(100);
			//OutputDebugStringA("555 \n");
			//Sleep(100);
			//OutputDebugStringA("666 \n");
			//Sleep(100);
			//OutputDebugStringA("777 \n");
			//Sleep(100);
			//OutputDebugStringA("888 \n");
			//Sleep(100);
			//OutputDebugStringA("999 \n");
			//Sleep(100);

			int xxx = 9;
			});
	}

	//Sleep(2200);

	// explicitly wait to ensure that previous future lambda finished
	if (routine.valid())
		routine.get();

	{
														// NOTE: Previous future destructor called  which makes the current thread "Wait" for async task finish.
		routine = std::async(std::launch::async, [] {	//		 Destructor was called after init new future, so may case when 2 lambdas executes										  
		//routine2 = std::async(std::launch::async, [] {
			OutputDebugStringA("aaa \n");
			Sleep(1000);
			OutputDebugStringA("bbb \n");
			if (routine.valid()) {
				if (routine.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
					int xx = 9;

					try {
						routine.get();
					}
					catch (...) {
						int err = 666;
					}
				}
			}
			Sleep(100);
			OutputDebugStringA("ccc \n");
			Sleep(100);

			int xxx = 9;
			});
	}

	while (1) {
		Sleep(100);
	}
	return 0;
}
