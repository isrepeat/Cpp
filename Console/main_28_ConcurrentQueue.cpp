#include <mutex>
#include <thread>
#include <memory>
#include <queue>
#include <condition_variable>
#include <Windows.h>

class ConcurrentQueue {
public:
	ConcurrentQueue() = default;
	~ConcurrentQueue() {
		int xx = 9;
	}

	void Push(int data) {
		std::unique_lock lk{ mx };
		items.push(data);
		cv.notify_one();
	}

	int Pop() {
		std::unique_lock lk{ mx };
		cv.wait(lk, [this] {
			Sleep(500); // emulate check predicate some time ...
			return !working;
			});

		int res = 0;
		if (!items.empty()) {
			res = std::move(items.front());
			items.pop();
		}
		return res;
	}

	void Stop() {
		working = false;
		cv.notify_one();
	}

private:
	std::mutex mx;
	std::queue<int> items;
	std::condition_variable cv;
	std::atomic<bool> working = true;
};


void main() {
	auto concurrentQueue = std::make_shared<ConcurrentQueue>();

	auto th = std::thread([concurrentQueue] { // if pass by ref will be crash when ~mutex()
		auto data = concurrentQueue->Pop();
		});

	Sleep(1000);
	// emulate destructor
	concurrentQueue->Stop(); // explicit call Stop in Dtor
	concurrentQueue.reset();


	if (th.joinable())
		th.join();
}