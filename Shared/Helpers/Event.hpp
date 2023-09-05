#pragma once
#include <mutex>
#include <vector>
#include <functional>

template<typename ...Args>
class Event {
public:
	typedef std::function<void(Args...)> callback;

	std::shared_ptr<callback> Add(callback func) {
		auto sharedPtr = std::make_shared<callback>(std::move(func));
		auto lock = std::lock_guard(callbacksMtx);
		callbacks.push_back(sharedPtr);
        return sharedPtr;
	}

    void operator()(Args... args) {
        decltype(callbacks) tmp;
        {
            auto lock = std::lock_guard(callbacksMtx);

            callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(),
                [](std::weak_ptr<callback> ptr)
                {
                    return ptr.expired();
                }),
                callbacks.end());

            tmp = callbacks;
        }

        for (auto wptr : tmp) {
            auto locked = wptr.lock();

            if (locked) {
                (*locked)(std::forward<Args>(args)...);
            }
        }
    }

private:
	std::mutex callbacksMtx;
	std::vector<std::weak_ptr<callback>> callbacks;
};