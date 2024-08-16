#pragma once
#include <MediaEngine/Audio/IAudioClientFactory.h>
#include <Helpers/System.h>

#if COMPILE_FOR_WINRT
//namespace MediaEngineWinRT {
//    namespace WinRt {
        class AudioClientFactory : public IAudioClientFactory {
        public:
            AudioClientFactory();
            virtual ~AudioClientFactory();

            Microsoft::WRL::ComPtr<IAudioClient> CreateAudioClient(AudioClientType type) override;
        };
//    }
//}
#endif