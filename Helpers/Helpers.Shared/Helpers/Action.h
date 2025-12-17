#pragma once
#include <Helpers/common.h>
#include <Helpers/System.h>
#include <functional>
#include <future>

namespace HELPERS_NS {
	// PerformActionWithAttempts also handling "std" | "any" exceptions
	class PerformActionWithAttempts {
	public:
		PerformActionWithAttempts(int attempts, std::function<void()> actionCallback);
		~PerformActionWithAttempts() = default;

		static void Break(); // throw CompleteException
	};

// use with HELPERS_NS::
#define BreakAction  PerformActionWithAttempts::Break()


#if COMPILE_FOR_WINRT
    namespace WinRt {
        inline HRESULT PerformSync(Windows::Foundation::IAsyncAction^ op) {
            std::promise<HRESULT> prom;
            auto fut = prom.get_future();

            op->Completed = ref new Windows::Foundation::AsyncActionCompletedHandler(
                [&](Windows::Foundation::IAsyncAction^ op, Windows::Foundation::AsyncStatus status) {
                    HRESULT res;

                    if (status == Windows::Foundation::AsyncStatus::Completed) {
                        res = S_OK;
                    }
                    else {
                        res = static_cast<HRESULT>(op->ErrorCode.Value);
                    }

                    prom.set_value(res);
                });

            auto result = fut.get();
            return result;
        }

        void PerformSyncThrow(Windows::Foundation::IAsyncAction^ op) {
            HELPERS_NS::System::ThrowIfFailed(PerformSync(op));
        }

        template <typename Fn>
        inline void RunOnUIThread(Windows::UI::Core::CoreDispatcher^ uiDispatcher, Fn fn) {
            auto currentWindow = Windows::UI::Core::CoreWindow::GetForCurrentThread();
            if (currentWindow && currentWindow->Dispatcher == uiDispatcher) {
                // already on UI thread
                fn();
            }
            else {
                auto task = uiDispatcher->RunAsync(
                    Windows::UI::Core::CoreDispatcherPriority::High,
                    ref new Windows::UI::Core::DispatchedHandler(
                        [fn] {
                            fn();
                        },
                        Platform::CallbackContext::Any)
                );
                PerformSyncThrow(task);
            }
        }
    }
#endif
}