#pragma once
#include "ThreadEx.h"
#include "Logger.h"

namespace HELPERS_NS {
	ThreadEx::ThreadEx(std::function<void()> exceptionCallback, std::function<void()> lambda)
		: std::thread(std::bind(&ThreadEx::WorkingRoutine, this, std::placeholders::_1, std::placeholders::_2), exceptionCallback, lambda)
	{
	}

	std::exception_ptr ThreadEx::GetExceptionPtr() {
		return exceptionPtr;
	}

	void ThreadEx::WorkingRoutine(std::function<void()> exceptionCallback, std::function<void()> lambda) {
		try {
			lambda();
		}
		catch (...) {
			LOG_ERROR_D("Caught exception");
			exceptionPtr = std::current_exception();
			exceptionCallback(); // TODO: use signal to notify main thread where ThreadEx was created (instead callback)
		}
	}
}