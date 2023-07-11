//#include <Windows.h>
//#include <functional>
//#include <memory>
//#include <thread>
//#include <mutex>
//
//void TestWeakAsToken() {
//	std::mutex mx;
//	std::weak_ptr<std::function<void()>> callbackWeakPtr;
//
//	{
//		std::shared_ptr<std::function<void()>> token;
//		{
//			auto sharedPtr = std::make_shared<std::function<void()>>([] {
//				int xx = 9;
//				});
//			callbackWeakPtr = sharedPtr;
//			token = sharedPtr;
//		}
//
//
//		{
//			auto locked = callbackWeakPtr.lock();
//			if (locked) {
//				(*locked)();
//				token = nullptr;
//			}
//		}
//
//		{
//			auto lockedAgain = callbackWeakPtr.lock();
//			if (lockedAgain) {
//				(*lockedAgain)();
//			}
//		}
//	}
//
//	//auto locked = callbackWeakPtr.lock();
//	//if (locked) {
//	//	(*locked)();
//	//}
//}
//
//void TestWeakOnEmptySharedOrEmptyPointer() {
//	std::shared_ptr<std::function<void()>> shPtr1 = nullptr;
//	std::shared_ptr<std::function<void()>> shPtr2 = std::make_shared<std::function<void()>>(nullptr);
//
//	std::weak_ptr<std::function<void()>> weakPtr1 = shPtr1;
//	std::weak_ptr<std::function<void()>> weakPtr2 = shPtr2;
//
//	if (!weakPtr1.expired()) {
//		printf("weakPtr1 not expired \n"); // not enter
//	}
//	if (auto ptr1 = weakPtr1.lock()) {
//		printf("weakPtr1 not empty \n");  // not enter
//	}
//
//	if (!weakPtr2.expired()) {
//		printf("weakPtr2 not expired \n"); // enter
//	}
//	if (auto ptr2 = weakPtr2.lock()) {
//		printf("weakPtr2 not empty \n"); // enter
//		//if (ptr2) { // will be Access violation below
//		if (*ptr2) {
//			(*ptr2)();
//		}
//	}
//}
//
//struct Temp {
//	Temp() {
//		int xx = 9;
//	}
//	~Temp() {
//		int xx = 9;
//	}
//};
//
//template <typename T>
//struct Deleter {
//	void operator() (T* ptr) {
//		delete ptr;
//	}
//};
//
//void TestWeakAndDestuctorShared() {
//	//std::shared_ptr<Temp> shPtr = std::make_shared<Temp>(Deleter<Temp>{});
//
//	std::weak_ptr<Temp> weakPtr;
//	{
//		std::shared_ptr<Temp> shPtr = std::shared_ptr<Temp>(new Temp{}, Deleter<Temp>{});
//		weakPtr = shPtr;
//		Sleep(100);
//	}
//
//	if (weakPtr.expired()) {
//		printf("weakPtr expired \n");
//	}
//	Sleep(100);
//}
//
//void main() {
//	//TestWeakAsToken();
//	//TestWeakOnEmptySharedOrEmptyPointer();
//	TestWeakAndDestuctorShared();
//	return;
//}