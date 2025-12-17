#include "pch.h"
#include "InitDataFactory.h"
#include "Helpers/Dx/SwapChainPanel.h"

namespace Helpers {
    namespace WinRt {
        Helpers::WinRt::Dx::SwapChainPanelInitData InitDataFactory::CreateDefaultSwapChainPanelInitData() {
            H::Dx::SwapChainPanel::InitData initDataNative;

            Helpers::WinRt::Dx::SwapChainPanelInitData initData;
            initData.deviceType = Helpers::WinRt::Dx::SwapChainPanelInitData_Device::DxDevice;
            initData.deviceMutexType = Helpers::WinRt::Dx::SwapChainPanelInitData_DeviceMutex::None;
            initData.optionFlags = Helpers::WinRt::Dx::SwapChainPanelInitData_Options::None;
            //initData.optionFlags = static_cast<Helpers::WinRt::Dx::SwapChainPanelInitData_Options>(initDataNative.optionFlags.ToEnum());
            return initData;
        }
    }
}