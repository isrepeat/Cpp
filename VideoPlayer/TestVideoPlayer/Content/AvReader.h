#pragma once
#include "IAvReaderEffect.h"
#include "MediaSample.h"
#include "AvReaderDxgiEffect.h"
#include "AvSourceStreamManager.h"
#include "AvReaderDxgiManager.h"
#include <Helpers/MediaFoundation/MediaEngineInclude.h>
#include <Helpers/Dx/DxDevice.h>
#include <Helpers/System.h>
#include <Helpers/Dx/DxgiDeviceLock.h>

namespace TestVideoPlayer {

    class AvReader
    {
    public:
        AvReader(IStream* istream, H::Dx::DxDeviceSafeObj* dxDeviceSafeObj);
        ~AvReader();

        bool HasSample();

        std::unique_ptr<MediaSample> ReadSample();

        void AddEffect(std::unique_ptr<IAvReaderEffect> effect);

        void SetLoopPlayback(bool enabled);

    private:
        Microsoft::WRL::ComPtr<AvSourceStreamManager> sourceStreamManager;
        Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader;
        Microsoft::WRL::ComPtr<IMFMediaType> videoType;
        Microsoft::WRL::ComPtr<IMFMediaType> audioType;
        std::vector<std::unique_ptr<IAvReaderEffect>> effects;

        H::Dx::DxDeviceSafeObj* dxDeviceSafeObj;

        bool loopPlayback;
    };
}
