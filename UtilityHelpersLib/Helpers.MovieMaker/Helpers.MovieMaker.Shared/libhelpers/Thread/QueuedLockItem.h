#pragma once

namespace thread {
	class QueuedLockItem {
	public:
		QueuedLockItem();
		virtual ~QueuedLockItem();

		virtual void Acquired() = 0;
		virtual bool IsCancelled() = 0;
	};
}