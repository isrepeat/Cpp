#include "MediaSample.h"

#define LOG_DEBUG_D(fmt, ...)

namespace MEDIA_FOUNDATION_NS {
    MFSampleDataAccessor::MFSampleDataAccessor(const Microsoft::WRL::ComPtr<IMFMediaBuffer>& buffer)
        : buffer{ buffer }
        , bufferData{ nullptr }
        , bufferSize{ 0 }
    {
        HRESULT hr = S_OK;
        BYTE* data;
        DWORD maxLength, length;

        hr = this->buffer->Lock(&data, &maxLength, &length);
        if (FAILED(hr)) {
            this->buffer = nullptr;
            H::System::ThrowIfFailed(hr);
        }
        else {
            bufferData = static_cast<void*>(data);
            bufferSize = static_cast<size_t>(length);
        }
    }

    MFSampleDataAccessor::~MFSampleDataAccessor() {
        if (this->buffer) {
            this->buffer->Unlock();
        }
    }


    void* MFSampleDataAccessor::GetData() {
        return bufferData;
    }

    const void* MFSampleDataAccessor::GetData() const {
        return bufferData;
    }

    size_t MFSampleDataAccessor::GetSize() const {
        return bufferSize;
    }



    MFSample::MFSample()
    {}

    MFSample::MFSample(H::Chrono::Hns pts, H::Chrono::Hns duration, Microsoft::WRL::ComPtr<IMFSample> pSample)
        : MF::SampleInfo(pts, duration)
        , pSample{ pSample }
    {
        LOG_DEBUG_D("MFSample(): pSample = {}", static_cast<void*>(this->pSample));
    }

    MFSample::MFSample(const MFSample& other)
        : MF::SampleInfo(other.pts, other.duration)
        , pSample{ other.pSample }
    {
        LOG_DEBUG_D("MFSample(const MFSample&): pSample = {}", static_cast<void*>(this->pSample));
    }

    MFSample::~MFSample() {
        LOG_DEBUG_D("~MFSample(): pSample = {}", static_cast<void*>(this->pSample));
    }

    MFSampleDataAccessor MFSample::GetDataAccessor() {
        assert(false && "TODO: implement");
        //return MFSampleDataAccessor{ buffer };
        return MFSampleDataAccessor{ nullptr };
    }


    MFVideoSample::MFVideoSample(const MFSample& mfSample, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture)
        : MFSample{ mfSample }
        , texture{ texture }
    {
        // TODO: rewrite
        if (this->texture) {
            this->pSample = nullptr; // force release pSample (because texture placed on same address)
        }
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> MFVideoSample::GetTexture() {
        if (this->texture) {
            return this->texture;
        }

        HRESULT hr = S_OK;

        DWORD mfSampleTotalLength = 0;
        hr = this->pSample->GetTotalLength(&mfSampleTotalLength);

        Microsoft::WRL::ComPtr<IMFMediaBuffer> mfSampleBuffer;
        hr = this->pSample->ConvertToContiguousBuffer(&mfSampleBuffer);
        H::System::ThrowIfFailed(hr);

        Microsoft::WRL::ComPtr<IMFDXGIBuffer> mfSampleDxgiBuffer;
        hr = mfSampleBuffer.As(&mfSampleDxgiBuffer);
        H::System::ThrowIfFailed(hr);

        Microsoft::WRL::ComPtr<ID3D11Texture2D> mfSampleTexture;
        hr = mfSampleDxgiBuffer->GetResource(IID_PPV_ARGS(&mfSampleTexture));
        H::System::ThrowIfFailed(hr);

        return mfSampleTexture;
    }

    MFAudioSample::MFAudioSample(const MFSample& mfSample, std::vector<BYTE> audioData)
        : MFSample{ mfSample }
        , audioData{ std::move(audioData) }
    {}
}