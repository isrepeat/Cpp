#include <chrono>
#include <coroutine>
#include <condition_variable>
#include <exception>
#include <iostream>
#include <mutex>
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
    struct SharedState {
        std::mutex mtx;
        std::condition_variable cv;
        bool completed = false;
        std::exception_ptr storedException;
    };

    struct promise_type;

    struct FinalAwaiter {
        std::shared_ptr<SharedState> state;

        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<promise_type> h) const noexcept {
            auto& promise = h.promise();
            auto localState = promise.state;
            {
                std::lock_guard lock(localState->mtx);
                localState->completed = true;
            }
            localState->cv.notify_all();
        }

        void await_resume() const noexcept {
        }
    };

    struct promise_type {
        Task get_return_object() {
            return Task{ std::coroutine_handle<promise_type>::from_promise(*this), state };
        }

        std::suspend_never initial_suspend() const noexcept {
            return {};
        }

        FinalAwaiter final_suspend() const noexcept {
            return FinalAwaiter{ state };
        }

        void return_void() {
        }

        void unhandled_exception() {
            state->storedException = std::current_exception();
        }

        std::shared_ptr<SharedState> state = std::make_shared<SharedState>();
    };

    explicit Task(std::coroutine_handle<promise_type> handle, std::shared_ptr<SharedState> state)
        : handle(handle)
        , state(std::move(state)) {}

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& other) noexcept
        : handle(other.handle)
        , state(std::move(other.state)) {
        other.handle = nullptr;
    }

    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (handle) {
                handle.destroy();
            }
            handle = other.handle;
            state = std::move(other.state);
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
        if (!state) {
            return;
        }

        std::unique_lock lock(state->mtx);
        state->cv.wait(lock, [&] { return state->completed; });

        if (state->storedException) {
            std::rethrow_exception(state->storedException);
        }
    }

private:
    std::coroutine_handle<promise_type> handle = nullptr;
    std::shared_ptr<SharedState> state;
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
