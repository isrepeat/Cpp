#include "Semaphore.h"
#include "System.h"

namespace HELPERS_NS {
	Semaphore::Semaphore(const std::wstring& name) {
		auto fullName = L"Semaphore_" + name;
		semaphore = CreateSemaphoreEx(nullptr, 1, 1, fullName.c_str(), 0, SEMAPHORE_ALL_ACCESS);
		if (!semaphore) {
			auto lastErrorHr = HRESULT_FROM_WIN32(GetLastError());
			HELPERS_NS::System::ThrowIfFailed(lastErrorHr);
		}
	}

	Semaphore::Semaphore(Semaphore&& other)
		: semaphore(other.semaphore)
	{
		other.semaphore = nullptr;
	}

	Semaphore::~Semaphore() {
		CloseHandle(semaphore);
	}

	void Semaphore::Lock() {
		auto result = WaitForSingleObject(semaphore, INFINITE);
		if (result == WAIT_FAILED) {
			HELPERS_NS::System::ThrowIfFailed(E_FAIL);
		}
	}

	void Semaphore::Unlock() {
		BOOL result = ReleaseSemaphore(semaphore, 1, nullptr);
		if (!result) {
			HELPERS_NS::System::ThrowIfFailed(E_FAIL);
		}
	}

	HELPERS_NS::Scope<std::function<void()>> Semaphore::LockScoped() {
		Lock();
		return { [this] { Unlock(); } };
	}
}