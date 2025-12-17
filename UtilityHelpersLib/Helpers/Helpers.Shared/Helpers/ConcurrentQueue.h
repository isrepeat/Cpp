#pragma once
#include "common.h"
#include "Concurrency.h"
#include "MoveLambda.hpp"
#include "Logger.h"

#include <condition_variable>
#include <queue>
#include <mutex>

namespace HELPERS_NS {
	struct TaskItemWithDescription {
		std::string descrtiption;
		//std::function<void()> task;
		HELPERS_NS::movable_function<void()> task;
	};

	enum class ConcurrentQueueBehaviour {
		WaitWhenQueueSizeGreaterThanOrEqualBuffer,
		SkipWhenQueueSizeGreaterThanOrEqualBuffer,
	};

	template<class T>
	class ConcurrentQueue {
		CLASS_FULLNAME_LOGGING_INLINE_IMPLEMENTATION(ConcurrentQueue);

	public:
		ConcurrentQueue() = default;
		~ConcurrentQueue() = default;

		// TODO: write move Ctor
		ConcurrentQueue(const ConcurrentQueue& other) = delete;
		//ConcurrentQueue(const ConcurrentQueue& other) {
		//	std::lock_guard lkOther{ other.mx };
		//	this->working = other.working;
		//	this->behaviour = other.behaviour;
		//	this->bufferSize = other.bufferSize;
		//}

		void Push(T&& item) {
			if constexpr (std::is_same_v<T, TaskItemWithDescription>) {
				LOG_FUNCTION_ENTER_C("Push(item) <{}>", item.descrtiption);
			}

			auto pushPredicate = [this](std::function<void()> callback) {
				if (working) {
					if (IsQueueBufferExceeded()) {
						switch (behaviour) {
						case ConcurrentQueueBehaviour::WaitWhenQueueSizeGreaterThanOrEqualBuffer:
							return CV::WAIT;
						case ConcurrentQueueBehaviour::SkipWhenQueueSizeGreaterThanOrEqualBuffer:
							return CV::NO_WAIT; // no wait and ignore push
						};
					}
					else {
						if (callback) {
							callback(); // we can push here
						}
						return CV::NO_WAIT;
					}
				}
				
				return CV::NO_WAIT;
			};

			HELPERS_NS::movable_function<void()> pushCallback = [this, itemMoved = std::move(item)] () mutable {
				items.push(std::move(itemMoved));
			};

			// Use cv wait helper to avoid double checking for pushing (poping)
			std::unique_lock lk(mx);
			HELPERS_NS::CvExecuteCallbackAfterWaitWithPredicate(lk, cv, pushPredicate, std::move(pushCallback));
			cv.notify_one(); // signal to can pop 1 item
		}

		T Pop() {
			auto popPredicate = [this](std::function<void()> callback) {
				if (working) {
					if (!items.empty()) {
						if (callback) {
							callback(); // we can pop here
						}
						return CV::NO_WAIT;
					}
					else {
						return CV::WAIT;
					}
				}

				return CV::NO_WAIT;
			};
			
			std::unique_lock lk(mx);
			T res = HELPERS_NS::CvExecuteCallbackAfterWaitWithPredicate(lk, cv, popPredicate, [this] { // if callback not executed it return default T{}
				auto item = std::move(items.front());
				items.pop();
				return item;
				});

			if constexpr (std::is_same_v<T, TaskItemWithDescription>) {
				LOG_FUNCTION_ENTER_C("Pop() <{}>", res.descrtiption);
			}

			cv.notify_one(); // signal to can push 1 or more items if queue was overflow before
			return res;
		}

		void StopWork() {
			working = false;
			cv.notify_one();
		}

		void StartWork() {
			working = true;
		}

		bool IsWorking() {
			return working;
		}

		bool HasItems() {
			std::lock_guard lk(mx);
			return !items.empty();
		}

		int ItemsCount() {
			std::lock_guard lk(mx);
			return items.size();
		}

		void SetBufferSize(const uint32_t size, ConcurrentQueueBehaviour behaviour = ConcurrentQueueBehaviour::WaitWhenQueueSizeGreaterThanOrEqualBuffer) {
			std::lock_guard lk(mx);
			this->bufferSize = size;
			this->behaviour = behaviour;
		}

	private:
		bool IsQueueBufferExceeded() {
			return items.size() >= bufferSize;
		}

	private:
		std::mutex mx;
		std::queue<T> items;
		std::condition_variable cv;
		std::atomic<bool> working = true;
		uint32_t bufferSize = 200000;
		ConcurrentQueueBehaviour behaviour = ConcurrentQueueBehaviour::WaitWhenQueueSizeGreaterThanOrEqualBuffer;
	};
}