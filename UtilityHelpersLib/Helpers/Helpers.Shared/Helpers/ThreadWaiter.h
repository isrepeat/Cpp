#pragma once
#include "common.h"
#include "Thread.h"
#include "Logger.h"
#include <condition_variable>

namespace HELPERS_NS {
	class ThreadWaiter : public IThread {
	public:
		ThreadWaiter() = default;
		~ThreadWaiter() {
			LOG_FUNCTION_ENTER("~ThreadWaiter()");
			if (!stop) { // If the owner of this class not use ThreadsFinishHelper so finish threads manually
				NotifyAboutStop();
				WaitingFinishThreads();
			}
		}


		void Add(std::function<void(const ThreadWaiter&)> callback) {
			threads.push_back(std::thread([this, callback] {
				callback(*this);
				}));
		}

		template<typename Duration>
		bool WaitFor(std::mutex& mutex, Duration duration) const {
			std::unique_lock<std::mutex> lk(mutex);
			return !cv.wait_for(lk, duration, [this] { return static_cast<bool>(stop); }); // If timeout or notified - return Predicate result;
		}

		// Overriden with local mutex if not need to protect outside data
		template<typename Duration>
		bool WaitFor(Duration duration) const {
			std::mutex m;
			return this->WaitFor(m, duration);
		}

		template<typename Duration>
		bool WaitAbs(std::mutex& mutex, std::chrono::steady_clock::time_point& timePoint, Duration duration) const {
			std::unique_lock<std::mutex> lk(mutex);
			bool continuation = !cv.wait_until(lk, timePoint + duration, [this] { return static_cast<bool>(stop); });
			timePoint = std::chrono::high_resolution_clock::now();
			return continuation;
		}

		template<typename Duration>
		bool WaitAbs(std::chrono::steady_clock::time_point& timePoint, Duration duration) const {
			std::mutex m;
			return this->WaitAbs(m, timePoint, duration);
		}

		// IThread
		void NotifyAboutStop() override {
			stop = true;
			cv.notify_all();
		}

		void WaitingFinishThreads() override {
			LOG_FUNCTION_SCOPE("WaitingFinishThreads()");
			for (auto& thread : threads) {
				if (thread.joinable())
					thread.join();
			}
			threads.clear();
		}

	private:
		mutable std::condition_variable cv;
		std::vector<std::thread> threads;
		std::atomic<bool> stop = false;
	};
}