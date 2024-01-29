#pragma once
#include <Helpers/Logger.h>
#include <Helpers/Time.h>
#include "Awaitables.h"
#include <coroutine>
#include <queue>

namespace std {
    template<typename T>
    struct coroutine_traits<std::unique_ptr<T>> {
        using promise_type = typename T::promise_type;
    };

    template<typename T, typename Caller>
    struct coroutine_traits<std::unique_ptr<T>, Caller> {
        using promise_type = typename T::promise_type;
    };
}

namespace Async {
    struct __PromiseInitialSuspendNever {
        auto initial_suspend() const noexcept {
            LOG_FUNCTION_SCOPE_VERBOSE("initial_suspend()");
            return std::suspend_never{}; // execute co-function body and return control to caller after first "suspend point"
        }
        auto final_suspend() const noexcept {
            LOG_FUNCTION_SCOPE_VERBOSE("final_suspend()");
            return std::suspend_never{};
        }
    };

    struct __PromiseInitialSuspendAlways {
        auto initial_suspend() const noexcept {
            LOG_FUNCTION_SCOPE_VERBOSE("initial_suspend()");
            return std::suspend_always{}; // return control to caller immediatly (do not execute co-function body)
        }
        auto final_suspend() const noexcept {
            LOG_FUNCTION_SCOPE_VERBOSE("final_suspend()");
            return std::suspend_never{};
        }
    };


    // Universal co-task class:
    // * PromiseBaseSuspendT must implement initial_suspend() and final_suspend() methods
    template<typename PromiseBaseSuspendT>
    class CoTask {
    public:
        using Ret_t = std::unique_ptr<CoTask>;

        struct promise_type : PromiseBaseSuspendT {
            promise_type() {
                LOG_FUNCTION_ENTER_VERBOSE("promise_type()");
            }
            ~promise_type() {
                LOG_FUNCTION_ENTER_VERBOSE("~promise_type()");
            }

            Ret_t get_return_object() noexcept {
                LOG_FUNCTION_SCOPE_VERBOSE("get_return_object()");
                // NOTE: declare CoTask ctor as public because unique_ptr need it
                return std::make_unique<CoTask>(std::coroutine_handle<promise_type>::from_promise(*this), token);
            }

            auto initial_suspend() const noexcept {
                LOG_FUNCTION_SCOPE_VERBOSE("initial_suspend()");
                return this->PromiseBaseSuspendT::initial_suspend();
            }

            void unhandled_exception() {
                LOG_FUNCTION_SCOPE_VERBOSE("unhandled_exception()");
            }

            auto final_suspend() const noexcept {
                LOG_FUNCTION_SCOPE_VERBOSE("final_suspend()");
                return this->PromiseBaseSuspendT::final_suspend();
            }

            void return_void() {
                LOG_FUNCTION_ENTER_VERBOSE("return_void()");
            }

        private:
            std::shared_ptr<int> token = std::make_shared<int>();
        };


        CoTask() {
            LOG_FUNCTION_ENTER_VERBOSE("CoTask()");
        }

        explicit CoTask(std::coroutine_handle<> coroHandle, std::weak_ptr<int> promiseToken)
            : coroHandle{ coroHandle }
            , promiseToken{ promiseToken }
        {
            LOG_FUNCTION_ENTER_VERBOSE("CoTask(coroHandle, promiseToken)");
        }

        ~CoTask() {
            LOG_FUNCTION_ENTER_VERBOSE("~CoTask()");
        }


        void resume() {
            LOG_FUNCTION_SCOPE_VERBOSE("resume()");

            if (promiseToken.expired()) {
                LOG_WARNING_D("promiseToken expired!");
                return;
            }
            if (!coroHandle) {
                LOG_WARNING_D("coroHandle is empty!");
                return;
            }
            coroHandle.resume();
        }

    private:
        std::coroutine_handle<> coroHandle;
        std::weak_ptr<int> promiseToken;
    };



    class AsyncTasks {
    public:
        //static constexpr std::string_view taskDescription = "Task_t";
        //static constexpr std::string_view workTokenDescription = "WorkToken_t";

        //using Task_t = typename CoTask<__PromiseInitialSuspendAlways, taskDescription>::Ret_t;
        //using WorkToken_t = typename CoTask<__PromiseInitialSuspendNever, workTokenDescription>::Ret_t;

        using Task_t = typename CoTask<__PromiseInitialSuspendAlways>::Ret_t;
        using WorkToken_t = typename CoTask<__PromiseInitialSuspendNever>::Ret_t;

        AsyncTasks() {
            LOG_FUNCTION_ENTER_VERBOSE("AsyncTasks()");
        }

        ~AsyncTasks() {
            LOG_FUNCTION_SCOPE_VERBOSE("~AsyncTasks()");
            // you need done all not started tasks (check whether need mutex lock here)
        }

        void SetResumeCallback(std::function<void(std::coroutine_handle<>)> resumeCallback) {
            LOG_FUNCTION_ENTER_VERBOSE("SetResumeCallback(resumeCallback)");
            this->resumeCallback = resumeCallback;
        }

        void Add(Task_t coTask) {
            LOG_FUNCTION_SCOPE_VERBOSE("Add(coTask)");
            std::unique_lock lk{ mx };
            tasks.push(std::move(coTask));
        }

        // CHECK: Should not be called untill previous StartExecutingCoroutine() not finished
        void StartExecuting() {
            LOG_FUNCTION_SCOPE_VERBOSE("StartExecuting()");
            auto workToken = StartExecutingCoroutine();
            return;
        }

    private:
        // TODO: Add multiple calls guard
        // TODO: Implement special CoTask for StartExecuting() to avoid resume this task from another thread
        WorkToken_t StartExecutingCoroutine() {
            LOG_FUNCTION_SCOPE_VERBOSE("StartExecutingCoroutine()");
            static thread_local std::size_t functionEnterThreadId = H::GetThreadId();

            while (auto task = GetNextTask()) {
                LOG_DEBUG_D("resume co-task ...");
                task->resume(); // here context is changed on co-task ...
                LOG_DEBUG_D("co-task finished");

                co_await InvokeCallbackAfter(2s, resumeCallback); // [suspend point] here control is returned to caller

                if (H::GetThreadId() != functionEnterThreadId) {
                    LOG_ERROR_D("It seems you resume this task from thread that differs from initial. Force return.");
                    co_return;
                }
            }
            co_return;
        }

        Task_t GetNextTask() {
            LOG_FUNCTION_SCOPE_VERBOSE("GetNextTask()");
            std::unique_lock lk{ mx };

            if (tasks.empty()) {
                return nullptr;
            }

            auto task = std::move(tasks.front());
            tasks.pop();
            return task;
        }

    private:
        std::mutex mx;
        std::queue<Task_t> tasks;
        std::function<void(std::coroutine_handle<>)> resumeCallback;
    };
}