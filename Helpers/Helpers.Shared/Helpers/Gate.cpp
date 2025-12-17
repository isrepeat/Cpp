#include "Gate.h"

namespace HELPERS_NS {
	Gate::Gate() {
		Lock();
	}

	void Gate::Lock() {
		gotResult = false;
	}

	void Gate::Wait() {
		std::unique_lock<std::mutex> lk{ mx };
		cv.wait(lk, [this] { return gotResult.load(); });
	}

	void Gate::Notify() {
		gotResult = true;
		cv.notify_all();
	}
}