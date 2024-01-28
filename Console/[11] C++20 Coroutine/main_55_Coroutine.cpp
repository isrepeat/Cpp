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
            LOG_FUNCTION_ENTER("Awaitable(duration)");
        }

        ~Awaitable() {
            LOG_FUNCTION_ENTER("~Awaitable()");
        }

        bool await_ready() const noexcept {
            LOG_FUNCTION_ENTER("await_ready()");
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
        promise_type() {
            LOG_FUNCTION_ENTER("promise_type()");
        }
        ~promise_type() {
            LOG_FUNCTION_ENTER("~promise_type()");
        }

        auto get_return_object() noexcept {
            LOG_FUNCTION_SCOPE("get_return_object()");
            return Task{ std::coroutine_handle<promise_type>::from_promise(*this), token };
        }

        auto initial_suspend() const noexcept {
            LOG_FUNCTION_SCOPE("initial_suspend()");
            return std::suspend_always{};
            //return std::suspend_never{};
        }

        void unhandled_exception() {
            LOG_FUNCTION_SCOPE("unhandled_exception()");
        }

        auto final_suspend() const noexcept {
            LOG_FUNCTION_SCOPE("final_suspend()");
            return std::suspend_never{};
        }

        void return_void() {
            LOG_FUNCTION_ENTER("return_void()");
        }

        std::shared_ptr<int> token = std::make_shared<int>();
    };

    void resume() {
        LOG_FUNCTION_SCOPE("resume()");

        if (promiseToken.expired()) {
            LOG_WARNING_D("promiseToken expired");
            return;
        }
        if (coro_handle) {
            coro_handle.resume();
        }
    }

    ~Task() {
        LOG_FUNCTION_ENTER("~Task()");
    }

private:
    Task() {
        LOG_FUNCTION_ENTER("Task()");
    }
    explicit Task(std::coroutine_handle<> handle, std::weak_ptr<int> promiseToken)
        : coro_handle(handle)
        , promiseToken{ promiseToken }
    {
        LOG_FUNCTION_ENTER("Task(handle)");
    }

    std::coroutine_handle<> coro_handle;
    std::weak_ptr<int> promiseToken;
};



Task TickTask() {
    LOG_FUNCTION_SCOPE("TickTask()");
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

void TestCoroutineTick() {
	LOG_FUNCTION_SCOPE("TestCoroutineTick()");
	
    LOG_DEBUG_D("actions before TickTask()");
    auto saveTask = TickTask();
	LOG_DEBUG_D("actions after TickTask()");

    LOG_DEBUG_D("wait 8s ...");
    Sleep(8'000);
}



Task DefferedTask() {
    LOG_FUNCTION_SCOPE("DefferedTask()");
    using namespace std::chrono_literals;

    LOG_DEBUG_D("Some heavy computes ~1s ...");
    Sleep(1000);

    co_return;
    LOG_DEBUG_D("... after 'co_await 2000ms'");
}

void TestCoroutineDefferedStart() {
    LOG_FUNCTION_SCOPE("TestCoroutineDefferedStart()");

    LOG_DEBUG_D("actions before DefferedTask()");
    auto saveTask = DefferedTask();
    LOG_DEBUG_D("actions after DefferedTask()");

    LOG_DEBUG_D("wait 2s ...");
    Sleep(2'000);

    saveTask.resume();

    LOG_DEBUG_D("wait 4s ...");
    Sleep(4'000);
}



int main() {
	lg::DefaultLoggers::Init("D:\\main_55_Coroutine.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);
    LOG_FUNCTION_SCOPE("main()");

    //TestCoroutineTick();
    TestCoroutineDefferedStart();

    system("pause");
	return 0;
}