#include <Helpers/ConcurrentQueue.h>
#include <Helpers/Async/AsyncTasks.h>
#include <Helpers/Logger.h>
//#include "AsyncTasksCo.h"
#include <iostream>

H::Async::AsyncTasks asyncTasks{ L"asyncTasks" };


H::Async::AsyncTasks::Task::Ret_t EmbeddedTask() {
    LOG_FUNCTION_SCOPE("EmbeddedTask()");
    LOG_DEBUG_D("Some heavy computes ~1s ...");
    Sleep(1000);
    co_return;
}


H::Async::AsyncTasks::Task::Ret_t DeferredTask_AAA(std::wstring coroFrameName) {
    LOG_FUNCTION_SCOPE("DeferredTask_AAA(coroFrameName)");
    //asyncTasks.Add(EmbeddedTask(), 700ms);
    LOG_DEBUG_D("Some heavy computes ~1s ...");
    Sleep(1000);
    co_return;
}

H::Async::AsyncTasks::Task::Ret_t DeferredTask_BBB(std::wstring coroFrameName) {
    LOG_FUNCTION_SCOPE("DeferredTask_BBB(coroFrameName)");
    LOG_DEBUG_D("Some heavy computes ~1s ...");
    Sleep(1000);
    co_return;
}


int main() {
	lg::DefaultLoggers::Init("D:\\main_AsyncTasks_Coroutine.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);
    LOG_FUNCTION_SCOPE("main()");

    H::ConcurrentQueue<H::TaskItemWithDescription> workQueue;
    
    asyncTasks.SetResumeCallback([&workQueue](std::weak_ptr<H::Async::AsyncTasks::RootTask> rootTaskWeak) {
        workQueue.Push({ "signal ResumeNextCoTask()", [rootTaskWeak] {
            auto rootTask = rootTaskWeak.lock();
            if (!rootTask) {
                LOG_ERROR_D("rootTask expired");
                return;
            }
            rootTask->resume();
            } });
        });

    asyncTasks.Add(DeferredTask_AAA(L"AAA"), 1000ms);
    asyncTasks.Add(DeferredTask_BBB(L"BBB"), 0ms);

    workQueue.Push({ "signal ShowEvent()", [&workQueue] {
        LOG_DEBUG_D("ShowEvent_Action_1");
        Sleep(500);

        asyncTasks.StartExecuting();

        LOG_DEBUG_D("ShowEvent_Action_2");
        Sleep(500);

        workQueue.Push({ "signal CustomSlot()", [&workQueue] {
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