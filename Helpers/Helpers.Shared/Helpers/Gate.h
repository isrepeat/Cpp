#pragma once
#include "common.h"
#include <condition_variable>
#include <mutex>

namespace HELPERS_NS {
	class Gate {
	public:
		Gate();
		~Gate() = default;

		void Lock();
		void Wait();
		void Notify();

	private:
		std::mutex mx;
		std::atomic<bool> gotResult;
		std::condition_variable cv;
	};
}