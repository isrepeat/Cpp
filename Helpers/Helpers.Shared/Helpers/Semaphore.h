#pragma once
#include "common.h"
#include "HWindows.h"
#include "Scope.h"
#include <functional>
#include <string>

namespace HELPERS_NS {
	class Semaphore {
	public:
		Semaphore(const std::wstring& name);
		Semaphore(const Semaphore&) = delete;
		Semaphore(Semaphore&&);
		~Semaphore();

		void Lock();
		void Unlock();

		HELPERS_NS::Scope<std::function<void()>> LockScoped();

	private:
		HANDLE semaphore{};
	};
}