#pragma once
#include "MainTypesWinRt.g.h"
#include "MainTypesNative.h"

namespace winrt::TestLibrary_WinRt_WinUI3::implementation {
    struct MainTypesWinRt : MainTypesWinRtT<MainTypesWinRt> {
        MainTypesWinRt() = default;

        int32_t TestIntProperty();
        void TestIntProperty(int32_t value);

    private:
        TestLibraryNative::MainTypesNative mainTypesNative;
    };
}
namespace winrt::TestLibrary_WinRt_WinUI3::factory_implementation {
    struct MainTypesWinRt : MainTypesWinRtT<MainTypesWinRt, implementation::MainTypesWinRt> {
    };
}
