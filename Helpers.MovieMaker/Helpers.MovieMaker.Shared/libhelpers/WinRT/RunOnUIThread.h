#pragma once
#include "..\HSystem.h"

#if HAVE_WINRT == 1
namespace H {
    template<class Fn>
    void RunOnUIThread(Windows::UI::Core::CoreDispatcher^ uiDispatcher, Fn fn) {
        auto currentWindow = Windows::UI::Core::CoreWindow::GetForCurrentThread();
        if (currentWindow && currentWindow->Dispatcher == uiDispatcher) {
            // already on UI thread
            fn();
        }
        else {
			auto task = uiDispatcher->RunAsync(
				Windows::UI::Core::CoreDispatcherPriority::High,
				ref new Windows::UI::Core::DispatchedHandler([fn]
					{
                        fn();
					}, Platform::CallbackContext::Any));

            H::System::PerformSyncThrow(task);
        }
    }
}
#endif
