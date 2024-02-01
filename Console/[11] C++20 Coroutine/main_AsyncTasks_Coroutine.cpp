#include <Helpers/ConcurrentQueue.h>
#include <Helpers/Async/AsyncTasks.h>
#include <Helpers/Logger.h>
//#include "AsyncTasksCo.h"
#include <iostream>

H::ConcurrentQueue<H::TaskItemWithDescription> workQueue;
H::Async::AsyncTasks asyncTasks{ L"asyncTasks" };

//
//H::Async::AsyncTasks::Task::Ret_t EmbeddedTask() {
//    LOG_FUNCTION_SCOPE("EmbeddedTask()");
//    static thread_local std::size_t functionEnterThreadId = HELPERS_NS::GetThreadId();
//    using currentTask_t = H::Async::AsyncTasks::Task;
//
//    LOG_DEBUG_D("EmbeddedTask computes 1  ~500ms ...");
//    Sleep(500);
//
//    co_await H::Async::ResumeAfter(1000ms, [](std::weak_ptr<currentTask_t> currentTaskWeak) {
//        LOG_FUNCTION_SCOPE("EmbeddedTask(...)::<timeout_lambda>");
//        auto currentTask = currentTaskWeak.lock();
//        if (!currentTask) {
//            LOG_ERROR_D("currentTask expired");
//            return;
//        }
//        currentTask->resume();
//        int xxx = 9;
//        });
//
//    LOG_DEBUG_D("EmbeddedTask computes 2  ~500ms ...");
//    Sleep(500);
//    co_return;
//}

//H::Async::CoTask<H::Async::PromiseDefault>::Ret_t SimpleTask(std::wstring coroFrameName) {
//    LOG_FUNCTION_SCOPE("SimpleTask(coroFrameName)");
//    LOG_DEBUG_D("Some heavy computes ~1s ...");
//    Sleep(1000);
//    co_return;
//}

H::Async::CoTask<H::Async::PromiseSignal>::Ret_t TaskWithResumeSignal(std::wstring coroFrameName, std::shared_ptr<H::Signal> resumeSignal) {
    LOG_FUNCTION_SCOPE(L"TaskWithSignal({}, resumeSignal)", coroFrameName);
    
    co_await H::Async::AsyncOperationWithResumeSignal([](std::weak_ptr<H::Signal> resumeSignalWeak) {
        LOG_FUNCTION_SCOPE(L"AsyncOperationWithSignal__lambda()");

        H::Timer::Once(5000ms, [resumeSignalWeak] { 
            auto resumeSignal = resumeSignalWeak.lock();
            if (!resumeSignal) {
                LOG_ERROR_D("resumeSignal expired");
                return;
            }
            (*resumeSignal)();
            int xx = 9;
            });
        });

    LOG_DEBUG_D("Got response");
    co_return;
}

H::Async::AsyncTasks::Task::Ret_t DeferredTask_AAA(std::wstring coroFrameName) {
    LOG_FUNCTION_SCOPE(L"DeferredTask_AAA({})", coroFrameName);
    static thread_local std::size_t functionEnterThreadId = HELPERS_NS::GetThreadId();
    using currentTask_t = H::Async::AsyncTasks::Task;

    LOG_DEBUG_D("Some heavy computes ~1s ...");
    Sleep(1000);
    
    LOG_DEBUG_D("await TaskWithSignal ...");
    //co_await H::Async::ResumeAfter<currentTask_t::promise_type>(2000ms);
    //co_await *SimpleTask(L"SimpleTask");
    co_await *TaskWithResumeSignal(L"TaskSignal", std::make_shared<H::Signal>());
    LOG_DEBUG_D("TaskWithSignal finished");

    if (HELPERS_NS::GetThreadId() != functionEnterThreadId) {
        LOG_ERROR_D("It seems you resume this task from thread that differs from initial. Force return.");
        co_return;
    }
    co_return;
}

H::Async::AsyncTasks::Task::Ret_t DeferredTask_BBB(std::wstring coroFrameName) {
    LOG_FUNCTION_SCOPE("DeferredTask_BBB(coroFrameName)");
    LOG_DEBUG_D("Some heavy computes ~1s ...");
    Sleep(1000);
    co_return;
}

//H::Async::CoTask<H::Async::PromiseRoot>::Ret_t CustomTask(std::wstring coroFrameName) { 
//    co_return;
//}


int main() {
    _set_error_mode(_OUT_TO_MSGBOX);
	lg::DefaultLoggers::Init("D:\\main_AsyncTasks_Coroutine.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);
    LOG_FUNCTION_SCOPE("main()");

    asyncTasks.SetResumeCallback([](std::weak_ptr<H::Async::CoTaskBase> rootTaskWeak) {
        workQueue.Push({ "signal ResumeNextCoTask()", [rootTaskWeak] {
            auto rootTask = rootTaskWeak.lock();
            if (!rootTask) {
                LOG_ERROR_D("rootTask expired");
                return;
            }
            rootTask->resume();
            int xx = 9;
            } });
        });

    asyncTasks.Add(TaskWithResumeSignal(L"TaskSignal", std::make_shared<H::Signal>()), 100ms);
    //asyncTasks.Add(DeferredTask_AAA(L"AAA"), 100ms);
    //asyncTasks.Add(DeferredTask_BBB(L"BBB"), 0ms);

    workQueue.Push({ "signal ShowEvent()", [] {
        LOG_DEBUG_D("ShowEvent_Action_1");
        Sleep(250);

        asyncTasks.StartExecuting();
        //asyncTasks.StartExecuting(); // will be ignored if coroutine not finished

        Sleep(250);
        LOG_DEBUG_D("ShowEvent_Action_2");
        Sleep(500);

        workQueue.Push({ "signal CustomSlot()", [] {
            LOG_DEBUG_D("CustomSlot_Action_1");
            Sleep(500);
            LOG_DEBUG_D("CustomSlot_Action_2");
            Sleep(500);
            LOG_DEBUG_D("CustomSlot_Action_last");
            Sleep(500);
            return;
            } });

        LOG_DEBUG_D("ShowEvent_Action_last");
        Sleep(500);
        return;
        } });
    

    while (workQueue.IsWorking()) {
        auto item = workQueue.Pop();
        if (item.task) {
            LOG_FUNCTION_SCOPE("task <{}>()", item.descrtiption);
            item.task();
        }
    }

	return 0;
}