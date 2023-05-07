#include <Windows.h>
#include <thread>
#include <mutex>
#include <memory>
#include <functional>

void main() {
	std::mutex mx;
	std::weak_ptr<std::function<void()>> callbackWeakPtr;
	
	{
		std::shared_ptr<std::function<void()>> token;
		{
			auto sharedPtr = std::make_shared<std::function<void()>>([] {
				int xx = 9;
				});
			callbackWeakPtr = sharedPtr;
			token = sharedPtr;
		}

		
		{
			auto locked = callbackWeakPtr.lock();
			if (locked) {
				(*locked)();
				token = nullptr;
			}
		}

		{
			auto lockedAgain = callbackWeakPtr.lock();
			if (lockedAgain) {
				(*lockedAgain)();
			}
		}
	}

	//auto locked = callbackWeakPtr.lock();
	//if (locked) {
	//	(*locked)();
	//}

	return;
}