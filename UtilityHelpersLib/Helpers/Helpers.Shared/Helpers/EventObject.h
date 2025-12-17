#pragma once
#include "common.h"
#include "HWindows.h"
#include "Scope.h"
#include <functional>
#include <string>

namespace HELPERS_NS {
	class EventObject {
	public:
		EventObject(const std::wstring& name);
		EventObject(const EventObject&) = delete;
		EventObject(EventObject&&);
		~EventObject();

		void Wait(DWORD timeoutMs = INFINITE);

		void Signal();
		void Reset();

		HELPERS_NS::Scope<std::function<void()>> ResetScoped();

	private:
		HANDLE event{};
	};
}