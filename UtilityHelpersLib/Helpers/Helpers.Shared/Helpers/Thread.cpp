#pragma once
#include "Thread.h"
#include "Logger.h"
#include <Windows.h>


namespace HELPERS_NS {
	ThreadsFinishHelper::~ThreadsFinishHelper() {
		Stop();
	}

	void ThreadsFinishHelper::Register(std::weak_ptr<IThread> threadClass, std::wstring className) {
		std::lock_guard lk{ mx };
		PurgeExpiredPointers();

		threadsClassesWeak.push_back({ threadClass, className });
		LOG_DEBUG_D(L"Registered [{}]", className);
	}

	void ThreadsFinishHelper::Stop() {
		if (stopped)
			return;

		Visit([](std::shared_ptr<IThread> threadClass, const std::wstring& className) {
			LOG_DEBUG_D(L"[{}] Notify about stop", className);
			threadClass->NotifyAboutStop();
			LOG_DEBUG_D(L"[{}] Notified", className);
			});

		LOG_DEBUG_D(L"--- All Registered classes notified aboud stop ---");

		Visit([](std::shared_ptr<IThread> threadClass, const std::wstring& className) {
			LOG_DEBUG_D(L"[{}] Waiting finish threads", className);
			threadClass->WaitingFinishThreads();
			LOG_DEBUG_D(L"[{}] Finished", className);
			});

		LOG_DEBUG_D(L"--- All Registered classes finished own threads ---");
		stopped = true;
	}

	void ThreadsFinishHelper::Visit(std::function<void(std::shared_ptr<IThread>, const std::wstring&)> handler) {
		for (auto& [threadClassWeak, className] : threadsClassesWeak) {
			if (auto threadClass = threadClassWeak.lock()) {
				handler(threadClass, className);
			}
		}
	}
	void ThreadsFinishHelper::PurgeExpiredPointers() {
		auto newEnd = std::remove_if(threadsClassesWeak.begin(), threadsClassesWeak.end(), [](decltype(threadsClassesWeak.back())& pair) {
			return pair.first.expired();
			});
		threadsClassesWeak.erase(newEnd, threadsClassesWeak.end());
	}



	thread_local std::wstring ThreadNameHelper::threadName = L"unnamed thread";

	void ThreadNameHelper::SetThreadName(const std::wstring& name) {
		threadName = name;
		SetThreadDescription(::GetCurrentThread(), threadName.c_str());
	}

	const std::wstring& ThreadNameHelper::GetThreadName() {
		return threadName;
	}

	size_t GetThreadId() {
		return static_cast<size_t>(::GetCurrentThreadId());
	}
}