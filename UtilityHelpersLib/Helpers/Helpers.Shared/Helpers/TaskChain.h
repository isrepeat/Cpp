#pragma once
#include "common.h"
#include "FunctionTraits.hpp"
#include "Logger.h"

#include <functional>
#include <ppltasks.h>
#include <mutex>

namespace HELPERS_NS {
    // NOTE: DerivedClass must implement BeforeStartCallback(...) and AfterFinishCallback()
    template <typename DerivedClass, typename... Args>
    class _TaskChainBase {
        CLASS_FULLNAME_LOGGING_INLINE_IMPLEMENTATION(_TaskChainBase);

    protected:
        _TaskChainBase() = default;

        // _TaskChainBase should not be used as interface directly, so it makes no sense declare Dtor as virtual
        ~_TaskChainBase() {
            LOG_FUNCTION_SCOPE_C("~_TaskChainBase()");
            CancelAndWaitInternal(); // not call "virtual" members (like AfterFinishCallback()) child part already destroyed
        }

    public:
        bool Reset() {
            LOG_FUNCTION_ENTER_C("Reset()");

            std::lock_guard lk{ mx };
            if (!canReset) {
                LOG_WARNING_D("can't reset task chain");
                return false;
            }
            ResetInternal();
            return true;
        }

        void Append(std::function<void()> taskLambda) {
            LOG_FUNCTION_ENTER_C("Append(taskLambda)");

            std::lock_guard lk{ mx };
            task = task.then(taskLambda); // when completion event set all previous and next tasks execute immediatly
        }

        void StartExecuting(Args... args) {
            LOG_FUNCTION_ENTER_C("StartExecuting(...)");

            std::lock_guard lk{ mx };
            if (executing.exchange(true)) {
                LOG_WARNING_D("already executing, ignore");
                return; // return if previous value == true
            }

            canReset = false;
            static_cast<DerivedClass*>(this)->BeforeStartCallback(std::forward<Args&&>(args)...);

            LOG_DEBUG("start executing task chain");
            taskCompletedEvent.set(); // at this point tasks start executing in some thread via task scheduler
        }

        void CancelAndWait() {
            LOG_FUNCTION_ENTER_C("CancelAndWait()");
            CancelAndWaitInternal();

            static_cast<DerivedClass*>(this)->AfterFinishCallback();
            ResetInternal();
            canReset = true;
        }

    private:
        void CancelAndWaitInternal() {
            LOG_FUNCTION_ENTER_C("CancelAndWaitInternal()");

            std::lock_guard lk{ mx };
            if (!executing.exchange(false)) {
                LOG_WARNING_D("already finished, ignore");
                return; // return if previous value == false
            }

            if (task.is_done()) {
                LOG_DEBUG_D("no need wait, last task already completed");
            }

            cancelationToken.cancel();

            try {
                task.wait();
            }
            catch (const std::exception& ex) {
                LOG_ERROR_D("Catch st::exception: {}", ex.what());
            }
            catch (...) {
                LOG_ERROR_D("Catch unrecognized exception !!!");
            }
        }

        void ResetInternal() {
            LOG_FUNCTION_ENTER_C("ResetInternal()");
            cancelationToken = {};
            taskCompletedEvent = {};
            task = Concurrency::task<void>(taskCompletedEvent, cancelationToken.get_token());
        }

    private:
        std::mutex mx;
        std::atomic<bool> canReset = true;
        std::atomic<bool> executing = false;

        Concurrency::cancellation_token_source cancelationToken;
        Concurrency::task_completion_event<void> taskCompletedEvent;
        Concurrency::task<void> task = Concurrency::task<void>(taskCompletedEvent, cancelationToken.get_token()); // must be created after completionEvent and token
    };


    template <typename T>
    class TaskChain : public _TaskChainBase<TaskChain<T>, T> {
        friend _TaskChainBase<TaskChain<T>, T>;
    public:
        static constexpr std::string_view templateNotes = "Primary template: T is startResult";

        TaskChain() = default;
        ~TaskChain() {
            LOG_FUNCTION_ENTER_C("~TaskChain()");
        }

        const T& GetStartResult() {
            static T defaultResult;

            if (LOG_ASSERT(startResult, "startResult is nullptr")) {
                return defaultResult;
            }
            return *startResult;
        }

    private:
        void BeforeStartCallback(const T& startResult) {
            this->startResult = std::make_unique<T>(startResult);
        }
        void AfterFinishCallback() {
            startResult = nullptr;
        }

    private:
        std::unique_ptr<T> startResult;
    };


    template <>
    class TaskChain<void> : public _TaskChainBase<TaskChain<void>> {
        friend _TaskChainBase<TaskChain<void>>;
    public:
        static constexpr std::string_view templateNotes = "Specialization: T = void";

        TaskChain() = default;
        ~TaskChain() = default;

    private:
        void BeforeStartCallback() {}
        void AfterFinishCallback() {}
    };
}