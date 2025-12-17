#pragma once
#include "common.h"
//#include "Meta/FunctionTraits.h"

#include <condition_variable>
#include <type_traits>
#include <functional>
#include <optional>
#include <utility>
#include <cassert>
#include <chrono>
#include <mutex>

namespace CV {
    inline constexpr bool WAIT = false;
    inline constexpr bool NO_WAIT = true;
}

namespace HELPERS_NS {
    namespace detail {
        template <typename Callback>
        using CallbackResultT = std::invoke_result_t<Callback&>;

        template <typename Callback>
        inline CallbackResultT<Callback> CvExecuteCallbackAfterWaitWithPredicateInternal(
            std::function<bool(std::function<void()>)> predicate,
            Callback userCallback,
            bool executeCallbackInPredicate
        ) {
            using Result = CallbackResultT<Callback>;

            if constexpr (std::is_void_v<Result>) {
                if (executeCallbackInPredicate) {
                    predicate([&] {
                        userCallback();
                        });
                }
                else {
                    userCallback();
                }
            }
            else {
                if (executeCallbackInPredicate) {
                    std::optional<Result> resultOpt;

                    predicate([&] {
                        resultOpt.emplace(userCallback());
                        });

                    assert(resultOpt.has_value() && "Predicate did not execute callback but Result is required.");
                    return std::move(*resultOpt); // CHECK: how it works with std::vector and r-value
                }
                else {
                    return userCallback();
                }
            }
        }
    } // namespace detail


    template <typename Callback>
    std::invoke_result_t<Callback&> CvExecuteCallbackAfterWaitWithPredicate(
        std::unique_lock<std::mutex>& lk,
        std::condition_variable& cv,
        std::function<bool(std::function<void()>)> predicate,
        Callback userCallback,
        bool executeCallbackInPredicate = true
    ) {
        cv.wait(lk, [&] {
            return predicate(nullptr);
            });

        // if we here so Predicate(nullptr) returned true (CV::NO_WAIT)
        return detail::CvExecuteCallbackAfterWaitWithPredicateInternal<Callback>(
            std::move(predicate),
            std::move(userCallback),
            executeCallbackInPredicate
        );
    }

    template <
        class _Rep,
        class _Period,
        typename Callback
    >
    std::invoke_result_t<Callback&> CvExecuteCallbackAfterWaitWithPredicate(
        std::unique_lock<std::mutex>& lk,
        std::condition_variable& cv,
        const std::chrono::duration<_Rep, _Period>& waitTime,
        std::function<bool(std::function<void()>)> predicate,
        Callback userCallback,
        bool executeCallbackInPredicate = true
    ) {
        bool cvRes = cv.wait_for(lk, waitTime, [&] {
            return predicate(nullptr);
            });

        if (cvRes == CV::WAIT) {
            // timeout
        }

        // if we here so eather Predicate(nullptr) returned true or waitTime expired (CV::WAIT or CV::NO_WAIT)
        return detail::CvExecuteCallbackAfterWaitWithPredicateInternal<Callback>(
            std::move(predicate),
            std::move(userCallback),
            executeCallbackInPredicate
        );
    }
}