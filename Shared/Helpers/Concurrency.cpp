#include "Concurrency.h"
#include "Helpers.h"

#define LogDebugWithFullClassNameA(...)
#define LogDebugWithFullClassNameW(...)

#define LogWarningWithFullClassNameA(...)
#define LogWarningWithFullClassNameW(...)

#define LogErrorWithFullClassNameA(...)
#define LogErrorWithFullClassNameW(...)

#define LOG_FUNCTION_ENTER(...)

#define LOG_DEBUG(...)
#define LOG_ERROR(...)

#define LOG_DEBUG_D(...)
#define LOG_ERROR_D(...)

#define LogLastError

namespace H {
    TaskChain::~TaskChain() {
        LogDebugWithFullClassNameA("~TaskChain() ...");
        CancelAndWait();
    }

    bool TaskChain::Reset() {
        std::lock_guard lk{ mx };
        if (!canReset) {
            LogWarningWithFullClassNameA("can't reset task chain");
            return false;
        }
        ResetInternal();
        return true;
    }

    void TaskChain::Append(std::function<void()> taskLambda) {
        std::lock_guard lk{ mx };
        LogDebugWithFullClassNameA("append new task lambda");
        task = task.then(taskLambda); // when completion event set all previous and next tasks execute immediatly
    }

    void TaskChain::StartExecuting() {
        std::lock_guard lk{ mx };
        if (executing.exchange(true))
            return; // return if previous value == true

        canReset = false;

        LogDebugWithFullClassNameA("start executing task chain");
        taskCompletedEvent.set(); // at this point tasks start executing in some thread via task scheduler
    }

    void TaskChain::CancelAndWait() {
        std::lock_guard lk{ mx };
        if (!executing.exchange(false))
            return; // return if previous value == false

        LogDebugWithFullClassNameA("cancel and wait task chain");
        if (task.is_done()) {
            LogWarningWithFullClassNameA("no wait, last task already completed");
        }

        cancelationToken.cancel();

        try {
            task.wait();
        }
        catch (const std::exception& ex) {
            LogErrorWithFullClassNameA("task std exception: {}", ex.what());
        }
        catch (...) {
            LogErrorWithFullClassNameA("task unrecognized exception !!!");
        }

        ResetInternal();
        canReset = true;
    }

    void TaskChain::ResetInternal() {
        LogDebugWithFullClassNameA("reset tasks chain");
        cancelationToken = {};
        taskCompletedEvent = {};
        task = Concurrency::task<void>(taskCompletedEvent, cancelationToken.get_token());
    }
}