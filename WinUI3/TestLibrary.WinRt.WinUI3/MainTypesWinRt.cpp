#include "pch.h"
#include "MainTypesWinRt.h"
#include "MainTypesWinRt.g.cpp"

namespace winrt::TestLibrary_WinRt_WinUI3::implementation {
    int32_t MainTypesWinRt::TestIntProperty() {
        return this->mainTypesNative.GetTestInt();
    }

    void MainTypesWinRt::TestIntProperty(int32_t value) {
        this->mainTypesNative.SetTestInt(value);
    }
}
