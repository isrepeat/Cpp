#pragma once
#include "MainTypesWinRt.g.h"
#include "MainTypesNative.h"

//#include <Helpers/Async/Awaitables.h>
//
//using RootTask = H::Async::CoTask<H::Async::PromiseRoot>;
//using Task = H::Async::CoTask<H::Async::PromiseDefault>;

namespace winrt::TestLibrary_WinRt_WinUI3::implementation {
    struct MainTypesWinRt : MainTypesWinRtT<MainTypesWinRt> {
        MainTypesWinRt() = default;
        
        //
        // Properties
        //
        int32_t TestIntProperty();
        void TestIntProperty(int32_t value);

        hstring TestStringProperty();
        void TestStringProperty(hstring const& value);
        
        hstring TestStringPropertyOnlyGet();

        //
        // Events
        //
        winrt::event_token ProgressEvent(winrt::Windows::Foundation::EventHandler<float> const& handler);
        void ProgressEvent(winrt::event_token const& token) noexcept;

        //
        // Methods
        //
        void DoSomething();

        winrt::Windows::Foundation::IAsyncAction TestAsyncAction();

        //Helpers::WinRt::Dx::DxSettings GetDxSettings();
        DxSettingsWrapper GetDxSettings();

    private:
        //winrt::Helpers::WinRt::Dx::DxSettings dxSettings;
        DxSettingsWrapper dxSettingsWrapper;

        TestLibraryNative::MainTypesNative mainTypesNative;
        winrt::event<Windows::Foundation::EventHandler<float>> progressEvent;
        //RootTask::Ret_t rootTask;
    };
}
namespace winrt::TestLibrary_WinRt_WinUI3::factory_implementation {
    struct MainTypesWinRt : MainTypesWinRtT<MainTypesWinRt, implementation::MainTypesWinRt> {
    };
}