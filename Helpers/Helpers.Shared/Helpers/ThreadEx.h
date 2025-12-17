#pragma once
#include "common.h"
#include <functional>
#include <thread>

namespace HELPERS_NS {
	// Wrapper over std::thread that notify you if was exception
	// NOTE: Move operator deleted because otherwise "this" can point to deleted ThreadEx object.
	//       So use std::unique_ptr / shared_ptr.
	class ThreadEx : public std::thread {
	public:
		ThreadEx() = default;
		~ThreadEx() = default;

		// TODO: Add Ctor and pass "thread message queue" to which signal will be sent

		explicit ThreadEx(std::function<void()> exceptionCallback, std::function<void()> lambda);

		ThreadEx(ThreadEx&&) = delete;
		ThreadEx& operator=(ThreadEx&&) = delete;

		ThreadEx(const ThreadEx&) = delete;
		ThreadEx& operator=(const ThreadEx&) = delete;

		std::exception_ptr GetExceptionPtr();

	private:
		void WorkingRoutine(std::function<void()> exceptionCallback, std::function<void()> lambda);

	private:
		std::exception_ptr exceptionPtr;
	};
}