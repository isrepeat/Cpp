#include <Helpers/ConcurrentQueue.h>
#include <Helpers/Logger.h>
#include "AsyncTasksCo.h"
#include <iostream>




std::unique_ptr<Async::Task> DefferedTask() {
    LOG_FUNCTION_SCOPE("DefferedTask()");

    LOG_DEBUG_D("Some heavy computes ~1s ...");
    Sleep(1000);

    co_return;
}


void foo(std::unique_ptr<Async::Task> task) {
}


int main() {
	lg::DefaultLoggers::Init("D:\\main_AsyncTasks_Coroutine.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);
    LOG_FUNCTION_SCOPE("main()");

    std::coroutine_traits<Async::Task>::promise_type;
    std::coroutine_traits<std::unique_ptr<Async::Task>>::promise_type;

    H::ConcurrentQueue<H::TaskItemWithDescription> workQueue;
    std::queue<std::unique_ptr<Async::Task>> defferedTasks;
    
    {
        auto task = DefferedTask();
        defferedTasks.push(std::move(task));

        workQueue.Push({ "signal ShowEvent()", [&workQueue, &defferedTasks] {
            LOG_DEBUG_D("ShowEvent_Action_1");
            Sleep(500);

            if (!defferedTasks.empty()) {
                auto task = std::move(defferedTasks.front());
                defferedTasks.pop();
                task->resume();
            }

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
    }

    while (workQueue.IsWorking()) {
        auto item = workQueue.Pop();
        if (item.task) {
            LOG_FUNCTION_SCOPE("task <{}>()", item.descrtiption);
            item.task();
        }
    }

	return 0;
}