#pragma once
#include "FunctionTraits.hpp"
//#include "Logger.h"
#include <functional>
#include <ppltasks.h>
#include <chrono>
#include <mutex>

namespace {
    template <typename Callback, typename Result>
    Result CvExecuteCallbackAfterWaitWithPredicateInternal(
        std::function<bool(std::function<void()>)> Predicate, Callback userCallback, bool executeCallbackInPredicate)
    {
        if constexpr (std::is_same_v<Result, void>) {
            if (executeCallbackInPredicate) { // for usual case - execute userCallback where Predicate return 'true'
                Predicate([&] {
                    userCallback();
                    });
            }
            else {
                userCallback();
            }
        }
        else {
            if (executeCallbackInPredicate) {
                Result result;
                Predicate([&] {
                    result = userCallback();
                    });
                return result; // CHECK: how it works with std::vector and r-value
            }
            else {
                return userCallback();
            }
        }
    }
}


namespace CV {
    static const bool WAIT = false;
    static const bool NO_WAIT = true;
}

namespace H {
    template <typename Callback, typename Result = typename FunctionTraits<Callback>::Ret>
    Result CvExecuteCallbackAfterWaitWithPredicate(
        std::unique_lock<std::mutex>& lk, std::condition_variable& cv,
        std::function<bool(std::function<void()>)> Predicate, Callback userCallback, bool executeCallbackInPredicate = true)
    {
        cv.wait(lk, [&] {
            return Predicate(nullptr);
            });


        // if we here so Predicate(nullptr) returned true (CV::NO_WAIT)
        return CvExecuteCallbackAfterWaitWithPredicateInternal<Callback, Result>(Predicate, userCallback, executeCallbackInPredicate);
    }

    template <class _Rep, class _Period, typename Callback, typename Result = typename FunctionTraits<Callback>::Ret>
    Result CvExecuteCallbackAfterWaitWithPredicate(
        std::unique_lock<std::mutex>& lk, std::condition_variable& cv, const std::chrono::duration<_Rep, _Period>& waitTime,
        std::function<bool(std::function<void()>)> Predicate, Callback userCallback, bool executeCallbackInPredicate = true)
    {
        bool cvRes = cv.wait_for(lk, waitTime, [&] {
            return Predicate(nullptr);
            });

        if (cvRes == CV::WAIT) {
        }

        // if we here so eather Predicate(nullptr) returned true or waitTime expired (CV::WAIT or CV::NO_WAIT)
        return CvExecuteCallbackAfterWaitWithPredicateInternal<Callback, Result>(Predicate, userCallback, executeCallbackInPredicate);
    }

    class TaskChain {
    public:
        //CLASS_FULLNAME_LOGGING_INLINE_IMPLEMENTATION(TaskChain);

        TaskChain() = default;
        ~TaskChain();

        bool Reset();
        void Append(std::function<void()> taskLambda);
        void StartExecuting();
        void CancelAndWait();

    private:
        void ResetInternal();

    private:
        std::mutex mx;
        std::atomic<bool> canReset = true;
        std::atomic<bool> executing = false;

        Concurrency::cancellation_token_source cancelationToken;
        Concurrency::task_completion_event<void> taskCompletedEvent;
        Concurrency::task<void> task = Concurrency::task<void>(taskCompletedEvent, cancelationToken.get_token()); // must be created after completionEvent and token
    };
}
