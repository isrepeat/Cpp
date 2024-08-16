#include "pch.h"
#include "AudioClientFactory.h"
#include "AudioInterfaceActivator.h"
#include <Helpers/Memory.h>

#include <mmdeviceapi.h>
#include <initguid.h>

#if COMPILE_FOR_WINRT
//namespace MediaEngine {
//    namespace WinRt {
        using namespace Windows::Media::Devices;

        AudioClientFactory::AudioClientFactory() {
            //#ifdef WINRT_API
            //    this->onDefaultAudioCaptureDeviceChangedToken = MediaDevice::DefaultAudioCaptureDeviceChanged += H::System::MakeTypedEventHandler(
            //        [=](Platform::Object ^sender, Windows::Media::Devices::DefaultAudioCaptureDeviceChangedEventArgs ^args) {
            //        if (args->Role != AudioDeviceRole::Default) {
            //            return;
            //        }
            //
            //        thread::critical_section::scoped_lock lk(this->eventsCs);
            //        this->OnCaptureDeviceChanged.Notify();
            //    });
            //
            //    this->onDefaultAudioRenderDeviceChangedToken = MediaDevice::DefaultAudioRenderDeviceChanged += H::System::MakeTypedEventHandler(
            //        [=](Platform::Object ^sender, Windows::Media::Devices::DefaultAudioRenderDeviceChangedEventArgs ^args) {
            //        if (args->Role != AudioDeviceRole::Default) {
            //            return;
            //        }
            //
            //        thread::critical_section::scoped_lock lk(this->eventsCs);
            //        this->OnRendererDeviceChanged.Notify();
            //    });
            //#endif
        }

        AudioClientFactory::~AudioClientFactory() {
            //#ifdef WINRT_API
            //    MediaDevice::DefaultAudioCaptureDeviceChanged -= this->onDefaultAudioCaptureDeviceChangedToken;
            //    MediaDevice::DefaultAudioRenderDeviceChanged -= this->onDefaultAudioRenderDeviceChangedToken;
            //#endif
        }

        Microsoft::WRL::ComPtr<IAudioClient> AudioClientFactory::CreateAudioClient(AudioClientType type) {
            Microsoft::WRL::ComPtr<IAudioClient> client;
            Platform::String^ audioDeviceId;

            if (type == AudioClientType::RenderAudioClient) {
                audioDeviceId = MediaDevice::GetDefaultAudioRenderId(AudioDeviceRole::Default);
            }
            else if (type == AudioClientType::CaptureAudioClient) {
                audioDeviceId = MediaDevice::GetDefaultAudioCaptureId(AudioDeviceRole::Default);
            }
            else {
                return client;
            }

            if (audioDeviceId == nullptr){
                throw ref new Platform::Exception(E_FAIL);
            }

            auto activationResult = AudioInterfaceActivator::Activate(audioDeviceId->Data(), nullptr);

            client = activationResult.future.get();
            return client;
        }
//    }
//}
#endif // COMPILE_FOR_WINRT