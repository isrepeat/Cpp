#pragma once
#include "DxSettingsWrapper.g.h"
#include <winrt/Helpers.WinRt.Dx.h>

namespace winrt::TestLibrary_WinRt_WinUI3::implementation
{
    struct DxSettingsWrapper : DxSettingsWrapperT<DxSettingsWrapper>
    {
        DxSettingsWrapper();

        bool MSAA();
        void MSAA(bool value);
    
    private:
        winrt::Helpers::WinRt::Dx::DxSettings dxSettings;
        //winrt::com_ptr<winrt::Helpers::WinRt::Dx::DxSettings> dxSettingsCom;
    };
}
namespace winrt::TestLibrary_WinRt_WinUI3::factory_implementation
{
    struct DxSettingsWrapper : DxSettingsWrapperT<DxSettingsWrapper, implementation::DxSettingsWrapper>
    {
    };
}
