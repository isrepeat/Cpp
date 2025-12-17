#include <chrono>
#include <coroutine>
#include <exception>
#include <iostream>
#include <thread>

struct TimerAwaitable {
    explicit TimerAwaitable(std::chrono::milliseconds delay) : delay(delay) {}

    bool await_ready() const noexcept {
        return delay.count() <= 0;
    }

    void await_suspend(std::coroutine_handle<> handle) {
        worker = std::jthread([handle, delay = delay]() {
            std::this_thread::sleep_for(delay);
            handle.resume();
        });
    }

    void await_resume() const noexcept {
    }

private:
    std::chrono::milliseconds delay;
    std::jthread worker;
};

struct Task {
    struct promise_type {
        Task get_return_object() {
            return Task{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        std::suspend_never initial_suspend() const noexcept {
            return {};
        }

        std::suspend_always final_suspend() const noexcept {
            return {};
        }

        void return_void() {
        }

        void unhandled_exception() {
            storedException = std::current_exception();
        }

        std::exception_ptr storedException;
    };

    explicit Task(std::coroutine_handle<promise_type> handle) : handle(handle) {}

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }

    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (handle) {
                handle.destroy();
            }
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }

    ~Task() {
        if (handle) {
            handle.destroy();
        }
    }

    void wait() {
        while (handle && !handle.done()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (handle && handle.promise().storedException) {
            std::rethrow_exception(handle.promise().storedException);
        }
    }

private:
    std::coroutine_handle<promise_type> handle = nullptr;
};

TimerAwaitable SleepFor(std::chrono::milliseconds delay) {
    return TimerAwaitable(delay);
}

Task CoroutineWithDelays() {
    using namespace std::chrono_literals;

    std::cout << "Корутина запущена" << std::endl;
    co_await SleepFor(250ms);

    std::cout << "Прошло 250 мс" << std::endl;
    co_await SleepFor(500ms);

    std::cout << "Прошло 750 мс в сумме" << std::endl;
    co_await SleepFor(1000ms);

    std::cout << "Корутина завершена (1750 мс)" << std::endl;
}

int main() {
    auto task = CoroutineWithDelays();
    task.wait();
    return 0;
}
