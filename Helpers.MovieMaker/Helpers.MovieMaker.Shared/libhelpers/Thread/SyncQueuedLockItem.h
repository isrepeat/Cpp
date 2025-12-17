#pragma once
#include "QueuedLock.h"
#include "critical_section.h"
#include "condition_variable.h"

namespace thread {
	class SyncQueuedLockItem : public QueuedLockItem {
	public:
		SyncQueuedLockItem(QueuedLock *queue);
		virtual ~SyncQueuedLockItem();

		void Acquired() override;
		bool IsCancelled() override;
		void Wait();
		void Cancel();

	private:
		QueuedLock *queue;

		thread::critical_section mtx;
		thread::condition_variable cv;
		bool acquired;
		bool cancelled;
	};
}