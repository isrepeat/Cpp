#pragma once
#include <Helpers/MediaFoundation/SampleInfo.h>
#include <Helpers/System.h>
#include <Helpers/Macros.h>
#include <mfobjects.h>
#include <d3d11.h>
#include <wrl.h>


namespace MEDIA_FOUNDATION_NS {
    class MFSampleDataAccessor {
    public:
        MFSampleDataAccessor(const Microsoft::WRL::ComPtr<IMFMediaBuffer>& buffer);
        ~MFSampleDataAccessor();

        NO_COPY(MFSampleDataAccessor);

        void* GetData();
        const void* GetData() const;
        size_t GetSize() const;

    private:
        Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer;
        void* bufferData;
        size_t bufferSize;

    };

    struct MFSample : MF::SampleInfo {
        Microsoft::WRL::ComPtr<IMFSample> pSample;

        MFSample();
        MFSample(H::Chrono::Hns pts, H::Chrono::Hns duration, Microsoft::WRL::ComPtr<IMFSample> pSample);
        MFSample(const MFSample& other);
        virtual ~MFSample();

        MFSampleDataAccessor GetDataAccessor();
    };


    struct MFVideoSample : MFSample {
        MFVideoSample(const MFSample& mfSample, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = nullptr);
        Microsoft::WRL::ComPtr<ID3D11Texture2D> GetTexture();

    private:
        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    };

    struct MFAudioSample : MFSample {
        std::vector<BYTE> audioData;

        MFAudioSample(const MFSample& mfSample, std::vector<BYTE> audioData);
    };
}