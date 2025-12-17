#pragma once
#include <Helpers/common.h>
#include "Helpers/Event/Signal.h"
#include <Helpers/Logger.h>
#include <Helpers/Time.h>
#include "CoTask.h"

#include <coroutine>
#include <optional>
#include <memory>

// Don't forget return original definitions for these macros at the end of file
#define LOG_FUNCTION_ENTER_VERBOSE(fmt, ...)
#define LOG_FUNCTION_SCOPE_VERBOSE(fmt, ...)
#define LOG_FUNCTION_ENTER_VERBOSE_C(fmt, ...)
#define LOG_FUNCTION_SCOPE_VERBOSE_C(fmt, ...)

namespace HELPERS_NS {
    namespace Async {
        // TODO: Add type_traits to detect check if caller promise_type does not have Ctor(..., resumeCallback).
        // NOTE: Must be called from coroutine that accepts resumeCallback as parameter.
        //       (resumeCallback will passed to this task::promise_type Ctor)
        template <typename PromiseT>
        auto ResumeAfter(std::chrono::milliseconds duration) noexcept {
            LOG_FUNCTION_SCOPE_VERBOSE("ResumeAfter(duration)");

            struct Awaitable {
                explicit Awaitable(std::chrono::milliseconds duration)
                    : duration{ duration }
                {
                    LOG_FUNCTION_ENTER_VERBOSE("Awaitable(duration)");
                }
                ~Awaitable() {
                    LOG_FUNCTION_ENTER_VERBOSE("~Awaitable()");
                }

                bool await_ready() const noexcept {
                    LOG_FUNCTION_ENTER_VERBOSE("await_ready()");
                    return duration.count() <= 0;
                }

                void await_suspend(std::coroutine_handle<PromiseT> callerCoroutine) {
                    LOG_FUNCTION_SCOPE_VERBOSE("await_suspend(callerCoroutine)");

                    std::weak_ptr<CoTaskBase> coTaskWeak = callerCoroutine.promise().get_task();
                    auto resumeCallback = callerCoroutine.promise().get_resume_callback();

                    if (LOG_ASSERT(resumeCallback, "resumeCallback is empty!")) { // This can happen if you call this function not in PromiseRoot task coroutine
                        LOG_WARNING_D("resume callerCoroutine ...");
                        callerCoroutine.resume();
                        LOG_DEBUG_D("callerCoroutine finished");
                        return;
                    }

                    HELPERS_NS::Timer::Once(duration, [resumeCallback, coTaskWeak] {
                        LOG_FUNCTION_SCOPE_VERBOSE("Timer::Once__lambda()");
                        resumeCallback(coTaskWeak);
                        });
                }

                void await_resume() noexcept {
                    LOG_FUNCTION_SCOPE_VERBOSE("await_resume()");
                }

            private:
                std::chrono::system_clock::duration duration;
            };

            return Awaitable{ duration };
        }

        template<typename ResultT>
        struct SignalAwaitableResult {
            template<typename ResumeSignalT>
            void AddFinish(
				ResumeSignalT& resumeSignal,
				std::function<void(std::weak_ptr<CoTaskBase>)> resumeCallback,
				std::weak_ptr<CoTaskBase> coTaskWeak
			) {
                resumeSignal->AddFinish([this, resumeCallback, coTaskWeak] (ResultT result) {
                    LOG_FUNCTION_SCOPE_VERBOSE("SignalAwaitableResult<ResultT>::AddFinish__lambda()");
                    this->result.emplace(std::move(result));
                    resumeCallback(coTaskWeak);
                    });
            }

            ResultT DetachResult() {
                LOG_FUNCTION_SCOPE_VERBOSE("SignalAwaitableResult<ResultT>::DetachResult__lambda()");
                return std::move(*this->result);
            }

        private:
            std::optional<ResultT> result;
        };

        template<>
        struct SignalAwaitableResult<void> {
            template<typename ResumeSignalT>
            void AddFinish(
				ResumeSignalT& resumeSignal,
				std::function<void(std::weak_ptr<CoTaskBase>)> resumeCallback,
				std::weak_ptr<CoTaskBase> coTaskWeak
			) {
                resumeSignal->AddFinish([resumeCallback, coTaskWeak]() {
                    LOG_FUNCTION_SCOPE_VERBOSE("SignalAwaitableResult<void>::AddFinish__lambda()");
                    resumeCallback(coTaskWeak);
                    });
            }

            void DetachResult() {
                LOG_FUNCTION_SCOPE_VERBOSE("SignalAwaitableResult<void>::DetachResult__lambda()");
            }
        };

        template<typename ResultT>
        struct SignalAwaitable : public SignalAwaitableResult<ResultT> {
            explicit SignalAwaitable(
				std::function<void(std::weak_ptr<HELPERS_NS::Event::Signal<void(ResultT)>>)> asyncOperation
			)
                : asyncOperation{ asyncOperation } {
                LOG_FUNCTION_ENTER_VERBOSE("Awaitable(asyncOperationTask)");
            }

            ~SignalAwaitable() {
                LOG_FUNCTION_ENTER_VERBOSE("~Awaitable()");
            }

            bool await_ready() const noexcept {
                LOG_FUNCTION_ENTER_VERBOSE("await_ready()");
                return suspend::always;
            }

            void await_suspend(std::coroutine_handle<PromiseDefault> callerCoroutine) {
                LOG_FUNCTION_SCOPE_VERBOSE("await_suspend(callerCoroutine)");

                std::weak_ptr<CoTaskBase> coTaskWeak = callerCoroutine.promise().get_task();
                auto resumeSignalWeak = callerCoroutine.promise().get_resume_signal();
                auto resumeCallback = callerCoroutine.promise().get_resume_callback();

                auto resumeSignal = resumeSignalWeak.lock();
                if (LOG_ASSERT(resumeSignal, "resumeSignalWeak expired!") ||
                    LOG_ASSERT(resumeCallback, "resumeCallback is empty!")) // This can happen if you call this function not in PromiseRoot task coroutine
                {
                    LOG_WARNING_D("resume callerPromiseCoroHandle ...");
                    callerCoroutine.resume();
                    LOG_DEBUG_D("callerPromiseCoroHandle finished");
                    return;
                }

                this->AddFinish(resumeSignal, resumeCallback, coTaskWeak);

                asyncOperation(resumeSignalWeak);
            }

            template<typename ReturnT>
            void await_suspend(std::coroutine_handle<PromiseWithResult<ReturnT>> callerCoroutine) {
                LOG_FUNCTION_SCOPE_VERBOSE("await_suspend(callerCoroutine)");

                std::weak_ptr<CoTaskBase> coTaskWeak = callerCoroutine.promise().get_task();
                auto resumeSignalWeak = callerCoroutine.promise().get_resume_signal();
                auto resumeCallback = callerCoroutine.promise().get_resume_callback();

                auto resumeSignal = resumeSignalWeak.lock();
                if (LOG_ASSERT(resumeSignal, "resumeSignalWeak expired!") ||
                    LOG_ASSERT(resumeCallback, "resumeCallback is empty!")) // This can happen if you call this function not in PromiseRoot task coroutine
                {
                    LOG_WARNING_D("resume callerPromiseCoroHandle ...");
                    callerCoroutine.resume();
                    LOG_DEBUG_D("callerPromiseCoroHandle finished");
                    return;
                }

                this->AddFinish(resumeSignal, resumeCallback, coTaskWeak);

                asyncOperation(resumeSignalWeak);
            }

            auto await_resume() {
                LOG_FUNCTION_SCOPE_VERBOSE("await_resume()");
                return this->DetachResult();
            }

        private:
            std::function<void(std::weak_ptr<HELPERS_NS::Event::Signal<void(ResultT)>>)> asyncOperation;
        };

        inline auto AsyncOperationWithResumeSignal(
			std::function<void(std::weak_ptr<HELPERS_NS::Event::Signal<void()>>)> asyncOperation
		) noexcept {
            LOG_FUNCTION_SCOPE_VERBOSE("AsyncOperationWithResumeSignal(asyncOperationTask)");
            return SignalAwaitable<void>{ asyncOperation };
        }

        template<typename ResultT>
        auto AsyncOperationWithResumeSignal(
			std::function<void(std::weak_ptr<HELPERS_NS::Event::Signal<void(ResultT)>>)> asyncOperation
		) noexcept {
            LOG_FUNCTION_SCOPE_VERBOSE("AsyncOperationWithResumeSignal(asyncOperationTask)");
            return SignalAwaitable<ResultT>{ asyncOperation };
        }
    } // namespace Async
} // namespace HELPERS_NS


// NOTE: Do not encapsulate overloaded global operator co_await in namespace by design to simplify use
// TODO: Add guards for expired promise / task
template<typename Rep, typename Period>
auto operator co_await(std::chrono::duration<Rep, Period> duration) noexcept {
    LOG_FUNCTION_SCOPE_VERBOSE("operator co_await(duration)");

    struct Awaitable {
        explicit Awaitable(std::chrono::system_clock::duration duration)
            : duration{ duration }
        {
            LOG_FUNCTION_ENTER_VERBOSE("Awaitable(duration)");
        }

        ~Awaitable() {
            LOG_FUNCTION_ENTER_VERBOSE("~Awaitable()");
        }

        bool await_ready() const noexcept {
            LOG_FUNCTION_ENTER_VERBOSE("await_ready()");
            return duration.count() <= 0;
        }

        void await_resume() noexcept {
            LOG_FUNCTION_SCOPE_VERBOSE("await_resume()");
        }

        void await_suspend(std::coroutine_handle<> coroHandle) {
            LOG_FUNCTION_SCOPE_VERBOSE("await_suspend(coroHandle)");

            HELPERS_NS::Timer::Once(duration, [coroHandle] {
                LOG_FUNCTION_SCOPE_VERBOSE("Timer::Once__lambda()");
                coroHandle.resume();
                });
        }

    private:
        std::chrono::system_clock::duration duration;
    };

    return Awaitable{ duration };
}

#if !defined(DISABLE_VERBOSE_LOGGING)
#define LOG_FUNCTION_ENTER_VERBOSE(fmt, ...) LOG_FUNCTION_ENTER(fmt, ##__VA_ARGS__)
#define LOG_FUNCTION_SCOPE_VERBOSE(fmt, ...) LOG_FUNCTION_SCOPE(fmt, ##__VA_ARGS__)

#define LOG_FUNCTION_ENTER_VERBOSE_C(fmt, ...) LOG_FUNCTION_ENTER_C(fmt, ##__VA_ARGS__)
#define LOG_FUNCTION_SCOPE_VERBOSE_C(fmt, ...) LOG_FUNCTION_SCOPE_C(fmt, ##__VA_ARGS__)
#else
#define LOG_FUNCTION_ENTER_VERBOSE(fmt, ...)
#define LOG_FUNCTION_SCOPE_VERBOSE(fmt, ...)

#define LOG_FUNCTION_ENTER_VERBOSE_C(fmt, ...)
#define LOG_FUNCTION_ENTER_VERBOSE_C(fmt, ...)
#endif