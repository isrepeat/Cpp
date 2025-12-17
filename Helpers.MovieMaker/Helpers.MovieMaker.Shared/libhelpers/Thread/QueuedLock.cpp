#include "pch.h"
#include "QueuedLock.h"

#include <assert.h>
#include <string>

namespace thread {
	QueuedLock::QueuedLock()
		: curThreadId(0)
	{}

	QueuedLock::~QueuedLock() {}

	void QueuedLock::Push(QueuedLockItem *item) {
		thread::critical_section::scoped_lock lk(this->cs);

		if (this->items.empty()) {
			item->Acquired();
		}

		if (item->IsCancelled()) {
			return;
		}

		this->items.push_back(item);
	}

	void QueuedLock::Pop(QueuedLockItem *item) {
		QueuedLockItem *nextLock = nullptr;

		{
			thread::critical_section::scoped_lock lk(this->cs);

			/*
			this->items can be empty in such case:
			t1: add item1 + acquired
			t2: add item2

			t2: item2 cancel

			t1: item1.pop
			t1: queue.pop(item1)
			t1: queue.pop(item2.cancelled)

			t2: item2.pop
			t2: queue.empty == true
			*/

			/*if (this->items.empty()) {
				std::wstring msg = L"Ya-ta-ta-ta-ta-ta-ta-ta ya-ta-ta-ta-ta-ta-ta-ta do-de-da-va-da-da-dada! Kaboom-Kaboom! " + std::to_wstring(GetTickCount64());
				OutputDebugStringW(msg.c_str());
			}*/

			// this->items must be not empty if item is not cancelled
			assert(!(!item->IsCancelled() && this->items.empty()));

			if (!this->items.empty()) {
				if (this->items.front() == item) {
					bool found = false;

					this->items.pop_front();
					this->curThreadId = 0;

					while (!this->items.empty() && !found) {
						nextLock = this->items.front();

						// if item is cancelled nextLock may be null
						// and cancelled item may be not only in front of this->items
						if (nextLock && !nextLock->IsCancelled()) {
							// Valid item found. Exit cycle
							found = true;
							break;
						}
						else {
							/*if (nextLock && nextLock->IsCancelled()) {
								std::wstring msg = L"RARE case " + std::to_wstring(GetTickCount64());
								OutputDebugStringW(msg.c_str());
							}*/

							this->items.pop_front();
						}
					}
				}
				else {
					// for now it must be the only case when item is not in front of the queue
					assert(item->IsCancelled());

					// item cancelled and it's not in front then set this item to null
					for (auto it = this->items.begin(); it != this->items.end(); ++it) {
						if (*it == item) {
							*it = nullptr;
							break;
						}
					}
				}
			}
		}

		// can be deleted by other thread!!!
		// but it must not do this because nextLock which is Acquired will be in front of this->items
		// therefore it will stay alive until it will be popped by client code(destructor for example)
		// TODO test cancellation!!!
		if (nextLock) {
			nextLock->Acquired();
		}
	}

	void QueuedLock::SetCurrentThreadAsOwner() {
		thread::critical_section::scoped_lock lk(this->cs);
		this->curThreadId = GetCurrentThreadId();
	}

	bool QueuedLock::IsLocked() {
		thread::critical_section::scoped_lock lk(this->cs);
		bool locked = this->curThreadId == GetCurrentThreadId();
		return locked;
	}
}