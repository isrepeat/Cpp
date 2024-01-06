#include "main_36_ThreadFinishHelper.h"

class Base : public IThread {
public:
	Base() {
		baseFuture = std::async(std::launch::async, [this] {
			while (working) {
				Sleep(5000);
			}
			int finish = 0;
			});
	}
	~Base() {
		Sleep(1);
		NotifyAboutStop();
		WaitingFinishThreads();
	}

	// IThread
	void NotifyAboutStop() override {
		working = false;
	}

	void WaitingFinishThreads() override {
		if (baseFuture.valid()) {
			baseFuture.wait();
		}
	}

private:
	LOG_FIELD_DESTRUCTION(Base);
	std::future<void> baseFuture;
	std::atomic<bool> working = true;
};


class ChildOne : public Base {
public:
	ChildOne() {
		futureChildOne = std::async(std::launch::async, [this] {
			while (working) {
				Sleep(5000);
			}
			int finish = 0;
			});
	}
	~ChildOne() {
		Sleep(1);
		NotifyAboutStop();
		WaitingFinishThreads();
	}

	// IThread
	void NotifyAboutStop() override {
		working = false;
		Base::NotifyAboutStop();
	}

	void WaitingFinishThreads() override {
		if (futureChildOne.valid()) {
			futureChildOne.wait();
		}
		Base::WaitingFinishThreads();
	}

private:
	LOG_FIELD_DESTRUCTION(ChildOne);
	std::future<void> futureChildOne;
	std::atomic<bool> working = true;
};

class ChildTwo : public ChildOne {
public:
	ChildTwo() {
	}
	~ChildTwo() {
		Sleep(1);
		NotifyAboutStop();
		WaitingFinishThreads();
	}


private:
	LOG_FIELD_DESTRUCTION(ChildTwo);
	std::atomic<bool> working = true;
};


class ChildThree : public ChildTwo {
public:
	ChildThree() {
		futureChildThree = std::async(std::launch::async, [this] {
			while (working) {
				Sleep(5000);
			}
			int finish = 0;
			});
	}
	~ChildThree() {
		Sleep(1);
		NotifyAboutStop();
		WaitingFinishThreads();
	}

	// IThread
	void NotifyAboutStop() override {
		working = false;
		ChildTwo::NotifyAboutStop(); // really called ChildOne::NotifyAboutStop() virtual method
	}

	void WaitingFinishThreads() override {
		if (futureChildThree.valid()) {
			futureChildThree.wait();
		}
		ChildTwo::WaitingFinishThreads(); // really called ChildOne::WaitingFinishThreads() virtual method
	}

private:
	LOG_FIELD_DESTRUCTION(ChildThree);
	std::future<void> futureChildThree;
	std::atomic<bool> working = true;
};


class Object {
public:
	Object() {
		instance = std::make_shared<ChildThree>();
		//threadsFinishHelper.Register(instance);
	}
	~Object() {
		Sleep(1);
	}

private:
	LOG_FIELD_DESTRUCTION(Object);
	std::shared_ptr<ChildThree> instance;
	ThreadsFinishHelper threadsFinishHelper;
};


void main() {
	{
		Object object;
		Sleep(3000);
	}
	Sleep(1000);
	return;
}