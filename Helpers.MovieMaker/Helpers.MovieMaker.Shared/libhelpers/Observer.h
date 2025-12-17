#pragma once
#include "ObservableToken.h"
#include "Thread\LockStack.h"

template<class Arg1>
class ObservableBase {
public:
	typedef std::function<void(Arg1)> FnType;

	ObservableToken<Arg1> Add(const FnType &fn) {
		ObservableToken<Arg1> token;

		token.sharedFn = std::make_shared<FnType>(fn);

		this->observers.push_back(token.sharedFn);

		token.iterator = --this->observers.end();
		token.observable = this;

		return token;
	}

	void Remove(const ObservableToken<Arg1> &v) {
		if (v.observable == this && v.sharedFn) {
			this->observers.erase(v.iterator);
		}
	}

	bool HaveObservers() {
		return !this->observers.empty();
	}

	bool CheckToken(const ObservableToken<Arg1> &v) {
		return v.observable == this;
	}

protected:
	std::list<std::weak_ptr<FnType>> observers;
};

// TODO refactor to remove or fix use of ScopedUnlock
// ScopedUnlock used to unlock thread::critical_section on stack in order to let event handlers to access locked objects

// implemetation of Observer pattern based on std::function
template<class Arg1>
class Observable : public ObservableBase<Arg1> {
public:
	void Notify(Arg1 arg, bool useScopedUnlock = true) {
		if (useScopedUnlock) {
			thread::LockStack::ScopedUnlock unlock;
			this->NotifyObservers(arg);
		}
		else {
			this->NotifyObservers(arg);
		}
	}

private:
	void NotifyObservers(Arg1 arg) {
		this->observers.remove_if(
			[&](const std::weak_ptr<FnType>& item)
			{
				bool remove = true;
				auto locked = item.lock();

				if (locked) {
					remove = false;
					(*locked)(arg);
				}

				return remove;
			});
	}
};

template<>
class Observable<void> : public ObservableBase<void> {
public:
	void Notify(bool useScopedUnlock = true) {
		if (useScopedUnlock) {
			thread::LockStack::ScopedUnlock unlock;
			this->NotifyObservers();
		}
		else {
			this->NotifyObservers();
		}
	}

private:
	void NotifyObservers() {
		this->observers.remove_if(
			[&](const std::weak_ptr<FnType>& item)
			{
				bool remove = true;
				auto locked = item.lock();

				if (locked) {
					remove = false;
					(*locked)();
				}

				return remove;
			});
	}
};
