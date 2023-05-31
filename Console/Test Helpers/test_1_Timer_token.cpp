//#include <Windows.h>
//#include <functional>
//#include <memory>
//#include <thread>
//#include <mutex>
//#include "../../Shared/Helpers/Time.h"
//
//
//class DesktopManager {
//public:
//	DesktopManager() {
//		auto th = std::thread([this] {
//			Sleep(500);
//			DesktopChangedHandler();
//			Sleep(500);
//			DesktopChangedHandler();
//			Sleep(500);
//			DesktopChangedHandler();
//			});
//		th.join();
//	}
//
//private:
//	void DesktopChangedHandler() {
//		//H::Timer::Once(100ms, [this] {
//		//	++printerNumbers;
//		//	printf("Desktop changed handler = %d \n", printerNumbers.load());
//		//	Sleep(500);
//		//	--printerNumbers;
//		//	});
//		
//		++printerNumbers;
//		auto taskSet = H::Timer::Once(token, 100ms, [this, localIndex = printerNumbers.load()] {
//			printf("Desktop changed handler [captured number =  %d] \n", localIndex);
//			Sleep(300);
//			});
//
//		if (!taskSet) {
//			int xx = 9;
//		}
//	}
//
//private:
//	std::shared_ptr<std::function<void()>> token;
//	std::atomic<int> printerNumbers = 0;
//};
//
//void main() {
//	DesktopManager manager;
//
//	Sleep(6'000);
//	return;
//}