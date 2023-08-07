// compile with: /EHsc
#include <ppltasks.h>
#include <Windows.h>
#include <iostream>
#include <memory>
#include <thread>

using namespace concurrency;

inline void Print(const std::wstring& msg) {
    OutputDebugStringW((L"[dbg]  " + msg + L"\n").c_str());
    std::wcout << msg << std::endl;
}




void TestTaskChainCompletion() {
    // NOTE: next task may executet eather previous task thread or some else (it is undefined)

    auto taskA = task<void>([]() {
        Print(L"Task A: started");
        Sleep(1'000);
        Print(L"Task A: completed");
        });

    //taskA = taskA.then([]() {
    auto taskB = taskA.then([]() {
        Print(L"Task B: started");
        Sleep(1'000);
        Print(L"Task B: completed");
        });

    //taskA = taskA.then([]() {
    auto taskC = taskB.then([]() {
        Print(L"Task C: started");
        Sleep(1'000);
        Print(L"Task C: completed");
        });

    auto taskD = taskC.then([]() {
        Print(L"Task D: started");
        Sleep(1'000);
        Print(L"Task D: completed");
        });

    auto taskE = taskD.then([]() {
        Print(L"Task E: started");
        Sleep(1'000);
        Print(L"Task E: completed");
        });

    auto taskF = taskE.then([]() {
        Print(L"Task F: started");
        Sleep(1'000);
        Print(L"Task F: completed");
        });

    auto taskG = taskF.then([]() {
        Print(L"Task G: started");
        Sleep(1'000);
        Print(L"Task G: completed");
        });


    taskG.wait();
    Sleep(100);
    //taskB.wait();
    //Sleep(1);
    //taskC.wait();
    //Sleep(1);
    Sleep(10);
}


void TestTaskChainWith—ompletionEvent() {
    task_completion_event<void> taskA_completionEvent;
    cancellation_token_source taskA_cancelationToken;
    {
        auto taskA = task<void>(taskA_completionEvent, taskA_cancelationToken.get_token());
        taskA = taskA.then([]() {
            Print(L"Task A: started");
            Sleep(300);
            Print(L"Task A: completed");
            });

        auto taskB = taskA.then([]() {
            Print(L"Task B: started");
            Sleep(300);
            Print(L"Task B: completed");
            });

        int xx = 9;
    }
    

    Sleep(2'000);
    Sleep(2'000);
    taskA_completionEvent.set();
    Sleep(2'000);
    Sleep(2'000);
    Sleep(2'000);
}



//void TestTaskChainWhenAll() {
//    auto taskA = task<void>([]() {
//        Print(L"Task A: started");
//        Sleep(500);
//        Print(L"Task A: completed");
//        });
//
//    auto taskB = taskA.then([]() {
//        Print(L"Task B: started");
//        Sleep(500);
//        Print(L"Task B: completed");
//        });
//
//    auto taskC = taskB.then([]() {
//        Print(L"Task C: started");
//        Sleep(500);
//        Print(L"Task C: completed");
//        });
//
//    auto taskD = taskC.then([]() {
//        Print(L"Task D: started");
//        Sleep(1'000);
//        Print(L"Task D: completed");
//        });
//
//
//    std::thread([cts] {
//        Sleep(500);
//        cts.cancel();
//        }).detach();
//
//        auto statusA = taskA.wait();
//        Sleep(10);
//        auto statusB = taskB.wait();
//        Sleep(10);
//        auto statusC = taskC.wait();
//        Sleep(10);
//        auto statusD = taskD.wait();
//        Sleep(10);
//
//}




void TestTaskChainCancelation() {
    cancellation_token_source cts;
    auto cancelationToken = cts.get_token();

    auto taskA = task<void>([cancelationToken]() {
        Print(L"Task A: started");
        Print(L"Task A: op 1 ...");
        Sleep(1'000);
        if (cancelationToken.is_canceled()) {
            cancel_current_task(); // next tasks canceled after even without this call 
        }   
        Print(L"Task A: op 2 ...");
        Sleep(1'000);
        Print(L"Task A: completed");
        }, cancelationToken);

    //taskA = taskA.then([]() {
    auto taskB = taskA.then([]() {
        Print(L"Task B: started");
        Sleep(1'000);
        Print(L"Task B: completed");
        });

    //taskA = taskA.then([]() {
     auto taskC = taskB.then([]() {
        Print(L"Task C: started");
        Sleep(1'000);
        Print(L"Task C: completed");
        });

     auto taskD = taskC.then([]() {
         Print(L"Task D: started");
         Sleep(1'000);
         Print(L"Task D: completed");
         });


     std::thread([cts] {
         Sleep(500);
         cts.cancel();
         }).detach();

    auto statusA = taskA.wait();
    Sleep(10);
    auto statusB = taskB.wait();
    Sleep(10);
    auto statusC = taskC.wait();
    Sleep(10);
    auto statusD = taskD.wait();
    Sleep(10);
}




void TestAsyncUnwrapping() {
        {
        //auto mainTask = create_task([]() {
        task<void> mainTask = task<void>([]() {
            Print(L"Task Main");
            Sleep(4'000);
            Sleep(10);
            });
        Sleep(10);
    }

    auto taskA = create_task([]() {
        Print(L"Task A");
        Sleep(500);
        Sleep(500);

        //// Create an inner task that runs before any continuation
        //// of the outer task.
        //return create_task([]() {
        //    Print(L"Task inner: operation 1 ...");
        //    Sleep(500);
        //    Print(L"Task inner: operation 2 ...");
        //    Sleep(500);
        //    Print(L"Task inner: operation 3 ...");
        //    Sleep(500);
        //    });

        return 123;
        });

    Sleep(6'000);
    Sleep(10);

    // Run and wait for a continuation of the outer task.
    auto taskB = taskA.then([](int result) {
        Sleep(500);
        Print(L"Task B");
        Sleep(500);
        return 3.14f;
        });

    taskA.wait();
    Sleep(10);
}







struct VncComponent {
    int data;
};
std::unique_ptr<VncComponent> vncComponent;
std::mutex mxVncComponent;
int responseDelay = 3'000; // ms



/* -------------------------------------------------------------- */
/*        Test Vnc Creation with ppl task completion event        */
/* -------------------------------------------------------------- */
task_completion_event<void> tceVncCreated;
cancellation_token_source ctsVncCreation;
task<void> vncAfterInitializedTask;

void UseVncComponent() {
    auto isSet = tceVncCreated._IsTriggered();

    vncAfterInitializedTask = vncAfterInitializedTask.then([] {
        std::lock_guard lk{ mxVncComponent };
        vncComponent->data = 1234;
        Sleep(400);
        Sleep(400);
        Sleep(400);
        Sleep(400);
        Sleep(400);
        });
}

void TestVncComponentCreation() {
    vncAfterInitializedTask = task<void>(tceVncCreated, ctsVncCreation.get_token());
    // request to get connection data ...

    // signal from remote user to use vnc component
    std::thread([] {
        Sleep(responseDelay+2'000); 
        UseVncComponent();
        }).detach();

    // simulate disconnect:
    std::thread([] {
        Sleep(responseDelay+1'000);
        ctsVncCreation.cancel();
        auto status = vncAfterInitializedTask.wait();
        std::lock_guard lk{ mxVncComponent };
        vncComponent = nullptr;
        }).detach();


    Sleep(responseDelay); // signal from remote user to create vnc component

    try {
        {
            // creation and initialization routine
            std::lock_guard lk{ mxVncComponent };
            vncComponent = std::make_unique<VncComponent>();
            Sleep(200);
            Sleep(200);
            // now here we completed and can use vnc component
            tceVncCreated.set();
            Sleep(200);
            Sleep(200);
        }
    }
    catch (...) {

    }
    
    Sleep(10);
}






class TaskChain {
public:
    TaskChain() = default;
    ~TaskChain() {
        CancelAndWait();
    }

    void Append(std::function<void()> taskLambda) {
        task = task.then(taskLambda); // when completion event set all previous and next tasks execute immediatly
    }

    void StartExecuting() {        
        if (executing.exchange(true))
            return; // return if previous value == true

        taskCompletedEvent.set(); // at this point tasks start executing in some thread via task scheduler
    }

    void CancelAndWait() {
        if (!executing.exchange(false))
            return; // return if previous value == false

        cancelationToken.cancel();
        bool isDone = task.is_done();
        auto status = task.wait();

        cancelationToken = {};
        taskCompletedEvent = {};
        task = Concurrency::task<void>(taskCompletedEvent, cancelationToken.get_token());
    }

private:
    std::atomic<bool> executing = false;

    Concurrency::cancellation_token_source cancelationToken;
    Concurrency::task_completion_event<void> taskCompletedEvent;
    Concurrency::task<void> task = Concurrency::task<void>(taskCompletedEvent, cancelationToken.get_token()); // must be created after completionEvent and token
};


/* -------------------------------------------------------------- */
/*             Test Vnc Creation with task chain class            */
/* -------------------------------------------------------------- */
TaskChain taskChainVncCreated;

void UseVncComponentWithTaskChainClass() {
    taskChainVncCreated.Append([] {
        std::lock_guard lk{ mxVncComponent };
        vncComponent->data = 1234;
        Sleep(400);
        Sleep(400);
        Sleep(400);
        Sleep(400);
        Sleep(400);
        });
}

void TestVncComponentCreationWithTaskChainClass() {
    // request to get connection data ...

    // signal from remote user to use vnc component
    std::thread([] {
        //Sleep(responseDelay - 500); // before vnc created
        Sleep(responseDelay + 1'000); // after vnc created but before destroyed signal
        //Sleep(responseDelay + 3'000); // after vnc created and after destroyed signal
        UseVncComponentWithTaskChainClass();
        }).detach();

    // simulate disconnect:
    std::thread([] {
        Sleep(responseDelay + 2'000);
        taskChainVncCreated.CancelAndWait();
        std::lock_guard lk{ mxVncComponent };
        vncComponent = nullptr;
        }).detach();


    Sleep(responseDelay); // signal from remote user to create vnc component

    try {
        {
            // creation and initialization routine
            std::lock_guard lk{ mxVncComponent };
            vncComponent = std::make_unique<VncComponent>();
            Sleep(200);
            Sleep(200);
            // now here we completed and can use vnc component
            taskChainVncCreated.StartExecuting();
            Sleep(200);
            Sleep(200);
        }
    }
    catch (...) {

    }

    Sleep(10);
}


void main() {
    //TestTaskChainCompletion();
    //TestTaskChainWith—ompletionEvent();
    //TestTaskChainCancelation();
    //TestAsyncUnwrapping();
    //TestVncComponentCreation();
    TestVncComponentCreationWithTaskChainClass();

    while (true) {
        Sleep(10);
    };
    return;
}