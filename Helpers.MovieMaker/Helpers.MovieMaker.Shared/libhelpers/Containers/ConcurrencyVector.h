#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>

template <class T, class Alloc = std::allocator<T>>
class ConcurrencyVector {
private:
	std::vector<T, Alloc> dataVector;
	uint32_t nowWaiting;
	std::mutex pushMtx;
	std::condition_variable pushCv;
	bool notified;
public:

	ConcurrencyVector()
		:notified(false), nowWaiting(0) {

	}

	~ConcurrencyVector() {
	}

	void PushBack(T item) {
		std::unique_lock<std::mutex> lk(this->pushMtx);
		try {
			this->dataVector.push_back(item);
		}
		catch (...) {
			int stop = 324;
		}
		auto size = this->dataVector.size();
		if (size > this->nowWaiting)
		{
			this->pushCv.notify_one();
			notified = true;
		}
	}

	T* Data() {
		std::unique_lock<std::mutex> lk(this->pushMtx);

		return this->dataVector.data();
	}

	uint32_t Size() {
		std::unique_lock<std::mutex> lk(this->pushMtx);
		return this->dataVector.size();
	}

	void Clear() {
		std::unique_lock<std::mutex> lk(this->pushMtx);
		this->pushCv.notify_all();
		notified = false;
		this->dataVector.clear();
		this->dataVector.resize(0);
	}

	T& operator[](int idx) {
		std::unique_lock<std::mutex> lk(this->pushMtx);

		if (this->dataVector.size() > idx)
			return this->dataVector[idx];
		else {

			this->nowWaiting = idx;
			while (!notified)
			{
				this->pushCv.wait(lk);
				notified = false;
			}
			return this->dataVector[idx];
		}
	}
};