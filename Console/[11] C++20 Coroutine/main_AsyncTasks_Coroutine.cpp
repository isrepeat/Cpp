#define DISABLE_VERBOSE_LOGGING

#include <Helpers/ConcurrentQueue.h>
#include <Helpers/Async/AsyncTasks.h>
#include <Helpers/Logger.h>
//#include "AsyncTasksCo.h"
#include <iostream>

H::ConcurrentQueue<H::TaskItemWithDescription> workQueue;
//H::Async::AsyncTasks asyncTasks{ L"asyncTasks" };

//H::Async::AsyncTasks::Task::Ret_t DeferredTask_AAA(std::wstring coroFrameName) {
//    LOG_FUNCTION_SCOPE(L"DeferredTask_AAA({})", coroFrameName);
//    static thread_local std::size_t functionEnterThreadId = HELPERS_NS::GetThreadId();
//    using currentTask_t = H::Async::AsyncTasks::Task;
//
//    LOG_DEBUG_D("Some heavy computes ~1s ...");
//    Sleep(1000);
//    
//    LOG_DEBUG_D("await TaskWithSignal ...");
//    //co_await H::Async::ResumeAfter<currentTask_t::promise_type>(2000ms);
//    //co_await *SimpleTask(L"SimpleTask");
//    co_await *TaskWithResumeSignal(L"TaskSignal", std::make_shared<H::Signal>());
//    LOG_DEBUG_D("TaskWithSignal finished");
//
//    if (HELPERS_NS::GetThreadId() != functionEnterThreadId) {
//        LOG_ERROR_D("It seems you resume this task from thread that differs from initial. Force return.");
//        co_return;
//    }
//    co_return;
//}
//
//H::Async::CoTask<H::Async::PromiseDefault>::Ret_t TaskSimple(std::wstring coroFrameName) {
//    LOG_FUNCTION_SCOPE(L"TaskSimple(name = {})", coroFrameName);
//    co_return;
//}


void AllocateMemoryOnStack() {
    LOG_FUNCTION_ENTER("AllocateMemoryOnStack()");
    char array[10'000];
}

struct Functor {
    int xx = 9;
    H::Async::CoTask<H::Async::PromiseDefault>::Ret_t operator()(std::wstring) {
        co_return;
    }
};

//H::Async::CoTask<H::Async::PromiseDefault>::Ret_t TaskWithAsyncOperation(H::Async::InstanceName, int, float) {
//H::Async::CoTask<H::Async::PromiseDefault>::Ret_t TaskWithAsyncOperation(int, float) {
H::Async::CoTask<H::Async::PromiseDefault>::Ret_t TaskWithAsyncOperation() {
    //LOG_FUNCTION_SCOPE(L"TaskWithAsyncOperation(name = {})", coroFrameName);

    
    co_await H::Async::AsyncOperationWithResumeSignal([](std::weak_ptr<H::Signal> resumeSignalWeak) {
        LOG_FUNCTION_SCOPE(L"AsyncOperationWithSignal__lambda()");

        H::Timer::Once(2000ms, [resumeSignalWeak] { // (1)
            auto resumeSignal = resumeSignalWeak.lock();
            if (!resumeSignal) {
                LOG_ERROR_D("resumeSignal expired");
                return;
            }
            (*resumeSignal)(); // (2)
            NOOP;
            });
        });
    
    LOG_DEBUG_D("Got response"); // (3)
    co_return;
}


struct Temp {
    Temp() {
        NOOP;
    }
    ~Temp() {
        NOOP;
    }

    void Test() {
        for (int i = 0; i < 10'000; i++) {
            array[i] = i;
        }
    }
 
    int array[10'000];
};

class Prototype {
public:
    Prototype() {
        asyncTasks.SetResumeCallback([](std::weak_ptr<H::Async::CoTaskBase> taskWeak) {
            workQueue.Push({ "signal ResumeNextCoTask()", [taskWeak] {
                H::Async::SafeResume(taskWeak);
                NOOP;
                } });
            });


        LOG_DEBUG("============================================================================");
        LOG_DEBUG("============================================================================\n\n");

        {
            auto temp = std::make_shared<Temp>();

            asyncTasks.AddTaskLambda(0ms, [this, temp](std::wstring taskName = {}) -> H::Async::AsyncTasks::Task::Ret_t {
                LOG_FUNCTION_ENTER("Task_1__lambda()");
                NOOP;
                temp->Test();
                //asyncTasks.Cancel();
                co_return;
                });
        }
        NOOP;


        {
            auto temp = std::make_shared<Temp>();

            asyncTasks.AddTaskLambda(0ms, [temp](std::wstring taskName = {}) -> H::Async::AsyncTasks::Task::Ret_t {
                LOG_FUNCTION_ENTER("Task_1__lambda()");
                NOOP;
                temp->Test();

                //co_await H::Async::AsyncOperationWithResumeSignal([](std::weak_ptr<H::Signal> resumeSignalWeak) {
                //    LOG_FUNCTION_SCOPE(L"AsyncOperationWithSignal__lambda()");

                //    H::Timer::Once(2000ms, [resumeSignalWeak] { // (1)
                //        auto resumeSignal = resumeSignalWeak.lock();
                //        if (!resumeSignal) {
                //            LOG_ERROR_D("resumeSignal expired");
                //            return;
                //        }
                //        (*resumeSignal)(); // (2)
                //        NOOP;
                //        });
                //    });

                //NOOP;
                ////asyncTasks.Cancel();
                co_return;
                }, L"Task_1");
        }
        NOOP;



        
        //asyncTasks.AddTaskFn(0ms, &TaskWithAsyncOperation, L"", 1, 3.0f);
        //asyncTasks.AddTaskFn(0ms, &TaskWithAsyncOperation, 1, 3.0f);
        asyncTasks.AddTaskFn(0ms, &TaskWithAsyncOperation);
        
        asyncTasks.AddTaskFn(0ms, this, &Prototype::TaskMember);
        //asyncTasks.AddTaskFn(0ms, &TaskWithAsyncOperation, L"");
        
        //auto fnPtr = &TaskWithAsyncOperation;
        //H::Async::CoTask<H::Async::PromiseDefault>::Ret_t(*fnPtr)(std::wstring coroFrameName = {});
        //fnPtr();
        
        //asyncTasks.AddTaskFn(0ms, &TaskWithAsyncOperation, L""); // Fails

        //std::function<H::Async::AsyncTasks::Task::Ret_t(std::wstring)> xxx = &Prototype::TaskMember;

        //std::function<H::Async::AsyncTasks::Task::Ret_t(std::wstring)> xxx = std::bind(&Prototype::TaskMember, this, std::placeholders::);
        
        //H::FunctionTraits<decltype(&Prototype::TaskMember)>::

        //asyncTasks.AddTaskLambda(0ms, &TaskWithAsyncOperation); // Fails
        //asyncTasks.AddTaskLambda(0ms, [](std::wstring taskName = {})->H::Async::AsyncTasks::Task::Ret_t { // Ok
        //    co_return;
        //    });

        //std::function<H::Async::AsyncTasks::Task::Ret_t(std::wstring)> stdFunc = [](std::wstring taskName = {})->H::Async::AsyncTasks::Task::Ret_t {
        //    co_return;
        //};
        //asyncTasks.AddTaskLambda(0ms, stdFunc); // Fails

        //asyncTasks.AddTaskFn(0ms, this, &Prototype::TaskMember);
        //
        ////auto xx = asyncTasks.AddTaskLambda(0ms, [](std::wstring) -> H::Async::CoTask<H::Async::PromiseDefault>::Ret_t {
        //////auto xx = asyncTasks.AddTaskLambda(0ms, [](std::wstring, std::function<void(std::weak_ptr<H::Async::CoTaskBase>)>) -> H::Async::CoTask<H::Async::PromiseRoot>::Ret_t {
        ////        co_return;
        ////        });


        //asyncTasks.AddTaskLambda(0ms, [this](std::wstring taskName = {}) -> H::Async::AsyncTasks::Task::Ret_t {
        //    LOG_FUNCTION_SCOPE("Task_Checker__lambda()");
        //    asyncTasks.Cancel();
        //    co_return;
        //    }, L"Task_Checker");


        //asyncTasks.AddTaskLambda(0ms, [](std::wstring taskName = {}) -> H::Async::AsyncTasks::Task::Ret_t {
        //    LOG_FUNCTION_SCOPE("Task_2__lambda()");
        //    co_return;
        //    }, L"Task_2");


        workQueue.Push({ "signal ShowEvent()", [this] {
            LOG_DEBUG_D("ShowEvent_Action_1");
            Sleep(250);

            asyncTasks.StartExecuting();
            //asyncTasks.StartExecuting(); // will be ignored if coroutine not finished

            Sleep(250);
            LOG_DEBUG_D("ShowEvent_Action_2");
            Sleep(500);

            //workQueue.Push({ "signal CustomSlot()", [] {
            //    LOG_DEBUG_D("CustomSlot_Action_1");
            //    Sleep(500);
            //    LOG_DEBUG_D("CustomSlot_Action_2");
            //    Sleep(500);
            //    LOG_DEBUG_D("CustomSlot_Action_last");
            //    Sleep(500);
            //    return;
            //    } });

            LOG_DEBUG_D("ShowEvent_Action_last");
            Sleep(500);
            return;
            } });
    }

    //H::Async::CoTask<H::Async::PromiseDefault>::Ret_t TaskMember(H::Async::InstanceName) {
    H::Async::CoTask<H::Async::PromiseDefault>::Ret_t TaskMember() {
        LOG_FUNCTION_SCOPE(L"TaskWithAsyncOperation()");


        co_await H::Async::AsyncOperationWithResumeSignal([](std::weak_ptr<H::Signal> resumeSignalWeak) {
            LOG_FUNCTION_SCOPE(L"AsyncOperationWithSignal__lambda()");

            H::Timer::Once(2000ms, [resumeSignalWeak] { // (1)
                auto resumeSignal = resumeSignalWeak.lock();
                if (!resumeSignal) {
                    LOG_ERROR_D("resumeSignal expired");
                    return;
                }
                (*resumeSignal)(); // (2)
                NOOP;
                });
            });

        LOG_DEBUG_D("Got response"); // (3)
        co_return;
    }

private:
    H::Async::AsyncTasks asyncTasks{ L"asyncTasks" };
};



int main() {
    _set_error_mode(_OUT_TO_MSGBOX);
	lg::DefaultLoggers::Init("D:\\main_AsyncTasks_Coroutine.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);
    LOG_FUNCTION_SCOPE("main()");


    Prototype prototype;

    while (workQueue.IsWorking()) {
        auto item = workQueue.Pop();
        if (item.task) {
            LOG_FUNCTION_SCOPE("task <{}>()", item.descrtiption);
            item.task();
        }
    }
	return 0;
}