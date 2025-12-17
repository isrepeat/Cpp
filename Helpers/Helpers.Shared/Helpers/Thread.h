#pragma once
#include "common.h"
#include "ThreadEx.h"
#include <functional>
#include <utility>
#include <memory>
#include <string>
#include <vector>
#include <mutex>

namespace HELPERS_NS {
	// Interface to manage lifetime internal class threads outside
	// - Use it with ThreadsFinishHelper. 
	// - You can call NotifyAboutStop & WaitingFinishThreads in derived class desctructor explicitly to guarantee that all threads will finished.
	class IThread {
	public:
		virtual void NotifyAboutStop() = 0;
		virtual void WaitingFinishThreads() = 0;
		virtual ~IThread() = default;
	};


	// Must be the last field of the class (or call Stop in owner class destructor explicitly)
	class ThreadsFinishHelper {
	public:
		ThreadsFinishHelper() = default;
		~ThreadsFinishHelper();

		// NOTE: classNames used for log/debug purpose so delete it in future builds
		void Register(std::weak_ptr<IThread> threadClass, std::wstring className = L"...");
		void Stop();

	private:
		void Visit(std::function<void(std::shared_ptr<IThread>, const std::wstring&)> handler);
		void PurgeExpiredPointers();

	private:
		std::mutex mx;
		std::vector<std::pair<std::weak_ptr<IThread>, std::wstring>> threadsClassesWeak;
		std::atomic<bool> stopped = false;
	};


	class ThreadNameHelper {
	public:
		static void SetThreadName(const std::wstring& name);
		static const std::wstring& GetThreadName();

	private:
		static thread_local std::wstring threadName;
	};

	size_t GetThreadId();
}