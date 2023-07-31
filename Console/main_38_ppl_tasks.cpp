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
    auto taskA = task<void>([]() {
        Print(L"Task A: started");
        Sleep(1'000);
        Print(L"Task A: completed");
        });

    taskA = taskA.then([]() {
    //auto taskB = taskA.then([]() {
        Print(L"Task B: started");
        Sleep(1'000);
        Print(L"Task B: completed");
        });

    taskA = taskA.then([]() {
    //auto taskC = taskB.then([]() {
        Print(L"Task C: started");
        Sleep(1'000);
        Print(L"Task C: completed");
        });


    taskA.wait();
    Sleep(100);
    //taskB.wait();
    //Sleep(1);
    //taskC.wait();
    //Sleep(1);
    Sleep(10);
}





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

task_completion_event<void> tceVncCreated;
task<void> vncAfterInitializedTask;

cancellation_token_source ctsVncCreation;


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





void main() {
    //TestTaskChainCompletion();
    //TestTaskChainCancelation();
    //TestAsyncUnwrapping();
    TestVncComponentCreation();
    while (true) {
        Sleep(10);
    };
    return;
}