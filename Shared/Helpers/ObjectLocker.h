#pragma once
#include <condition_variable>

namespace H {
	class ObjectLocker {
	public:
		ObjectLocker() = default;
		~ObjectLocker();

		void Lock();
		void Unlock();

		bool IsLocked();

	private:
		std::atomic<bool> locked = false;
		std::condition_variable cvLocker;
	};
}