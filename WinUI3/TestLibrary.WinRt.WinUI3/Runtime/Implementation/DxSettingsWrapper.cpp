#include "pch.h"
#include "DxSettingsWrapper.h"
#include "DxSettingsWrapper.g.cpp"

#pragma comment(lib, "RuntimeObject.lib")

namespace winrt::TestLibrary_WinRt_WinUI3::implementation
{
    DxSettingsWrapper::DxSettingsWrapper() 
        //: dxSettings{ nullptr }
        : dxSettings{}
    {
        //this->dxSettings = winrt::make<winrt::Helpers::WinRt::Dx::DxSettings>();
    }

    bool DxSettingsWrapper::MSAA() {
        //if (auto res = this->dxSettingsCom.try_as<winrt::Helpers::WinRt::Dx::DxSettings>()) {
        //    return res.MSAA();
        //}
        return this->dxSettings.MSAA();
    }
    void DxSettingsWrapper::MSAA(bool value) {
        //this->dxSettings.MSAA(value);
    }
}
