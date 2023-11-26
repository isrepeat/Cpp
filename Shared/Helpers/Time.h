#pragma once
#include <thread>
#include <chrono>
#include <future>
#include <functional>
#include <condition_variable>

using namespace std::chrono_literals;

namespace H {
	class Timer	{
	public:
		template <typename Duration>
		static void Once(Duration timeout, std::function<void()> callback) { // Not thread safe callback call
			std::thread([=] {
				std::this_thread::sleep_for(timeout);
				callback();
				}).detach();
		}

		template <typename Duration>
		static bool Once(std::unique_ptr<std::future<void>>& futureToken, Duration timeout, std::function<void()> callback) { // Thread safe callback call
			if (futureToken)
				return false; // Guard from double set. Don't init new future until previous not finished.

			futureToken = std::make_unique<std::future<void>>(std::async(std::launch::async, [&futureToken, timeout, callback] {
				std::this_thread::sleep_for(timeout);
				callback();
				futureToken = nullptr;
				}));

			return true;
		}


		Timer() = default;
		Timer(std::chrono::milliseconds timeout, std::function<void()> callback, bool autoRestart = false) {
			Start(timeout, callback, autoRestart);
		}
		~Timer() {
			StopThread();
		}

		void Start(std::chrono::milliseconds timeout, std::function<void()> callback, bool autoRestart = false) {
			StopThread();
			{
				std::lock_guard lk1{ mx };
				this->timeout = timeout;
				this->callback = callback;
				this->autoRestart = autoRestart;
			}
			StartThread();
		}
		void Stop() {
			StopThread();
		}

		void Reset(std::chrono::milliseconds timeout) {
			StopThread();
			StartThread();
		}

	private:
		void StopThread() {
			stop = true;
			cv.notify_all();
			if (threadTimer.joinable())
				threadTimer.join();
		}

		void StartThread() {
			stop = false;
			threadTimer = std::thread([this] {
				std::lock_guard lk1{ mx }; // guard from changing timeout and callback
				while (!stop) {
					std::mutex tmpMx;
					std::unique_lock lk2{ tmpMx };
					if (!cv.wait_for(lk2, timeout, [this] { return static_cast<bool>(stop); })) {
						if (callback) {
							callback();
						}
					}
					if (!autoRestart) {
						stop = true;
						break;
					}
				}
				});
		}
	
	private:
		std::mutex mx;
		std::thread threadTimer;
		std::condition_variable cv;

		std::function<void()> callback;
		std::chrono::milliseconds timeout;

		std::atomic<bool> stop = false;
		std::atomic<bool> autoRestart = false;
	};


	class MeasureTime {
	public:
		MeasureTime();
		~MeasureTime();

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> start;
	};

	class MeasureTimeScoped {
	public:
		MeasureTimeScoped(std::function<void(uint64_t dtMs)> completedCallback);
		~MeasureTimeScoped();

	private:
		std::function<void(uint64_t dtMs)> completedCallback;
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