#pragma once
#include <Helpers/common.h>
#include <Helpers/Mutex.h>
#include <memory>

namespace HELPERS_NS {
    namespace Com {
		// NOTE: mutex internal implementation may vary for different dlls 
		//       so use 'virtual' to safe call lock / unlock in the dll where the mutex was created.
		struct Mutex {
			Mutex(std::unique_ptr<HELPERS_NS::IMutex> mx)
				: mx{ std::move(mx) }
			{}

			virtual void lock() {
				this->mx->lock();
			}
			virtual void unlock() {
				this->mx->unlock();
			}

		private:
			std::unique_ptr<HELPERS_NS::IMutex> mx;
		};
    }
}