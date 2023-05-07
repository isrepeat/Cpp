//#include <Windows.h>
//#include <functional>
//#include <thread>
//#include <Memory>
//#include <future>
//
//std::future<void> routine;
//std::future<void> routine2;
//
//int main() {
//	{
//		routine = std::async(std::launch::async, [] {
//			OutputDebugStringA("111 \n");
//			Sleep(1000);
//			OutputDebugStringA("222 \n");
//			Sleep(1000);
//			OutputDebugStringA("333 \n");
//			Sleep(1000);
//			OutputDebugStringA("444 \n");
//			Sleep(1000);
//			OutputDebugStringA("555 \n");
//			Sleep(1000);
//			OutputDebugStringA("666 \n");
//			Sleep(1000);
//			OutputDebugStringA("777 \n");
//			Sleep(1000);
//			OutputDebugStringA("888 \n");
//			Sleep(1000);
//			OutputDebugStringA("999 \n");
//			Sleep(1000);
//
//			int xxx = 9;
//			});
//	}
//
//	Sleep(2200);
//
//	{
//		//routine = std::async(std::launch::async, [] { // previous future destructor called which makes the current thread "Wait" for async task finish
//		routine2 = std::async(std::launch::async, [] {
//			OutputDebugStringA("aaa \n");
//			Sleep(1000);
//			OutputDebugStringA("bbb \n");
//			Sleep(1000);
//			OutputDebugStringA("ccc \n");
//			Sleep(1000);
//
//			int xxx = 9;
//			});
//	}
//
//	while (1) {
//		Sleep(100);
//	}
//	return 0;
//}
