#include "pch.h"
#include "MainTypesWinRt.h"
#include "MainTypesWinRt.g.cpp"


namespace {
    RootTask::Ret_t CustomAsyncTask(std::wstring coroFrameName, std::function<void(std::weak_ptr<H::Async::CoTaskBase>)> resumeCallback) {
        LOG_DEBUG_D(L"CustomAsyncTask()");

        co_await H::Async::ResumeAfter<RootTask::promise_type>(3s);

        //co_await H::Async::AsyncOperationWithResumeSignal([](std::weak_ptr<H::Signal<void()>> resumeSignalWeak) {
        //    LOG_DEBUG_D(L"CustomAsyncTask__AsyncOp()");
        //    Sleep(2'000);
        //    LOG_DEBUG_D("tp1");
        //    H::Async::ResumeCoroutineViaSignal(resumeSignalWeak);
        //    LOG_DEBUG_D("tp3");
        //    });

        LOG_DEBUG_D(L"<= CustomAsyncTask()");
        co_return;
    }
}

namespace winrt::TestLibrary_WinRt_WinUI3::implementation {
    //
    // Properties
    //
    int32_t MainTypesWinRt::TestIntProperty() {
        return this->mainTypesNative.GetTestInt();
    }
    void MainTypesWinRt::TestIntProperty(int32_t value) {
        this->mainTypesNative.SetTestInt(value);
    }

    hstring MainTypesWinRt::TestStringProperty() {
        return winrt::hstring{ this->mainTypesNative.GetTestWString() };
    }
    void MainTypesWinRt::TestStringProperty(const hstring& hstr) {
        this->mainTypesNative.SetTestWString(std::wstring{ hstr });
    }

    hstring MainTypesWinRt::TestStringPropertyOnlyGet() {
        return winrt::hstring{ this->mainTypesNative.GetTestWString() };
    }

    //
    // Events
    //
    winrt::event_token MainTypesWinRt::ProgressEvent(const winrt::Windows::Foundation::EventHandler<float>& handler) {
        return this->progressEvent.add(handler);
    }
    void MainTypesWinRt::ProgressEvent(const winrt::event_token& token) noexcept {
        this->progressEvent.remove(token);
    }

    //
    // Methods
    //
    void MainTypesWinRt::DoSomething() {
        if (this->progressEvent) {
            this->progressEvent(*this, 3.14f);
        }
    }

    winrt::Windows::Foundation::IAsyncAction MainTypesWinRt::TestAsyncAction() {
        LOG_DEBUG_D("TestAsyncAction()");

        auto resumeCoroutineCallback = [] (std::weak_ptr<H::Async::CoTaskBase> taskWeak) {
            try {
                H::Async::SafeResume(taskWeak);
            }
            catch (std::exception& ex) {
                LOG_ERROR_D("Catch std::exception = {}", ex.what());
            }
            catch (...) {
                LOG_ERROR_D(L"Catch unhandled exception");
            }
            };

        this->rootTask = CustomAsyncTask(L"rootTask", resumeCoroutineCallback);
        H::Async::SafeResume(this->rootTask);


        // TODO: add co_await StorageFile

        LOG_DEBUG_D("<= TestAsyncAction()");
        co_return;
    }
}