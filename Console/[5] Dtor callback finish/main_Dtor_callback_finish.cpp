#include <mutex>
#include <iostream>
#include <Windows.h>
#include <functional>
#include "ServerCallbacks.h"
#include <Helpers/Time.h>


ServerCallbacks CreateServerCallbacks() {
	ServerCallbacks serverCallbacks;

	serverCallbacks.connected = std::function([] {
		Sleep(10);
		});

	serverCallbacks.adaptersChanged = std::function([] {
		Sleep(10);
		});

	serverCallbacks.capturerChangingConfirmation = std::function([] {
		Sleep(1000);
		Sleep(1000);
		return true;
		});

	serverCallbacks.frameData = std::function([] (int data) {
		Sleep(10);
		});

	return serverCallbacks;
}


class DesktopCapturer {
	struct FirstField {
		~FirstField() {
			int xxx = 9;
		}
	};
public:
	DesktopCapturer() {
		classData.resize(1000);
		auto xxx = classData[666];
	}

	~DesktopCapturer() {
		Stop();
		int xxx = 9;
	}

	void SetFrameHandler(std::function<void(int)> framaHandler) {
		this->framaHandler = framaHandler;
	}

	void SetCapturerChangingConfirmation(std::function<bool()> capturerChangingConfirmation) {
		this->capturerChangingConfirmation = capturerChangingConfirmation;
	}

	void Start() {
		working = true;
		thChannel = std::thread([this] {
			int frameCounter = 0;
			while (working) {
				Sleep(1000);
				frameCounter++;
				if (frameCounter < 3) {
					framaHandler(111);
				}
				if (frameCounter == 3) {
					H::Timer::Once(tokenFutureCapturerChangingConfirmation, 0ms, [=] {
						if (capturerChangingConfirmation()) { // TODO: may block this thread for a long time (so you need wait finish timer thread in ~DesktopCaptureManager)
							auto xxx = classData[666];
							isServiceCapturer = true;
						}
						});
				}
			}
			});
	}

	void Stop() {
		working = false;
		if (thChannel.joinable())
			thChannel.join();
	}

private:
	FirstField firstField;
	std::atomic<bool> working = false;
	std::atomic<bool> isServiceCapturer = false;
	std::thread thChannel;

	std::vector<int> classData;

	std::function<void(int)> framaHandler;
	std::function<bool()> capturerChangingConfirmation;

	std::unique_ptr<std::future<void>> tokenFutureCapturerChangingConfirmation;
};


class VncServer {
	struct FirstField {
		~FirstField() {
			int xxx = 9;
		}
	};
public:
	VncServer(const ServerCallbacks& callbacks)
		: serverCallbacks{ std::make_shared<ServerCallbacks>(callbacks) }
	{
		desktopCapturer.SetFrameHandler([this](int data) {
			serverCallbacks->frameData(data);
			});

		desktopCapturer.SetCapturerChangingConfirmation([this] {
			bool res = serverCallbacks->capturerChangingConfirmation();
			return res;
			});

		desktopCapturer.Start();
	}

	~VncServer() {
		//auto lk = serverCallbacks->capturerChangingConfirmation.ScopedLock();
		//if (callbacks->IsUseCvGuard()) {
		//	std::unique_lock lk{ mxCallbacks };
		//	cvCallbacks->wait(lk, [this] {
		//		int count = callbacks.use_count();
		//		return count <= 1; // wait if count > 1
		//		});
		//}

		int xxx = 9;
	}


private:
	FirstField firstField;
	std::mutex mxCallbacks;
	std::shared_ptr<int> tokenCallbacks = std::make_shared<int>();
	std::shared_ptr<ServerCallbacks> serverCallbacks;
	std::shared_ptr<std::condition_variable> cvCallbacks = std::make_shared<std::condition_variable>();
	
	DesktopCapturer desktopCapturer;
};



void main() {
	std::thread thDestroyer;
	std::unique_ptr<VncServer> vncServer = std::make_unique<VncServer>(CreateServerCallbacks());

	thDestroyer = std::thread([&vncServer] {
		std::this_thread::sleep_for(std::chrono::milliseconds(3500));
		vncServer.reset();
		});

	while (true) {
		Sleep(10);
	}

	if (thDestroyer.joinable())
		thDestroyer.join();

	return;
}