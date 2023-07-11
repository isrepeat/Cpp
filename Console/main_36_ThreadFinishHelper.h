#pragma once
#include <memory>
#include <string>
#include <vector>
#include <future>
#include <Windows.h>
#include <functional>

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

// TODO: purge expired ptr when register and add mutex guard for it
// Must be the last field of the class
class ThreadsFinishHelper {
public:
	ThreadsFinishHelper() = default;
	~ThreadsFinishHelper() {
		Stop();
	}

	void Stop() {
		if (stopped)
			return;

		Visit([](std::shared_ptr<IThread> threadClass) {
			threadClass->NotifyAboutStop();
			});

		Visit([](std::shared_ptr<IThread> threadClass) {
			threadClass->WaitingFinishThreads();
			});

		stopped = true;
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
	std::atomic<bool> stopped = false;
};