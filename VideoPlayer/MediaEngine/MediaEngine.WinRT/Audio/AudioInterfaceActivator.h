#pragma once
#include <Helpers/System.h>
#include <Audioclient.h>
#include <Mmdeviceapi.h>
#include <future>
#include <wrl.h>

#if COMPILE_FOR_WINRT
struct AudioActivationCompletionArgs {
	Microsoft::WRL::ComPtr<IActivateAudioInterfaceAsyncOperation> activeOp;
	std::shared_future<Microsoft::WRL::ComPtr<IAudioClient>> future;
};

class AudioInterfaceActivator : public
	Microsoft::WRL::RuntimeClass<
	Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
	Microsoft::WRL::FtmBase,
	IActivateAudioInterfaceCompletionHandler>
{
public:
	AudioInterfaceActivator();
	virtual ~AudioInterfaceActivator();

	static AudioActivationCompletionArgs Activate(const wchar_t *id, Windows::UI::Core::CoreDispatcher ^uiDispatcher);

private:
	std::promise<Microsoft::WRL::ComPtr<IAudioClient>> prom;
	std::shared_future<Microsoft::WRL::ComPtr<IAudioClient>> fut;

	virtual STDMETHODIMP ActivateCompleted(IActivateAudioInterfaceAsyncOperation *pAsyncOp) override;
};
#endif // COMPILE_FOR_WINRT
