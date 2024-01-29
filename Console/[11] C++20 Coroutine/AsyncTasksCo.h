#pragma once
#include <Helpers/Logger.h>
#include <Helpers/Time.h>
#include <coroutine>
#include <queue>

namespace std {
    template<typename T>
    struct coroutine_traits<std::unique_ptr<T>> {
        using promise_type = typename T::promise_type;
    };
}

namespace Async {
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
                // NOTE: declare Task ctor as public because unique_ptr need it
                return std::make_unique<Task>(std::coroutine_handle<promise_type>::from_promise(*this), token);
            }

            auto initial_suspend() const noexcept {
                LOG_FUNCTION_SCOPE("initial_suspend()");
                return std::suspend_always{};
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

        private:
            std::shared_ptr<int> token = std::make_shared<int>();
        };

    Task() {
        LOG_FUNCTION_ENTER("Task()");
    }

    explicit Task(std::coroutine_handle<> coroHandle, std::weak_ptr<int> promiseToken)
        : coroHandle{ coroHandle }
        , promiseToken{ promiseToken }
    {
        LOG_FUNCTION_ENTER("Task(coroHandle, promiseToken)");
    }

    ~Task() {
        LOG_FUNCTION_ENTER("~Task()");
    }

    void resume() {
        LOG_FUNCTION_SCOPE("resume()");

        if (promiseToken.expired()) {
            LOG_WARNING_D("promiseToken expired");
            return;
        }
        if (!coroHandle) {
            LOG_WARNING_D("coroHandle empty");
            return;
        }
        coroHandle.resume();
    }

    private:
        std::coroutine_handle<> coroHandle;
        std::weak_ptr<int> promiseToken;
    };



    class TaskManager {
    public:
        TaskManager() {
            LOG_FUNCTION_ENTER("TaskManager()");
        }

        ~TaskManager() {
            LOG_FUNCTION_SCOPE("~TaskManager()");
        }

    private:
        std::queue<std::unique_ptr<Task>> tasks;
    };
}