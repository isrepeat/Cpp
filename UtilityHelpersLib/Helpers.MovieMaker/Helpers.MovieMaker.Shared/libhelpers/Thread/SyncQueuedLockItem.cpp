#include "pch.h"
#include "SyncQueuedLockItem.h"

namespace thread {
	SyncQueuedLockItem::SyncQueuedLockItem(QueuedLock *queue)
		: acquired(false), cancelled(false), queue(queue)
	{}

	SyncQueuedLockItem::~SyncQueuedLockItem() {
		this->queue->Pop(this);
	}

	void SyncQueuedLockItem::Acquired() {
		{
			thread::critical_section::scoped_lock lk(this->mtx);
			this->acquired = true;
		}

		this->cv.notify();
	}

	bool SyncQueuedLockItem::IsCancelled() {
		thread::critical_section::scoped_lock lk(this->mtx);
		return this->cancelled;
	}

	void SyncQueuedLockItem::Wait() {
		thread::critical_section::scoped_lock lk(this->mtx);

		while (!this->acquired && !this->cancelled) {
			this->cv.wait(this->mtx);
		}

		if (this->cancelled) {
			throw std::exception("cancelled");
		}

		this->queue->SetCurrentThreadAsOwner();
	}

	void SyncQueuedLockItem::Cancel() {
		{
			thread::critical_section::scoped_lock lk(this->mtx);
			this->cancelled = true;
		}

		this->cv.notify();
	}
}