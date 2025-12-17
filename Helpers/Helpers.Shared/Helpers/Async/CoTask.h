#pragma once
#include <Helpers/common.h>
#include "Helpers/Event/Signal.h"
#include <Helpers/Thread.h>
#include <Helpers/Logger.h>

#include <type_traits>
#include <coroutine>
#include <optional>

// Don't forget return original definitions for these macros at the end of file
#define LOG_FUNCTION_ENTER_VERBOSE(fmt, ...)
#define LOG_FUNCTION_SCOPE_VERBOSE(fmt, ...)
#define LOG_FUNCTION_ENTER_VERBOSE_C(fmt, ...)
#define LOG_FUNCTION_SCOPE_VERBOSE_C(fmt, ...)

namespace std {
    template<typename T, typename... Args>
    struct coroutine_traits<std::shared_ptr<T>, Args...> {
        using promise_type = typename T::promise_type;
    };

    template<typename T, typename Caller, typename... Args>
    struct coroutine_traits<std::shared_ptr<T>, Caller*, Args...> {
        using promise_type = typename T::promise_type;
    };
}

namespace HELPERS_NS {
    namespace Async {
#ifdef __INTELLISENSE__ // https://stackoverflow.com/questions/67209981/weird-error-from-visual-c-no-default-constructor-for-promise-type
#define INTELLISENSE_DEFAULT_CTOR(className) className()
#else
#define INTELLISENSE_DEFAULT_CTOR(className)
#endif

        struct suspend {
            static constexpr bool always = false; // call await_suspend()
            static constexpr bool never = true; // resume immediately (not call await_suspend())
        };

        struct initial_suspend_never {
            auto initial_suspend() const noexcept {
                LOG_FUNCTION_SCOPE_VERBOSE("initial_suspend()");
                return std::suspend_never{}; // execute co-function body and return control to caller after first "suspend point"
            }
        };
        struct initial_suspend_always {
            auto initial_suspend() const noexcept {
                LOG_FUNCTION_SCOPE_VERBOSE("initial_suspend()");
                return std::suspend_always{}; // return control to caller immediatly (do not execute co-function body)
            }
        };

        struct InstanceName {
            InstanceName(const wchar_t* name) : name(name) {}
            InstanceName(std::wstring name) : name(name) {}
            std::wstring name;
        };

        /*------------------------*/
        /*      PROMISE BASE      */
        /*------------------------*/
        class CoTaskBase;

        template <typename PromiseImplT>
        class CoTask;

        struct LambdaBindCoroKey {};

        template<typename ReturnT>
        class PromiseResult {
        public:
            void return_value(ReturnT result) {
                LOG_FUNCTION_ENTER_VERBOSE_C("return_value()");
                this->result.emplace(std::move(result));
            }

            ReturnT detach_result() {
                LOG_FUNCTION_ENTER_VERBOSE_C("detach_result()");
                return std::move(*this->result);
            }

        protected:
            std::optional<ReturnT> result;
        };

        template<>
        class PromiseResult<void> {
        public:
            void return_void() {
                LOG_FUNCTION_ENTER_VERBOSE_C("return_void()");
            }

            void detach_result() {
                LOG_FUNCTION_ENTER_VERBOSE_C("detach_void_result()");
            }
        };

        template <typename PromiseImplT, typename ReturnT>
        class Promise : public PromiseResult<ReturnT> {
            CLASS_FULLNAME_LOGGING_INLINE_IMPLEMENTATION(Promise);
        public:
            using ObjectRet_t = std::shared_ptr<CoTask<PromiseImplT>>;
            using _ReturnT = ReturnT;

            class FinalAwaiter {
                CLASS_FULLNAME_LOGGING_INLINE_IMPLEMENTATION(FinalAwaiter);
            public:
                FinalAwaiter(InstanceName instanceName) {
                    this->SetFullClassNameSilent(instanceName.name);
                    LOG_FUNCTION_ENTER_VERBOSE_C(L"FinalAwaiter()");
                }
                ~FinalAwaiter() {
                    LOG_FUNCTION_ENTER_VERBOSE_C(L"~FinalAwaiter()");
                }
                bool await_ready() noexcept {
                    return suspend::always;
                }
                // NOTE: currentCoroutine associated with current co-function context where was created Promise
                std::coroutine_handle<> await_suspend(std::coroutine_handle<PromiseImplT> currentCoroutine) noexcept {
                    LOG_FUNCTION_SCOPE_VERBOSE_C("await_suspend(currentCoroutine)");
                    if (currentCoroutine.promise().previousCoroutine) {
                        return currentCoroutine.promise().previousCoroutine;
                    }
                    return std::noop_coroutine();
                }

                void await_resume() noexcept {
                    LOG_FUNCTION_ENTER_VERBOSE_C("await_resume()");
                    assertm(false, "Unexpected call");
                    std::terminate();
                }
            };

            // Used for non-class functions
            Promise(InstanceName instanceName) {
                this->SetFullClassNameSilent(instanceName.name);
                LOG_FUNCTION_ENTER_VERBOSE_C(L"Promise()");
            }

            // Used for Caller class methods
            template <typename Caller>
            Promise(Caller&, InstanceName instanceName) {
                this->SetFullClassNameSilent(instanceName.name);
                LOG_FUNCTION_ENTER_VERBOSE_C(L"Promise(Caller)");
            }

            ~Promise() {
                LOG_FUNCTION_ENTER_VERBOSE_C("~Promise()");
            }

            ObjectRet_t get_return_object() noexcept {
                LOG_FUNCTION_SCOPE_VERBOSE_C("get_return_object()");
                auto coTaskShared = std::make_shared<CoTask<PromiseImplT>>(
                    std::coroutine_handle<PromiseImplT>::from_promise(static_cast<PromiseImplT&>(*this)),
                    token,
                    this->GetFullClassNameW()
                    );
                coTaskWeak = coTaskShared;
                return coTaskShared;
            }

            auto initial_suspend() const noexcept {
                LOG_FUNCTION_SCOPE_VERBOSE_C("initial_suspend()");
                return std::suspend_always{};
            }
            void unhandled_exception() {
                LOG_FUNCTION_SCOPE_VERBOSE_C("unhandled_exception()");
                // NOTE: By rethrowing an exception we break the tasks chain (if it exists).
                //       This behavior is by design because next tasks may depend on current.
                std::rethrow_exception(std::current_exception()); // "root resumer" must handle exceptions.
            }
            auto final_suspend() const noexcept {
                LOG_FUNCTION_SCOPE_VERBOSE_C("final_suspend()");
                // NOTE: "suspend always" at finish mean that we need to destroy coroutine_handle manually.
                //        This is need to avoid double destroying problem in CoTask RAII class.
                return FinalAwaiter{ this->GetFullClassNameW() };
            }

            template <typename PromiseT>
            void remember_other_coroutine(std::coroutine_handle<PromiseT> otherCoroutine) {
                this->resumeCallback = otherCoroutine.promise().get_resume_callback();
                this->previousCoroutine = otherCoroutine;
            }

            void cancel() {
                canceled = true;
            }
            bool is_canceled() {
                return canceled;
            }

            std::function<void(std::weak_ptr<CoTaskBase>)> get_resume_callback() {
                return resumeCallback;
            }
            std::weak_ptr<CoTaskBase> get_task() {
                return coTaskWeak;
            }
            std::weak_ptr<int> get_token() {
                return token;
            }

        protected:
            std::function<void(std::weak_ptr<CoTaskBase>)> resumeCallback; // can be initialized in derived classes

        private:
            std::atomic<bool> canceled = false;
            std::coroutine_handle<> previousCoroutine;
            std::weak_ptr<CoTaskBase> coTaskWeak;
            std::shared_ptr<int> token = std::make_shared<int>();
        };


        /*------------------------------*/
        /*   PROMISES IMPLEMENTATIONS   */
        /*------------------------------*/
        class PromiseDefault : public Promise<PromiseDefault, void> {
        public:
            using _MyBase = Promise<PromiseDefault, void>;

            template <typename... Args>
            PromiseDefault(Args&... args)
                : PromiseDefault(InstanceName{ L"co-task" }, args...) // NOTE: use InstanceName explicitly to help compiler deduction
            {}

            template <typename... Args>
            PromiseDefault(InstanceName instanceName, Args&...)
                : _MyBase(instanceName)
                , resumeSignal{ std::make_shared<HELPERS_NS::Event::Signal<void()>>() }
            {
                this->SetFullClassNameSilent(instanceName.name);
                LOG_FUNCTION_ENTER_VERBOSE_C(L"PromiseDefault()");
            }

            template <typename Caller, typename... Args>
            PromiseDefault(Caller& caller, Args&... args)
                : PromiseDefault(caller, InstanceName{ L"co-task" }, args...)
            {}

            template <typename Caller, typename... Args>
            PromiseDefault(Caller& caller, InstanceName instanceName, Args&...)
                : _MyBase(caller, instanceName)
                , resumeSignal{ std::make_shared<HELPERS_NS::Event::Signal<void()>>() }
            {
                this->SetFullClassNameSilent(instanceName.name);
                LOG_FUNCTION_ENTER_VERBOSE_C(L"PromiseDefault(Caller)");
            }

            // must be used only by LambdaBindCoro helper
            template <typename LambdaT, typename... Args>
            PromiseDefault(LambdaBindCoroKey key, LambdaT& lambda, Args&...)
                : _MyBase(L"LambdaBindCoroKey")
                , resumeSignal{ std::make_shared<HELPERS_NS::Event::Signal<void()>>() }
            {
                this->SetFullClassNameSilent(L"LambdaBindCoroKey");
                LOG_FUNCTION_ENTER_VERBOSE_C(L"PromiseDefault(LambdaCtorKey, LambdaT)");
            }

            std::weak_ptr<HELPERS_NS::Event::Signal<void()>> get_resume_signal() {
                return resumeSignal;
            }

        private:
            std::shared_ptr<HELPERS_NS::Event::Signal<void()>> resumeSignal;
        };

        template<typename ReturnT>
        class PromiseWithResult : public Promise<PromiseWithResult<ReturnT>, ReturnT> {
        public:
            using _MyBase = Promise<PromiseWithResult<ReturnT>, ReturnT>;

            template <typename... Args>
            PromiseWithResult(Args&... args)
                : PromiseWithResult(InstanceName{ L"co-task" }, args...) // NOTE: use InstanceName explicitly to help compiler deduction
            {}

            template <typename... Args>
            PromiseWithResult(InstanceName instanceName, Args&...)
                : _MyBase(instanceName)
                , resumeSignal{ std::make_shared<HELPERS_NS::Event::Signal<void(ReturnT)>>() }
            {
                this->SetFullClassNameSilent(instanceName.name);
                LOG_FUNCTION_ENTER_VERBOSE_C(L"PromiseWithResult()");
            }

            template <typename Caller, typename... Args>
            PromiseWithResult(Caller& caller, Args&... args)
                : PromiseWithResult(caller, InstanceName{ L"co-task" }, args...)
            {}

            template <typename Caller, typename... Args>
            PromiseWithResult(Caller& caller, InstanceName instanceName, Args&...)
                : _MyBase(caller, instanceName)
                , resumeSignal{ std::make_shared<HELPERS_NS::Event::Signal<void(ReturnT)>>() }
            {
                this->SetFullClassNameSilent(instanceName.name);
                LOG_FUNCTION_ENTER_VERBOSE_C(L"PromiseWithResult(Caller)");
            }

            // must be used only by LambdaBindCoro helper
            template <typename LambdaT, typename... Args>
            PromiseWithResult(LambdaBindCoroKey key, LambdaT& lambda, Args&...)
                : _MyBase(L"LambdaBindCoroKey")
                , resumeSignal{ std::make_shared<HELPERS_NS::Event::Signal<void(ReturnT)>>() }
            {
                this->SetFullClassNameSilent(L"LambdaBindCoroKey");
                LOG_FUNCTION_ENTER_VERBOSE_C(L"PromiseWithResult(LambdaCtorKey, LambdaT)");
            }

            std::weak_ptr<HELPERS_NS::Event::Signal<void(ReturnT)>> get_resume_signal() {
                return resumeSignal;
            }

        private:
            std::shared_ptr<HELPERS_NS::Event::Signal<void(ReturnT)>> resumeSignal;
        };


        class PromiseRoot : public Promise<PromiseRoot, void> {
        public:
            using _MyBase = Promise<PromiseRoot, void>;
            INTELLISENSE_DEFAULT_CTOR(PromiseRoot);

            template <typename... Args>
            PromiseRoot(InstanceName instanceName, std::function<void(std::weak_ptr<CoTaskBase>)> resumeCallback, Args&... args)
                : _MyBase(instanceName)
            {
                this->SetFullClassNameSilent(instanceName.name);
                LOG_FUNCTION_ENTER_VERBOSE_C(L"PromiseRoot(resumeCallback)");
                this->_MyBase::resumeCallback = resumeCallback;
            }

            template <typename Caller, typename... Args>
            PromiseRoot(Caller& caller, InstanceName instanceName, std::function<void(std::weak_ptr<CoTaskBase>)> resumeCallback, Args&... args)
                : _MyBase(caller, instanceName)
            {
                this->SetFullClassNameSilent(instanceName.name);
                LOG_FUNCTION_ENTER_VERBOSE_C(L"Promise(Caller, resumeCallback)");
                this->_MyBase::resumeCallback = resumeCallback;
            }
        };

        /*------------------*/
        /*   AWAITER BASE   */
        /*------------------*/
        template <typename PromiseImplT>
        class AwaiterBase { // Base Awaiter class for CoTask. You need overload await_suspend for all type of promises that can be used. 
            CLASS_FULLNAME_LOGGING_INLINE_IMPLEMENTATION(AwaiterBase);
        public:
            AwaiterBase(std::coroutine_handle<PromiseImplT> selfCoroutine, InstanceName instanceName)
                : selfCoroutine{ selfCoroutine }
            {
                this->SetFullClassNameSilent(instanceName.name);
                LOG_FUNCTION_ENTER_VERBOSE_C(L"AwaiterBase(selfCoroutine)");
            }
            ~AwaiterBase() {
                LOG_FUNCTION_ENTER_VERBOSE_C(L"~AwaiterBase()");
            }
            bool await_ready() noexcept {
                return suspend::always; // suspend selfCoroutine
            }
            std::coroutine_handle<> await_suspend(std::coroutine_handle<PromiseDefault> callerCoroutine) noexcept {
                LOG_FUNCTION_SCOPE_VERBOSE_C("await_suspend(coroutine_handle<PromiseDefault>)");
                return await_suspend_internal<PromiseDefault>(callerCoroutine);
            }
            template<typename ReturnT>
            std::coroutine_handle<> await_suspend(std::coroutine_handle<PromiseWithResult<ReturnT>> callerCoroutine) noexcept {
                LOG_FUNCTION_SCOPE_VERBOSE_C("await_suspend(coroutine_handle<PromiseWithResult>)");
                return await_suspend_internal<PromiseWithResult<ReturnT>>(callerCoroutine);
            }
            std::coroutine_handle<> await_suspend(std::coroutine_handle<PromiseRoot> callerCoroutine) noexcept {
                LOG_FUNCTION_SCOPE_VERBOSE_C("await_suspend(coroutine_handle<PromiseRoot>)");
                return await_suspend_internal<PromiseRoot>(callerCoroutine);
            }
            auto await_resume() {
                LOG_FUNCTION_ENTER_VERBOSE_C("await_resume()");
                return selfCoroutine.promise().detach_result();
            }

        private:
            // NOTE: callerCoroutine associated with outter co-function context where called operator co_await CoTask{}
            template <typename PromiseT>
            std::coroutine_handle<> await_suspend_internal(std::coroutine_handle<PromiseT> callerCoroutine) noexcept {
                LOG_FUNCTION_SCOPE_VERBOSE_C("await_suspend_internal(callerCoroutine)");
                if (selfCoroutine.promise().is_canceled()) {
                    return callerCoroutine; // continue caller coroutine
                }
                // 1. Remember callerCoroutine (it will resumed in Promise::FinalAwaiter when selfCoroutine finished)
                // 2. Resume selfCoroutine
                selfCoroutine.promise().remember_other_coroutine(callerCoroutine);
                return selfCoroutine;
                // 3. Control returned to callerCoroutine only when selfCoroutine finished;
                //    if selfCoroutine has its own suspend points control returned to base resumer (previous stack frame)
            }

        private:
            std::coroutine_handle<PromiseImplT> selfCoroutine;
        };


        /*------------------*/
        /*   CO-TASK BASE   */
        /*------------------*/
        // Base (universal) class that encapsulates logic to work with coroutine_handle
        class CoTaskBase {
            CLASS_FULLNAME_LOGGING_INLINE_IMPLEMENTATION(CoTaskBase);
        public:
            CoTaskBase() { // In most cases default Ctor must not be called
                LOG_FUNCTION_ENTER_VERBOSE("CoTaskBase()");
            }
            CoTaskBase(std::coroutine_handle<> selfCoroutineBase, std::weak_ptr<int> promiseToken, InstanceName instanceName)
                : selfCoroutineBase{ selfCoroutineBase }
                , promiseToken{ promiseToken }
            {
                this->SetFullClassNameSilent(instanceName.name);
                LOG_FUNCTION_ENTER_VERBOSE_C("CoTaskBase(selfCoroutineBase, promiseToken)");
            }
            ~CoTaskBase() {
                LOG_FUNCTION_ENTER_VERBOSE_C("~CoTaskBase()");
                // here promise_type already destroyed (in derived class Dtor)
            }

            CoTaskBase(const CoTaskBase&) = delete;
            CoTaskBase& operator=(const CoTaskBase&) = delete;

            CoTaskBase(CoTaskBase&&) = delete;
            CoTaskBase& operator=(CoTaskBase&&) = delete;

            bool operator==(std::coroutine_handle<> otherCoroutine) {
                return selfCoroutineBase == otherCoroutine;
            }

            friend void SafeResume(std::weak_ptr<CoTaskBase>);

            void cancel() {
                LOG_FUNCTION_ENTER_VERBOSE_C("cancel()");
                canceled = true;
                cancelPromise();
            }

        protected:
            virtual void cancelPromise() = 0;

            // You also can use std::noop_coroutine() instead unique_ptr
            std::unique_ptr<std::coroutine_handle<>> get_coro_handle() {
                LOG_FUNCTION_SCOPE_VERBOSE_C("get_coro_handle()");
                if (canceled) {
                    LOG_WARNING_D("task canceled");
                    return nullptr;
                }
                if (promiseToken.expired()) {
                    LOG_WARNING_D("promiseToken expired!");
                    return nullptr;
                }
                if (!selfCoroutineBase) {
                    LOG_WARNING_D("selfCoroutineBase is empty!");
                    return nullptr;
                }
                return std::make_unique<std::coroutine_handle<>>(selfCoroutineBase);
            }

        protected:
            std::coroutine_handle<> selfCoroutineBase;
            std::weak_ptr<int> promiseToken;
            std::atomic<bool> canceled = false;
        };


        // NOTE: we don't save shared_ptr<CoTaskBase>
        inline void SafeResume(std::weak_ptr<CoTaskBase> taskWeak) {
            LOG_FUNCTION_SCOPE_VERBOSE("SafeResume(taskWeak)");
            std::unique_ptr<std::coroutine_handle<>> coroHandle;

            if (auto task = taskWeak.lock()) {
                coroHandle = task->get_coro_handle();
            }

            if (coroHandle) { // if task->get_coro_handle() return std::noop_coroutine() this condition not need
                coroHandle->resume();
            }
        }


        /*-----------------------------*/
        /*   CO-TASK IMPLEMENTATIONS   */
        /*-----------------------------*/
        template <typename PromiseImplT>
        class CoTask : public CoTaskBase {
            CLASS_FULLNAME_LOGGING_INLINE_IMPLEMENTATION(CoTask);
        public:
            using Ret_t = typename PromiseImplT::ObjectRet_t;
            using CoHandle_t = std::coroutine_handle<PromiseImplT>;
            using promise_type = PromiseImplT;

            CoTask() { // In most cases default Ctor must not be called
                LOG_FUNCTION_ENTER_VERBOSE("CoTask()");
            }
            CoTask(CoHandle_t selfCoroutine, std::weak_ptr<int> promiseToken, InstanceName instanceName)
                : CoTaskBase(selfCoroutine, promiseToken, instanceName)
                , selfCoroutine{ selfCoroutine }
            {
                this->SetFullClassNameSilent(instanceName.name);
                LOG_FUNCTION_ENTER_VERBOSE_C("CoTask(selfCoroutine, promiseToken)");
            }
            ~CoTask() {
                LOG_FUNCTION_ENTER_VERBOSE_C("~CoTask()");
                if (selfCoroutine) {
                    // promise_type will be destroyed; no need destroy selfCoroutineBase because it is the same.
                    selfCoroutine.destroy();
                }
            }

            CoTask(const CoTask&) = delete;
            CoTask& operator=(const CoTask&) = delete;

            CoTask(CoTask&&) = delete;
            CoTask& operator=(CoTask&&) = delete;

            // TODO: quilify operator with &&
            auto operator co_await() {
                return AwaiterBase<promise_type> { selfCoroutine, this->GetFullClassNameW() };
            }

        private:
            void cancelPromise() override {
                LOG_FUNCTION_ENTER_VERBOSE_C("cancelPromise()");
                selfCoroutine.promise().cancel();
            }

        private:
            CoHandle_t selfCoroutine;
        };

        template<typename PromiseImplT>
        auto operator co_await(const std::shared_ptr<CoTask<PromiseImplT>>& task) {
            return (*task).operator co_await();
        }

        /*------------*/
        /*   HELPERS  */
        /*------------*/
        template <typename T>
        struct PromiseExtractor {
        };

        template <template<typename> typename CoTaskT, typename PromiseImplT>
        struct PromiseExtractor<CoTaskT<PromiseImplT>> {
            using promiseImpl_t = PromiseImplT;
        };

        template <template<typename> typename CoTaskT, typename PromiseImplT>
        struct PromiseExtractor<std::shared_ptr<CoTaskT<PromiseImplT>>> {
            using promiseImpl_t = PromiseImplT;
        };

        // TODO: use another specific signal type that associated with coroutines (because H::Signal may be anything)
        inline void ResumeCoroutineViaSignal(
			std::weak_ptr<HELPERS_NS::Event::Signal<void()>> resumeSignalWeak
		) {
            LOG_FUNCTION_ENTER("ResumeCoroutineViaSignal()");
            auto resumeSignal = resumeSignalWeak.lock();
            if (!resumeSignal) {
                LOG_ERROR_D("resumeSignal expired");
                return;
            }
            resumeSignal->Invoke();
        }

        template<typename ResultT>
        inline void ResumeCoroutineViaSignal(
			std::weak_ptr<HELPERS_NS::Event::Signal<void(ResultT)>> resumeSignalWeak,
			ResultT result
		) {
            LOG_FUNCTION_ENTER("ResumeCoroutineViaSignal()");
            auto resumeSignal = resumeSignalWeak.lock();
            if (!resumeSignal) {
                LOG_ERROR_D("resumeSignal expired");
                return;
            }
            resumeSignal->Invoke(std::move(result));
        }
    } // namespace Async
} // namespace HELPERS_NS


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