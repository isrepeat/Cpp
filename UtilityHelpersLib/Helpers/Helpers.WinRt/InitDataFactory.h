#pragma once
#include "Dx/SwapChainPanel.h"

namespace Helpers {
    namespace WinRt {
        public ref class InitDataFactory sealed {
        public:
            static Helpers::WinRt::Dx::SwapChainPanelInitData CreateDefaultSwapChainPanelInitData();
        };
    }
}