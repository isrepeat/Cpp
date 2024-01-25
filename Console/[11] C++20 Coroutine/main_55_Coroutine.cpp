#include <Helpers/Logger.h>
#include <Helpers/Time.h>
#include <coroutine>
#include <iostream>
#include <thread>

template<typename Rep, typename Period>
auto operator co_await(std::chrono::duration<Rep, Period> duration) noexcept {
    struct Awaitable {
        explicit Awaitable(std::chrono::system_clock::duration duration)
            : duration{ duration }
        {
            LOG_FUNCTION_SCOPE("Awaitable(duration)");
        }

        ~Awaitable() {
            LOG_FUNCTION_SCOPE("~Awaitable()");
        }

        bool await_ready() const noexcept {
            LOG_FUNCTION_SCOPE("await_ready()");
            return duration.count() <= 0;
        }

        void await_resume() noexcept {
            LOG_FUNCTION_SCOPE("await_resume()");
        }

        void await_suspend(std::coroutine_handle<> h) {
            LOG_FUNCTION_SCOPE("await_suspend()");

            H::Timer::Once(duration, [h] {
                LOG_FUNCTION_SCOPE("Timer::Once__lambda()");
                h.resume();
                });
        }

    private:
        std::chrono::system_clock::duration duration;
    };

    return Awaitable{ duration };
}

class Task {
public:
    struct promise_type {
        auto get_return_object() noexcept {
            LOG_FUNCTION_SCOPE("get_return_object()");
            return Task{ std::coroutine_handle<promise_type>::from_promise(*this) };
        }

        auto initial_suspend() const noexcept {
            LOG_FUNCTION_SCOPE("initial_suspend()");
            //return std::suspend_always{};
            return std::suspend_never{};
        }

        void unhandled_exception() {
            LOG_FUNCTION_SCOPE("unhandled_exception()");
        }

        auto final_suspend() const noexcept {
            LOG_FUNCTION_SCOPE("final_suspend()");
            return std::suspend_never{};
        }

        void return_void() {
            LOG_FUNCTION_SCOPE("return_void()");
        }
    };

    void resume() {
        LOG_FUNCTION_SCOPE("resume()");
        if (coro_handle) {
            coro_handle.resume();
        }
    }

    ~Task() {
        LOG_FUNCTION_SCOPE("~Task()");
    }

private:
    Task() {
        LOG_FUNCTION_SCOPE("Task()");
    }
    explicit Task(std::coroutine_handle<> handle)
        : coro_handle(handle)
    {
        LOG_FUNCTION_SCOPE("Task(handle)");
    }

    std::coroutine_handle<> coro_handle;
};


Task tick() {
    LOG_FUNCTION_SCOPE("tick()");
    using namespace std::chrono_literals;

    LOG_DEBUG_D("First heavy computes ~1s ...");
    Sleep(1000);

    co_await 2s;
    LOG_DEBUG_D("... after 'co_await 2s'");

    LOG_DEBUG_D("Second heavy computes ~1s ...");
    Sleep(1000);

    co_await 2000ms;
    LOG_DEBUG_D("... after 'co_await 2000ms'");
}


void TestCoroutine() {
	LOG_FUNCTION_SCOPE("TestCoroutine()");
	
    LOG_DEBUG_D("actions before tick()");
    auto saveTask = tick();
	LOG_DEBUG_D("actions after tick()");

    LOG_DEBUG_D("wait 8s ...");
    Sleep(8'000);
}


int main() {
	lg::DefaultLoggers::Init("D:\\main_55_Coroutine.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);
    LOG_FUNCTION_SCOPE("main()");

	TestCoroutine();

	return 0;
}