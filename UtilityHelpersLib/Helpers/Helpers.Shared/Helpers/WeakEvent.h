#pragma once
#include "common.h"
#include "IWeakEvent.h"

#include <vector>
#include <mutex>
#include <algorithm>

namespace HELPERS_NS {
    template<typename ...Args>
    class WeakEvent final : public IWeakEvent<Args...> {
    public:
        using typename IWeakEvent<Args...>::Handler;

        void Subscribe(Handler handler, IWeakEventToken token) override {
            auto lock = std::lock_guard(this->mx);
            this->callbacks.emplace_back(std::move(token), std::move(handler));
        }

        void Unsubscribe(IWeakEventToken token) override {
            auto lockedToken = token.lock();
            if (!lockedToken) {
                return;
            }

            auto lock = std::lock_guard(this->mx);
            auto it = std::find_if(this->callbacks.begin(), this->callbacks.end(),
                [&lockedToken](const HandlerItem& item)
                {
                    return lockedToken == item.token.lock();
                });
            if (it != this->callbacks.end()) {
                this->callbacks.erase(it);
            }
        }

        void operator()(Args... args) {
            std::vector<HandlerItem> tmp;
            {
                auto lock = std::lock_guard(this->mx);

                std::erase_if(this->callbacks,
                    [](const HandlerItem& item)
                    {
                        return item.token.expired();
                    });

                tmp = callbacks;
            }

            for (auto& item : tmp) {
                if (auto locked = item.token.lock()) {
                    item.handler(std::forward<Args>(args)...);
                }
            }
        }

    private:
        struct HandlerItem {
            IWeakEventToken token;
            Handler handler;
        };

        std::mutex mx;
        std::vector<HandlerItem> callbacks;
    };
}
