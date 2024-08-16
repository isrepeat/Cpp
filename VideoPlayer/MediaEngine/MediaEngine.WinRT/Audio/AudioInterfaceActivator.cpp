#include "pch.h"
#include "AudioInterfaceActivator.h"
#include <wrl\implements.h>

#if COMPILE_FOR_WINRT
namespace {
	HRESULT PerformSync(Windows::Foundation::IAsyncAction^ op) {
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
}

AudioInterfaceActivator::AudioInterfaceActivator() {
	this->fut = this->prom.get_future();
}

AudioInterfaceActivator::~AudioInterfaceActivator() {
}

AudioActivationCompletionArgs AudioInterfaceActivator::Activate(const wchar_t *id, Windows::UI::Core::CoreDispatcher ^uiDispatcher) {
	HRESULT hr = S_OK;
	AudioActivationCompletionArgs completionArgs;

	Microsoft::WRL::ComPtr<AudioInterfaceActivator> completionHandler;

	hr = Microsoft::WRL::MakeAndInitialize<AudioInterfaceActivator>(&completionHandler);
	H::System::ThrowIfFailed(hr);

	if (uiDispatcher) {
		HRESULT hr2 = S_OK;
		auto workItemHandler = ref new Windows::UI::Core::DispatchedHandler([&]() {
			hr = ActivateAudioInterfaceAsync(id, __uuidof(IAudioClient), nullptr, completionHandler.Get(), completionArgs.activeOp.GetAddressOf());
		});
		hr2 = PerformSync(uiDispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, workItemHandler));
		H::System::ThrowIfFailed(hr2);
	}
	else {
		hr = ActivateAudioInterfaceAsync(id, __uuidof(IAudioClient), nullptr, completionHandler.Get(), completionArgs.activeOp.GetAddressOf());
	}

	H::System::ThrowIfFailed(hr);
	completionArgs.future = completionHandler->fut;

	return completionArgs;
}

STDMETHODIMP AudioInterfaceActivator::ActivateCompleted(IActivateAudioInterfaceAsyncOperation *pAsyncOp) {
	HRESULT hr = S_OK, activatedResult = S_OK;
	Microsoft::WRL::ComPtr<IAudioClient> result;

	try {
		Microsoft::WRL::ComPtr<IUnknown> tmp;

		hr = pAsyncOp->GetActivateResult(&activatedResult, tmp.GetAddressOf());
		H::System::ThrowIfFailed(hr);
		H::System::ThrowIfFailed(activatedResult);

		hr = tmp.As(&result);
		H::System::ThrowIfFailed(hr);
	}
	catch (...) {
		this->prom.set_exception(std::current_exception());
	}

	this->prom.set_value(result);

	return hr;
}
#endif // COMPILE_FOR_WINRT
