#include <Helpers/ConcurrentQueue.h>
#include <Helpers/Logger.h>
#include "AsyncTasksCo.h"
#include <iostream>

Async::AsyncTasks asyncTasks;


Async::AsyncTasks::Task_t EmbeddedTask() {
    LOG_FUNCTION_SCOPE("EmbeddedTask()");
    LOG_DEBUG_D("Some heavy computes ~1s ...");
    Sleep(1000);
    co_return;
}


Async::AsyncTasks::Task_t DefferedTask_AAA() {
    LOG_FUNCTION_SCOPE("DefferedTask_AAA()");
    //asyncTasks.Add(EmbeddedTask());
    LOG_DEBUG_D("Some heavy computes ~1s ...");
    Sleep(1000);
    co_return;
}

Async::AsyncTasks::Task_t DefferedTask_BBB() {
    LOG_FUNCTION_SCOPE("DefferedTask_BBB()");
    LOG_DEBUG_D("Some heavy computes ~1s ...");
    Sleep(1000);
    co_return;
}


int main() {
	lg::DefaultLoggers::Init("D:\\main_AsyncTasks_Coroutine.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);
    LOG_FUNCTION_SCOPE("main()");


    //Async::AsyncTasks::Task_t::element_type::taskDescription;
    //Async::AsyncTasks::WorkToken_t::element_type::taskDescription;

    std::coroutine_traits<Async::AsyncTasks::Task_t>::promise_type;
    std::coroutine_traits<Async::AsyncTasks::WorkToken_t, Async::AsyncTasks*>::promise_type;


    H::ConcurrentQueue<H::TaskItemWithDescription> workQueue;
    
    asyncTasks.SetResumeCallback([&workQueue](std::coroutine_handle<> coroHandle) {
        workQueue.Push({ "signal ResumeNextCoTask()", [coroHandle] {
            coroHandle.resume();
            } });
        });

    asyncTasks.Add(DefferedTask_AAA());
    asyncTasks.Add(DefferedTask_BBB());

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