#pragma once
#include "common.h"

namespace HELPERS_NS {
	struct IMutex {
		virtual ~IMutex() = default;

		virtual void lock() = 0;
		virtual void unlock() = 0;
	};

	template <typename MutexBaseT>
	struct Mutex : public IMutex, public MutexBaseT {
		void lock() override {
			this->MutexBaseT::lock();
		}
		void unlock() override {
			this->MutexBaseT::unlock();
		}
	};

	struct EmptyMutex : IMutex {
		void lock() override {}
		void unlock() override {}
	};
}