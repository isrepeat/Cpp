//#include <Helpers/Logger.h>
//#include <Helpers/Time.h>
//#include <coroutine>
//#include <iostream>
//#include <thread>
//
//template<typename Rep, typename Period>
//auto operator co_await(std::chrono::duration<Rep, Period> duration) noexcept {
//    struct Awaitable {
//        explicit Awaitable(std::chrono::system_clock::duration duration)
//            : duration{ duration }
//        {
//            LOG_FUNCTION_ENTER("Awaitable(duration)");
//        }
//
//        ~Awaitable() {
//            LOG_FUNCTION_ENTER("~Awaitable()");
//        }
//
//        bool await_ready() const noexcept {
//            LOG_FUNCTION_ENTER("await_ready()");
//            return duration.count() <= 0;
//        }
//
//        void await_resume() noexcept {
//            LOG_FUNCTION_SCOPE("await_resume()");
//        }
//
//        void await_suspend(std::coroutine_handle<> h) {
//            LOG_FUNCTION_SCOPE("await_suspend()");
//
//            H::Timer::Once(duration, [h] {
//                LOG_FUNCTION_SCOPE("Timer::Once__lambda()");
//                h.resume();
//                });
//        }
//
//    private:
//        std::chrono::system_clock::duration duration;
//    };
//
//    return Awaitable{ duration };
//}
//
//class Task {
//public:
//    struct promise_type {
//        promise_type() {
//            LOG_FUNCTION_ENTER("promise_type()");
//        }
//        ~promise_type() {
//            LOG_FUNCTION_ENTER("~promise_type()");
//        }
//
//        auto get_return_object() noexcept {
//            LOG_FUNCTION_SCOPE("get_return_object()");
//            return Task{ std::coroutine_handle<promise_type>::from_promise(*this), token };
//        }
//
//        auto initial_suspend() const noexcept {
//            LOG_FUNCTION_SCOPE("initial_suspend()");
//            return std::suspend_always{}; // return control to caller immediatly (do not execute co-function body)
//            //return std::suspend_never{}; // execute co-function body and return control to caller after first "suspend point"
//        }
//
//        void unhandled_exception() {
//            LOG_FUNCTION_SCOPE("unhandled_exception()");
//        }
//
//        auto final_suspend() const noexcept {
//            LOG_FUNCTION_SCOPE("final_suspend()");
//            return std::suspend_always{}; // need manually destroy coroutine_handle
//            //return std::suspend_never{}; // coroutine_handle destroyed automatically after this method
//        }
//
//        void return_void() {
//            LOG_FUNCTION_ENTER("return_void()");
//        }
//
//        std::shared_ptr<int> token = std::make_shared<int>();
//    };
//
//    void resume() {
//        LOG_FUNCTION_SCOPE("resume()");
//        bool isDone = coro_handle.done();
//
//        if (canceled) {
//            LOG_WARNING_D("task canceled");
//            return;
//        }
//
//        if (promiseToken.expired()) {
//            LOG_WARNING_D("promiseToken expired");
//            return;
//        }
//
//        if (coro_handle) {
//            coro_handle.resume(); // change context on co-function ...
//        }
//    }
//
//    // May be called from any thread
//    void Cancel() {
//        canceled = true;
//    }
//
//    ~Task() {
//        LOG_FUNCTION_ENTER("~Task()");
//        bool isDone = coro_handle.done();
//        if (coro_handle) {
//            LOG_DEBUG_D("destroy coro_handle");
//            coro_handle.destroy(); // after this call promise_type will destroy (if final_suspend return suspend_always)
//        }
//    }
//
//private:
//    Task() {
//        LOG_FUNCTION_ENTER("Task()");
//    }
//    explicit Task(std::coroutine_handle<> handle, std::weak_ptr<int> promiseToken)
//        : coro_handle(handle)
//        , promiseToken{ promiseToken }
//    {
//        LOG_FUNCTION_ENTER("Task(handle)");
//    }
//
//    std::coroutine_handle<> coro_handle;
//    std::weak_ptr<int> promiseToken;
//    std::atomic<bool> canceled = false;
//};
//
//
//
//Task TickTask() {
//    LOG_FUNCTION_SCOPE("TickTask()");
//    using namespace std::chrono_literals;
//
//    LOG_DEBUG_D("First heavy computes ~1s ...");
//    Sleep(1000);
//
//    co_await 2s;
//    LOG_DEBUG_D("... after 'co_await 2s'");
//
//    LOG_DEBUG_D("Second heavy computes ~1s ...");
//    Sleep(1000);
//
//    co_await 2000ms;
//    LOG_DEBUG_D("... after 'co_await 2000ms'");
//}
//
//void TestCoroutineTick() {
//	LOG_FUNCTION_SCOPE("TestCoroutineTick()");
//	
//    LOG_DEBUG_D("actions before TickTask()");
//    auto saveTask = TickTask();
//	LOG_DEBUG_D("actions after TickTask()");
//
//    LOG_DEBUG_D("wait 8s ...");
//    Sleep(8'000);
//}
//
//
//
//Task DefferedTask() {
//    LOG_FUNCTION_SCOPE("DefferedTask()");
//    using namespace std::chrono_literals;
//
//    LOG_DEBUG_D("Some heavy computes ~1s ...");
//    Sleep(1000);
//    LOG_DEBUG_D("Some heavy computes ~1s ...");
//    Sleep(1000);
//    LOG_DEBUG_D("Some heavy computes ~1s ...");
//    Sleep(1000);
//
//    co_return;
//    LOG_DEBUG_D("... after 'co_await 2000ms'");
//}
//
//void TestCoroutineDefferedStart() {
//    LOG_FUNCTION_SCOPE("TestCoroutineDefferedStart()");
//
//    LOG_DEBUG_D("actions before DefferedTask()");
//    auto saveTask = DefferedTask();
//    LOG_DEBUG_D("actions after DefferedTask()");
//
//    LOG_DEBUG_D("wait 2s ...");
//    Sleep(2'000);
//
//    H::Timer::Once(1'500ms, [&saveTask] {
//        LOG_DEBUG_D("Cancel ...");
//        saveTask.Cancel();
//        });
//
//    saveTask.resume();
//
//
//    LOG_DEBUG_D("wait 4s ...");
//    Sleep(4'000);
//}
//
//
//#include <coroutine>
//
//class MyClass;
//
//namespace std {
//    template<typename T, typename... Args>
//    struct coroutine_traits<std::shared_ptr<T>, Args...> {
//        using promise_type = typename T::promise_type;
//    };
//
//    template<typename T, typename Caller, typename... Args>
//    struct coroutine_traits<std::shared_ptr<T>, Caller*, Args...> {
//        using promise_type = typename T::promise_type;
//    };
//}
//
//struct initial_suspend_always {
//    std::suspend_never initial_suspend() { return {}; }
//};
//
//template <typename Caller>
//struct task {
//    struct promise_type : initial_suspend_always {
//#ifdef __INTELLISENSE__ // https://stackoverflow.com/questions/67209981/weird-error-from-visual-c-no-default-constructor-for-promise-type
//        // Not use default Ctor, just mark it for intelli sense
//        promise_type();
//#endif
//        // Used for non-class functions
//        promise_type(int) {
//            int xx = 9;
//        }
//
//        // Used for Caller class methods
//        template <typename Caller>
//        promise_type(Caller&, int) {
//            int xx = 9;
//        }
//
//        //std::shared_ptr<task> get_return_object() { return std::make_shared<task>(); }
//        task get_return_object() { return task{}; }
//        std::suspend_never final_suspend() noexcept { return {}; }
//        void return_void() {}
//        void unhandled_exception() {}
//    };
//
//    task() {
//    }
//    ////explicit CoTask(std::coroutine_handle<> coroHandle, std::weak_ptr<int> promiseToken, std::wstring coroFrameName)
//    //explicit task(std::coroutine_handle<> coroHandle, std::weak_ptr<int> promiseToken)
//    //{
//    //}
//    //~task() {
//    //}
//    //task(task&& other)
//    //{
//    //}
//    //task& operator=(task&& other) {
//    //    return *this;
//    //}
//
//    //task(task const&) = delete;
//    //task& operator=(task const&) = delete;
//};
//
//class MyClass {
//public:
//    MyClass() {}
//
//    //std::shared_ptr<task<MyClass>> foo(int) {
//    task<MyClass> foo(int) {
//    //task foo(int) {
//        co_await std::suspend_never{};
//    }
//
//};
//
//task<void> bar(int) {
//    co_await std::suspend_never{};
//}
//
//
//int main() {
//	lg::DefaultLoggers::Init("D:\\main_55_Coroutine.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);
//    LOG_FUNCTION_SCOPE("main()");
//    MyClass myClass;
//    myClass.foo(12);
//    bar(12);
//    //TestCoroutineTick();
//    TestCoroutineDefferedStart();
//
//    system("pause");
//	return 0;
//}