#include "MultithreadMutex.h"
#include <windows.h>
#include <cassert>

namespace HELPERS_NS {
	MultithreadMutex::~MultithreadMutex() {
		assert(!this->is_locked());
	}

	void MultithreadMutex::lock() {
		std::unique_lock lk{ mx };
		cvLocker.wait(lk, [this] {
			return !locked;
			});
		locked = true;
	}

	void MultithreadMutex::unlock() {
		std::unique_lock lk{ mx };
		locked = false;
		cvLocker.notify_all();
	}

	bool MultithreadMutex::is_locked() {
		std::unique_lock lk{ mx };
		return locked;
	}


	MultithreadMutexRecursive::~MultithreadMutexRecursive() {
		assert(!this->is_locked());
	}

	void MultithreadMutexRecursive::lock() {
		std::unique_lock lk{ mx };

		auto currentThreadId = static_cast<long>(::GetCurrentThreadId());
		if (lockedThreadId != currentThreadId) {
			cvLocker.wait(lk, [this] {
				return lockedCount == 0;
				});

			lockedThreadId = currentThreadId;
		}

		++lockedCount;
	}

	void MultithreadMutexRecursive::unlock() {
		std::unique_lock lk{ mx };
		if (--lockedCount == 0) {
			lockedThreadId = -1;
			cvLocker.notify_all();
		}
	}

	bool MultithreadMutexRecursive::is_locked() {
		std::unique_lock lk{ mx };
		return lockedCount > 0;
	}
}