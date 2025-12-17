#pragma once
#include "IEvent.h"

#include <vector>
#include <algorithm>
#include <mutex>

template<typename ...Args>
class Event : public IEvent<Args...> {
public:
    using IEvent<Args...>::attach;
    using IEvent<Args...>::callback;

    std::shared_ptr<callback> attach(std::shared_ptr<callback> r) override {
        std::lock_guard<std::mutex> lock(m);
        targets.push_back(r);
        return r;
    }

    void call(Args... args) override {
        (*this)(std::forward<Args>(args)...);
    }

    void operator()(Args... args) {
        decltype(targets) tmp;
        {
            std::lock_guard<std::mutex> lock(m);

            targets.erase(std::remove_if(std::begin(targets), std::end(targets),
                [](std::weak_ptr<callback> ptr)
                {
                    return !(bool)(ptr.lock());
                }),
                std::end(targets));

            tmp = targets;
        }

        for (auto wptr : tmp) {
            auto locked = wptr.lock();

            if (locked) {
                (*locked)(std::forward<Args>(args)...);
            }
        }
    }

private:
    std::mutex m;
    std::vector<std::weak_ptr<callback>> targets;
};