#pragma once
#include <Helpers/common.h>
#include <Helpers/Collection/iterable_queue.h>
#include <Helpers/Meta/FunctionTraits.h>
#include <Helpers/Thread.h>
#include <Helpers/Logger.h>
#include "Awaitables.h"
#include "CoTask.h"

// Don't forget return original definitions for these macros at the end of file
#define LOG_FUNCTION_ENTER_VERBOSE(fmt, ...)
#define LOG_FUNCTION_SCOPE_VERBOSE(fmt, ...)
#define LOG_FUNCTION_ENTER_VERBOSE_C(fmt, ...)
#define LOG_FUNCTION_SCOPE_VERBOSE_C(fmt, ...)

using namespace std::chrono_literals;

namespace HELPERS_NS {
	namespace Async {
		// TODO: move implementation to .cpp
		class AsyncTasks {
			CLASS_FULLNAME_LOGGING_INLINE_IMPLEMENTATION(AsyncTasks);

#       ifdef TEST_AsyncTasks_FirendWrapper
			friend class TEST_AsyncTasks_FirendWrapper; // NOTE: class-wrapper must be in same namespace
#       endif

		public:
			using Task = CoTask<PromiseDefault>;

			AsyncTasks(std::wstring instanceName = L"Unknown") {
				this->SetFullClassName(instanceName);
				LOG_FUNCTION_ENTER_C("AsyncTasks()");
			}
			~AsyncTasks() {
				LOG_FUNCTION_SCOPE_C("~AsyncTasks()");
				Cancel();
			}

		private:
			using RootTask = CoTask<PromiseRoot>;

			// TODO: Rewrite wtih concepts
			template <bool Requirements, typename PromiseImplT>
			using AddedResult = std::enable_if_t<Requirements, std::weak_ptr<CoTask<PromiseImplT>>>;

			template <typename PromiseImplT>
			static constexpr bool IsValidPromise = std::is_same_v<PromiseImplT, PromiseDefault>;

			class TaskWrapper {
			public:
				TaskWrapper(std::shared_ptr<Task> task, std::chrono::milliseconds startAfter)
					: task{ task }
					, startAfter{ startAfter } {
				}
				std::shared_ptr<Task>& GetTask() {
					return task;
				}
				std::chrono::milliseconds GetStartAfterTime() {
					return startAfter;
				}
			private:
				std::shared_ptr<Task> task;
				std::chrono::milliseconds startAfter;
			};

		public:
			void SetResumeCallback(std::function<void(std::weak_ptr<CoTaskBase>)> resumeCallback) {
				LOG_FUNCTION_ENTER_C("SetResumeCallback(resumeCallback)");
				this->resumeCallback = resumeCallback;
			}

			template <
				typename PromiseImplT,
				typename... FnArgs,
				typename... RealArgs
			>
			AddedResult<IsValidPromise<PromiseImplT>, PromiseImplT> AddTaskFn(
				std::chrono::milliseconds startAfter,
				std::shared_ptr<CoTask<PromiseImplT>>(*taskFn)(FnArgs...),
				RealArgs&&... args
			) {
				LOG_FUNCTION_ENTER_C("AddTaskFn(...)");
				std::unique_lock lk{ mx };
				tasks.push(TaskWrapper{ taskFn(std::forward<RealArgs&&>(args)...), startAfter });
				return tasks.front().GetTask();
			}

			template <
				typename PromiseImplT,
				typename C,
				typename... FnArgs,
				typename... RealArgs
			>
			AddedResult<IsValidPromise<PromiseImplT>, PromiseImplT> AddTaskFn(
				std::chrono::milliseconds startAfter,
				C* classPtr, std::shared_ptr<CoTask<PromiseImplT>>(C::* taskFn)(FnArgs...),
				RealArgs&&... args
			) {
				LOG_FUNCTION_ENTER_C("AddTaskFn(classPtr, ...)");
				std::unique_lock lk{ mx };
				tasks.push(TaskWrapper{ std::invoke(taskFn, classPtr, std::forward<RealArgs&&>(args)...), startAfter });
				return tasks.front().GetTask();
			}

			template <
				typename Fn,
				typename... Args,
				typename PromiseImplT = PromiseExtractor<typename meta::FunctionTraits<Fn>::Ret>::promiseImpl_t
			>
			AddedResult<IsValidPromise<PromiseImplT>, PromiseImplT> AddTaskLambda(
				std::chrono::milliseconds startAfter,
				Fn lambda,
				Args... args
			) {
				LOG_FUNCTION_ENTER_C("AddTaskLambda(...)");
				// based on https://devblogs.microsoft.com/oldnewthing/20211103-00/?p=105870#comment-138536
				struct LambdaBindCoro {
					static typename meta::FunctionTraits<Fn>::Ret Bind(LambdaBindCoroKey, Fn lambda, Args... args) {
						LOG_FUNCTION_SCOPE_VERBOSE("LambdaBindCoro::Bind(...)");
						co_return co_await *lambda(std::move(args)...);
					}
				};

				std::unique_lock lk{ mx };
				tasks.push(TaskWrapper{ LambdaBindCoro::Bind(LambdaBindCoroKey{}, std::move(lambda), std::move(args)...), startAfter });
				return tasks.front().GetTask();
			}

			bool StartExecuting() {
				LOG_FUNCTION_SCOPE_C("StartExecuting()");
				if (LOG_ASSERT(!executingStarted.exchange(true), "Executing of root coroutine already started!")) {
					return false;
				}
				rootTask = StartExecutingCoroutine(L"rootTask", resumeCallback);
				HELPERS_NS::Async::SafeResume(rootTask);
				return true;
			}

			bool IsExecutingStarted() {
				return executingStarted;
			}

			void Cancel() {
				LOG_FUNCTION_SCOPE_C("Cancel()");
				if (rootTask)
					rootTask->cancel(); // no need mutex synchronization

				std::unique_lock lk{ mx };
				for (auto& taskWrapper : tasks) {
					if (auto& task = taskWrapper.GetTask()) {
						task->cancel();
					}
				}
			}

		private:
			// TODO: Add multiple calls guard
			// TODO: Implement special CoTask for StartExecuting() to avoid resume this task from another thread
			RootTask::Ret_t StartExecutingCoroutine(
				std::wstring coroFrameName,
				std::function<void(std::weak_ptr<CoTaskBase>)> resumeCallback
			) {
				LOG_FUNCTION_SCOPE_C("StartExecutingCoroutine(...)");
				static thread_local std::size_t functionEnterThreadId = HELPERS_NS::GetThreadId();
				try {
					while (!tasks.empty()) {
						auto taskWrapper = GetNextTask();
						auto startAfter = taskWrapper.GetStartAfterTime();
						auto& task = taskWrapper.GetTask();

						// NOTE: if you want resume such tasks (with 0ms timeout) asynchronously (in next workQueue Pop event) - comment this condition
						if (startAfter.count() > 0) {
							co_await ResumeAfter<RootTask::promise_type>(startAfter); // [suspend point] here control is returned to caller
						}

						if (HELPERS_NS::GetThreadId() != functionEnterThreadId) {
							LOG_ERROR_D("You resume this task from thread that differs from initial!");
						}

						LOG_DEBUG_D("await co-task ...");
						co_await *task;
						LOG_DEBUG_D("co-task finished");
					}
				}
				catch (...) {
					LOG_ERROR_D("Catch unhandled exception");
					LOG_WARNING_D("clear tasks queue and rethrow");
					tasks = {}; // clear queue
					executingStarted = false;
					throw;
				}
				executingStarted = false; // TODO: move to promise final_suspend logic
				co_return;
			}

			TaskWrapper GetNextTask() {
				LOG_FUNCTION_ENTER_C("GetNextTask()");
				std::unique_lock lk{ mx };
				if (tasks.empty()) {
					return TaskWrapper{ nullptr, 0ms };
				}
				auto taskWrapper = std::move(tasks.front());
				tasks.pop();
				return taskWrapper;
			}

		private:
			std::mutex mx;
			RootTask::Ret_t rootTask;
			std::atomic<bool> executingStarted = false;
			HELPERS_NS::Collection::iterable_queue<TaskWrapper> tasks;
			std::function<void(std::weak_ptr<CoTaskBase>)> resumeCallback;
		};
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