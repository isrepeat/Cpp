#define DISABLE_VERBOSE_LOGGING
#define DISABLE_ASSERT_LOGGING // NOTE: Redefine LOG_ASSERT macro to avoid potential side effects with computings something in other threads.
                               //       For example if default assert triggered Timers still will not stop (executes in other thread) and test may fails.
#define TEST_AsyncTasks_FirendWrapper AsyncTasks_Test

#include <Helpers/Async/AsyncTasks.h>
#include <Helpers/ConcurrentQueue.h>
#include <Helpers/Signal.h>
#include <Helpers/Logger.h>

#include <gtest/gtest.h> // GoogleTest: https://google.github.io/googletest/primer.html
#include <iostream>


namespace HELPERS_NS {
    namespace Async {
        class AsyncTasks_Test : public H::Async::AsyncTasks {
        public:
            std::function<void(std::weak_ptr<CoTaskBase>)> GetResumeCallback() {
                return resumeCallback;
            }

            int RemainingTasksCount() {
                std::unique_lock lk{ mx };
                return tasks.size();
            }
        };
    }
}


#define FUNCTION_SCOPE_SET_BOOL_VALUE(exist_bool_variable, bool_value)   \
    exist_bool_variable = bool_value;                                    \
    H::MakeScope([&exist_bool_variable] {                                \
        exist_bool_variable = !exist_bool_variable;                      \
        });


void SomeAsyncOperationWithResumeSignal(std::chrono::milliseconds duration, std::weak_ptr<H::Signal<void()>> resumeSignalWeak) {
    LOG_FUNCTION_SCOPE(L"SomeAsyncOperationWithResumeSignal()");

    H::Timer::Once(duration, [resumeSignalWeak] { // (1)
        auto resumeSignal = resumeSignalWeak.lock();
        if (!resumeSignal) {
            LOG_ERROR_D("resumeSignal expired");
            return;
        }
        (*resumeSignal)(); // (2)
        NOOP;
        });
}


class Prototype {
public:
    Prototype() = default;
    ~Prototype() = default;

    H::Async::CoTask<H::Async::PromiseDefault>::Ret_t TaskMethod() {
        LOG_FUNCTION_SCOPE(L"TaskMethod()");
        co_await H::Async::AsyncOperationWithResumeSignal([](std::weak_ptr<H::Signal<void()>> resumeSignalWeak) {
            //SomeAsyncOperationWithResumeSignal(resumeSignalWeak);
            });
        co_return;
    }
};



// The fixture for testing <YOUR_CLASS>.
class WorkQueueCoroutineTest : public testing::Test {
protected:
    // You can remove any or all of the following functions if their bodies would be empty.

    WorkQueueCoroutineTest()
        : oldErrorMode{ _set_error_mode(_OUT_TO_MSGBOX) } // prevent termination for console app if assert triggered
    {
        // You can do set-up work for each test here.
    }
    ~WorkQueueCoroutineTest() override {
        // You can do clean-up work that doesn't throw exceptions here.
        _set_error_mode(oldErrorMode);
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    void SetUp() override {
        // Code here will be called immediately after the constructor (right before each test).
    }
    void TearDown() override {
        // Code here will be called immediately after each test (right before the destructor).
    }

    // Class members declared here can be used by all tests in the test suite for <YOUR_CLASS>.

    void StartEventLoopFor(std::chrono::milliseconds timeout) {
        H::Timer::Once(timeout, [this] {
            LOG_DEBUG_D("Stop workQueue");
            workQueue.StopWork();
            });

        while (workQueue.IsWorking()) {
            auto item = workQueue.Pop();
            if (item.task) {
                LOG_FUNCTION_SCOPE("task <{}>()", item.descrtiption);
                item.task();
            }
        }
        LOG_DEBUG_D("workQueue finished");
    }

protected:
    int oldErrorMode;
    H::Async::AsyncTasks_Test asyncTasks;
    H::ConcurrentQueue<H::TaskItemWithDescription> workQueue;
};


// Tests that asyncTasks corectly added / handled lambda and start it in workQueue after showEvent
TEST_F(WorkQueueCoroutineTest, AsyncLambdaCalledInWorkQueueAfterShowEvent) {
    LOG_FUNCTION_SCOPE("TEST: AsyncLambdaCalledInWorkQueueAfterShowEvent()");

    std::atomic<bool> insideTaskLambdaBody = false;
    std::atomic<bool> insideShowEventLambdaBody = false;
    std::chrono::milliseconds taskLambda_startAfter = 500ms;
    std::chrono::milliseconds someAsyncOperation_duration = 1000ms;

    asyncTasks.SetResumeCallback([&](std::weak_ptr<H::Async::CoTaskBase> taskWeak) {
        workQueue.Push({ "signal ResumeNextCoTask()", [&, taskWeak] {
            EXPECT_FALSE(insideTaskLambdaBody);
            H::Async::SafeResume(taskWeak);
            EXPECT_FALSE(insideTaskLambdaBody);
            } });
    });

    asyncTasks.AddTaskLambda(taskLambda_startAfter, [&]() -> H::Async::AsyncTasks::Task::Ret_t {
        EXPECT_FALSE(insideShowEventLambdaBody);
        FUNCTION_SCOPE_SET_BOOL_VALUE(insideTaskLambdaBody, true);
        
        LOG_FUNCTION_SCOPE(L"TaskLambda()");
                
        auto timePointA = std::chrono::high_resolution_clock::now();
        co_await H::Async::AsyncOperationWithResumeSignal([&](std::weak_ptr<H::Signal<void()>> resumeSignalWeak) {
            SomeAsyncOperationWithResumeSignal(someAsyncOperation_duration, resumeSignalWeak);
            });
        auto timePointB = std::chrono::high_resolution_clock::now();
                
        EXPECT_FALSE(insideShowEventLambdaBody);
                
        std::chrono::duration<double> asyncOperation_realDuration = timePointB - timePointA;
        EXPECT_TRUE((asyncOperation_realDuration - someAsyncOperation_duration) < 100ms);
        co_return;
        });

    workQueue.Push({ "signal ShowEvent()", [&] {
        FUNCTION_SCOPE_SET_BOOL_VALUE(insideShowEventLambdaBody, true);
        EXPECT_FALSE(insideTaskLambdaBody);
        EXPECT_FALSE(asyncTasks.IsExecutingStarted());

        LOG_DEBUG_D("ShowEvent_Action_1");
        std::this_thread::sleep_for(taskLambda_startAfter / 2);

        EXPECT_TRUE(asyncTasks.GetResumeCallback() != nullptr);        
        bool started = asyncTasks.StartExecuting();
        EXPECT_TRUE(started);

        EXPECT_TRUE(asyncTasks.IsExecutingStarted());
        EXPECT_FALSE(insideTaskLambdaBody);
                  
        std::this_thread::sleep_for(taskLambda_startAfter / 2);
        std::this_thread::sleep_for(10ms);
        LOG_DEBUG_D("ShowEvent_Action_2");

        started = asyncTasks.StartExecuting(); // must be ignored if coroutine not finished
        EXPECT_FALSE(started);

        EXPECT_TRUE(asyncTasks.IsExecutingStarted());
        EXPECT_FALSE(insideTaskLambdaBody);

        LOG_DEBUG_D("ShowEvent_Action_last");
        return;
        } });

    StartEventLoopFor(taskLambda_startAfter + someAsyncOperation_duration + 1000ms);
    EXPECT_FALSE(asyncTasks.IsExecutingStarted());
    EXPECT_FALSE(insideTaskLambdaBody);
}



// Tests that added lambda and it captured args still valid when co-task resumed
TEST_F(WorkQueueCoroutineTest, AsyncLambdaCapturedArgsIsValid) {
    LOG_FUNCTION_SCOPE("TEST: AsyncLambdaCapturedArgsIsValid()");
    struct Temp {
        void TestAccessToArray() {
            for (int i = 0; i < 100'000; i++) {
                arrayOnStack[i] = i;
            }
        }
        int arrayOnStack[100'000];
    };

    asyncTasks.SetResumeCallback([&](std::weak_ptr<H::Async::CoTaskBase> taskWeak) {
        workQueue.Push({ "signal ResumeNextCoTask()", [&, taskWeak] {
            H::Async::SafeResume(taskWeak);
            } });
        });
    
    for (int i = 0; i < 10; i++) {
        auto temp = std::make_shared<Temp>();
        asyncTasks.AddTaskLambda(0ms, [temp]() -> H::Async::AsyncTasks::Task::Ret_t {
            LOG_FUNCTION_SCOPE(L"TaskLambda()");
            temp->TestAccessToArray(); // If lambda was destroyed here will be Access Violation
            co_return;
            });
    }

    workQueue.Push({ "signal ShowEvent()", [&] {
        asyncTasks.StartExecuting();
        return;
        } });

    StartEventLoopFor(1000ms);
}



int main(int argc, char** argv) {
    lg::DefaultLoggers::Init(".\\Logs\\main.log", lg::InitFlags::DefaultFlags);
    
    testing::InitGoogleTest(&argc, argv);
    int exitCode = RUN_ALL_TESTS();
    system("pause");
    return exitCode;
}