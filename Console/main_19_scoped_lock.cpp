//#include <Windows.h>
//#include <thread>
//#include <mutex>
//#include <string>
//#include <vector>
//
//
////std::string strData = "Data: ";
//std::vector<int> myData;
//
//std::mutex mxData;
//
//std::unique_lock<std::mutex> LockScoped() {
//	std::unique_lock<std::mutex> lk{ mxData };
//	return lk;
//}
//
//
//int main() {
//
//	auto th = std::thread([]() {
//		while (true) {
//			//std::lock_guard<std::mutex> lk{ mxData };
//
//			auto lk = LockScoped();
//
//			auto size = myData.size();
//			int aaa = 9;
//
//			for (int i = 0; i < size; i++) {
//				myData[i];
//			}
//
//
//			int xxx = 9;
//		}
//		});
//
//
//	int n = 0;
//	while (true) {
//		//std::lock_guard<std::mutex> lk{ mxData };
//		auto lk = LockScoped();
//
//		myData.clear();
//		myData.push_back(1);
//		myData.push_back(2);
//		myData.push_back(3);
//		myData.push_back(4);
//		myData.push_back(5);
//		myData.push_back(6);
//		myData.push_back(7);
//		myData.push_back(8);
//	};
//
//	th.join();
//	return 0;
//}
