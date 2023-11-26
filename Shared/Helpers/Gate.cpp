#include "Gate.h"

namespace H {
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


//namespace H {
//	CvEvent::CvEvent()
//		: gotResult{ false }
//	{}
//
//	CvEvent::~CvEvent() {
//		Notify();
//	}
//
//	//void CvEvent::Lock() {
//	//	gotResult = false;
//	//}
//
//	//void CvEvent::Wait() {
//	//	std::unique_lock<std::mutex> lk{ mx };
//	//	cv.wait(lk, [this] { return gotResult.load(); });
//	//}
//
//	// If was notify before call wait and result already 
//	void CvEvent::Wait(std::unique_lock<std::mutex>& lk, std::function<bool()> Pred) {
//		cv.wait(lk, [&] {
//			if (Pred) {
//				gotResult = Pred();
//			}
//			return gotResult.load();
//			});
//	}
//
//	void CvEvent::Notify() {
//		gotResult = true;
//		cv.notify_all();
//	}
//}