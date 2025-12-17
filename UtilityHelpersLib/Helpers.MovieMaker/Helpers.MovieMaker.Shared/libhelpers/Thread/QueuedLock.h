#pragma once
#include "QueuedLockItem.h"
#include "critical_section.h"

#include <deque>

namespace thread {
	// TEST with cancel
	class QueuedLock {
	public:
		QueuedLock();
		~QueuedLock();

		void Push(QueuedLockItem *item);
		void Pop(QueuedLockItem *item);

		void SetCurrentThreadAsOwner();
		bool IsLocked();

	private:
		thread::critical_section cs;

		DWORD curThreadId;
		std::deque<QueuedLockItem *> items;
	};
}