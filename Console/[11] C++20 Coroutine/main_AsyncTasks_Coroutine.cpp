#include <Helpers/ConcurrentQueue.h>
#include <Helpers/Logger.h>
#include "AsyncTasksCo.h"
#include <iostream>


//void TestCoroutine() {
//	LOG_FUNCTION_SCOPE("TestCoroutine()");
//	
//    LOG_DEBUG_D("actions before tick()");
//    auto saveTask = tick();
//	LOG_DEBUG_D("actions after tick()");
//
//    LOG_DEBUG_D("wait 8s ...");
//    Sleep(8'000);
//}

int main() {
	lg::DefaultLoggers::Init("D:\\main_AsyncTasks_Coroutine.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);
    LOG_FUNCTION_SCOPE("main()");

	//TestCoroutine();


    H::ConcurrentQueue<H::TaskItemWithDescription> workQueue;
    workQueue.Push({ "signal ShowEvent()", [&workQueue] {
        LOG_DEBUG_D("ShowEvent_Action_1");
        Sleep(500);
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