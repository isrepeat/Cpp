#include "ObjectLocker.h"

namespace H {
	void ObjectLocker::Lock() {
		if (locked) {
			std::mutex m;
			std::unique_lock<std::mutex> lk(m);
			cvLocker.wait(lk);
		} 
		locked = true;
	}

	void ObjectLocker::Unlock() {
		locked = false;
		cvLocker.notify_all();
	}

	bool ObjectLocker::IsLocked() {
		return locked;
	}

	ObjectLocker::~ObjectLocker() {
		cvLocker.notify_all();
	}
}