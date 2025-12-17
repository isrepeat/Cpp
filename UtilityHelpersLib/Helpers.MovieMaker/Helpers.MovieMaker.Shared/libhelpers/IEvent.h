#pragma once

#include <functional>
#include <memory>

template<typename ...Args>
class IEvent {
public:
    using callback = std::function<void(Args...)>;
    using token = std::shared_ptr<callback>;

    virtual ~IEvent() = default;

    std::shared_ptr<callback> attach(callback r) {
        return this->attach(std::make_shared<callback>(std::move(r)));
    }

    virtual std::shared_ptr<callback> attach(std::shared_ptr<callback> r) = 0;

    virtual void call(Args... args) = 0;
};