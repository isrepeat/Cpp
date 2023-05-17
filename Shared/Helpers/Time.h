#pragma once
#include <thread>
#include <chrono>
#include <functional>
#include <condition_variable>

using namespace std::chrono_literals;

namespace H {
	class Timer {
	public:
		template <typename Duration>
		static void Once(Duration timeout, std::function<void()> callback) {
			std::thread([=] {
				std::this_thread::sleep_for(timeout);
				callback();
				}).detach();
		}

		template <typename Duration>
		static bool Once(std::shared_ptr<std::function<void()>>& tokenTask, Duration timeout, std::function<void()> callback) {
			if (tokenTask)
				return false; // cannot rewrite token callback while it is not empty

			tokenTask = std::make_shared<std::function<void()>>(callback);
			std::weak_ptr<std::function<void()>> callbackWeakPtr = tokenTask;

			std::thread([&tokenTask, timeout, callbackWeakPtr] {
				std::this_thread::sleep_for(timeout);
				if (auto lockedCallback = callbackWeakPtr.lock()) {
					(*lockedCallback)();
					tokenTask = nullptr;
				}
				}).detach();

			return true;
		}

		Timer() = default;
		Timer(std::chrono::milliseconds timeout, std::function<void()> callback) {
			Start(timeout, callback);
		}
		~Timer() {
			StopThread();
		}

		void Start(std::chrono::milliseconds timeout, std::function<void()> callback) {
			StopThread();
			this->callback = callback;
			StartThread(timeout);
		}
		void Stop() {
			StopThread();
		}

		void Reset(std::chrono::milliseconds timeout) {
			StopThread();
			StartThread(timeout);
		}

	private:
		void StopThread() {
			stop = true;
			cv.notify_all();
			if (threadTimer.joinable())
				threadTimer.join();
		}

		void StartThread(std::chrono::milliseconds timeout) {
			stop = false;
			threadTimer = std::thread([this, timeout] {
				std::mutex mtx;
				std::unique_lock<std::mutex> lock(mtx);
				if (!cv.wait_for(lock, timeout, [this] { return static_cast<bool>(stop); })) {
					this->callback(); // if timeout and stop == false -> execute callback
				}
				});
		}

		std::function<void()> callback;
		std::condition_variable cv;
		std::thread threadTimer;
		std::atomic<bool> stop = false;
	};

	class MeasureTime {
	public:
		MeasureTime();
		~MeasureTime();

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> start;
	};


	enum class TimeFormat {
		None,
		Ymdhms_with_separators,
	};
	std::string GetTimeNow(TimeFormat format = TimeFormat::None);
	std::string GetTimezone();
};

#ifdef _DEBUG
#define MEASURE_TIME H::MeasureTime t;
#else
#define MEASURE_TIME
#endif