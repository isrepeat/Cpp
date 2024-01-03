#include "FutureWrapper.h"
#include <Helpers/Time.h>
#include <Windows.h>
#include <vector>
#include <string>
#include <set>

#define LOG_FIELD_DESTRUCTION(ClassName)																\
    struct ClassName##Field {																			\
        ~ClassName##Field() {																			\
            OutputDebugStringA("===== ~" #ClassName "Field() destroyed ===== \n");						\
        }																								\
    } ClassName##FieldInstance;

struct LongDestoyedField {
	LongDestoyedField(int ms) : ms{ ms } {
	}
	~LongDestoyedField() {
		Sleep(ms);
	}
private:
	int ms = 0;
};



// Interface to manage lifetime internal class threads outside (use with ThreadsFinishHelper)
class IThread {
public:
	virtual void NotifyAboutStop() = 0;
	virtual void WaitingFinishThreads() = 0;
	virtual ~IThread() = default;
};


// Must be the last field of the class
class ThreadsFinishHelper {
public:
	ThreadsFinishHelper() = default;
	~ThreadsFinishHelper() {
		Visit([](std::shared_ptr<IThread> threadClass) {
			threadClass->NotifyAboutStop();
			});

		Visit([](std::shared_ptr<IThread> threadClass) {
			threadClass->WaitingFinishThreads();
			});
	}

	void Register(std::weak_ptr<IThread> threadClass) {
		threadsClassesWeak.push_back(threadClass);
	}

private:
	void Visit(std::function<void(std::shared_ptr<IThread>)> handler) {
		for (auto& threadClassWeak : threadsClassesWeak) {
			if (auto threadClass = threadClassWeak.lock()) {
				handler(threadClass);
			}
		}
	}

private:
	std::vector<std::weak_ptr<IThread>> threadsClassesWeak;
};




class Base : public IThread {
public:
	Base() {
		// init base future ...
	}
	~Base() = default;

	// IThread
	void NotifyAboutStop() override {
		// ...
	}

	void WaitingFinishThreads() override {
		// ...
	}

private:
	std::future<void> baseFuture;
};


class SomeClass : public Base {
public:
	SomeClass() {
		// init future ...
	}
	~SomeClass() = default;

	// IThread
	void NotifyAboutStop() override {
		// ...
	}

	void WaitingFinishThreads() override {
		// ...
	}

private:
	std::future<void> future;
};


class Object {
public:
	Object() {
		// ...
		threadsFinishHelper.Register(someClass);
	}
	~Object() {
		// called SomeClass::NotifyAboutStop() & SomeClass::WaitingFinishThreads()
	}

private:
	std::shared_ptr<SomeClass> someClass;
	ThreadsFinishHelper threadsFinishHelper;
};




class BaseDesktopCapturer : public IThread {
public:
	BaseDesktopCapturer() {
		framesQueueRoutine = std::async(std::launch::async, [this] {
			while (working) {
				Sleep(500);
				if (frameHandler) {
					frameHandler(111);
				}
				Sleep(500);
			}
			});
	}

	~BaseDesktopCapturer() {
	}

	void SetFrameHandler(std::function<void(int)> handler) {
		frameHandler = handler;
	}

	// IThread
	void NotifyAboutStop() override {
		working = false;
	}

	void WaitingFinishThreads() override {
		if (framesQueueRoutine.valid()) {
			framesQueueRoutine.wait();
		}
	}

private:
	LOG_FIELD_DESTRUCTION(BaseDesktopCapturer);
	std::future<void> framesQueueRoutine;
	std::atomic<bool> working = true;

	std::function<void(int)> frameHandler = [] (int) {};
};


class DesktopCaptureManager {
public:
	DesktopCaptureManager() {
		capturer = std::make_shared<BaseDesktopCapturer>();
	}

	~DesktopCaptureManager() {
	}

	void SetFrameHandler(std::function<void(int)> handler) {
		frameHandler = handler;
		capturer->SetFrameHandler(frameHandler);
		frameHandler = nullptr;
	}

	std::shared_ptr<BaseDesktopCapturer> GetCapturer() {
		return capturer;
	}

private:
	LOG_FIELD_DESTRUCTION(DesktopCaptureManager);
	std::shared_ptr<BaseDesktopCapturer> capturer;
	std::function<void(int)> frameHandler = [] (int) {};
};



class Logger {
public:
	Logger() = default;
	~Logger() {
		int xx = 9;
	}

	void Print(std::string msg) {
		messages.push_back(msg);
		messages.push_back(msg);
		messages.push_back(msg);
		printf("[log] %s \n", messages.back().c_str());
		messages.clear();
	}

private:
	LOG_FIELD_DESTRUCTION(Logger);
	std::vector<std::string> messages;
};

class VncServer {
public:
	VncServer() {
		desktopManager = std::make_shared<DesktopCaptureManager>();
		desktopManager->SetFrameHandler([this] (int data) {
			logger.Print("Frame handler");
			logger.Print("   data = " + std::to_string(data));
			logger.Print("");
			});

		threadsFinishHelper.Register(desktopManager->GetCapturer());
	}

	~VncServer() {
	}

private:
	LOG_FIELD_DESTRUCTION(VncServer);
	std::shared_ptr<DesktopCaptureManager> desktopManager;
	LongDestoyedField longDestroyedField = 5'000; // [ms]
	Logger logger;
	ThreadsFinishHelper threadsFinishHelper;
};



int main() {

	auto server = std::make_unique<VncServer>();

	H::Timer::Once(3'000ms, [&server] {
		Sleep(10);
		server.reset();
		Sleep(10);
		});


	while (true) {
		Sleep(10);
	}
	return 0;
}