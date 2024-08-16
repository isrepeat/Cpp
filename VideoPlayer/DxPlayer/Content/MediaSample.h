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
        MFSampleDataAccessor(const Microsoft::WRL::ComPtr<IMFMediaBuffer>& buffer)
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

        ~MFSampleDataAccessor() {
            if (this->buffer) {
                this->buffer->Unlock();
            }
        }

        NO_COPY(MFSampleDataAccessor);

        void* GetData() {
            return bufferData;
        }

        const void* GetData() const {
            return bufferData;
        }

        size_t GetSize() const {
            return bufferSize;
        }

    private:
        Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer;
        void* bufferData;
        size_t bufferSize;

    };

    struct MFSample : MF::SampleInfo {
        Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer;

        MFSample() {}
        MFSample(H::Chrono::Hns pts, H::Chrono::Hns duration, Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer)
            : MF::SampleInfo(pts, duration)
            , buffer{ buffer }
        {}
        virtual ~MFSample() {}

        MFSampleDataAccessor GetDataAccessor() {
            return MFSampleDataAccessor{ buffer };
        }
    };

    struct MFVideoSample : MFSample {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;

        MFVideoSample(const MFSample& mfSample, Microsoft::WRL::ComPtr<ID3D11Texture2D> texture)
            : MFSample{ mfSample }
            , texture{ texture }
        {}
    };

    struct MFAudioSample : MFSample {
        std::vector<BYTE> audioData;

        MFAudioSample(const MFSample& mfSample, std::vector<BYTE> audioData)
            : MFSample{ mfSample }
            , audioData{ std::move(audioData) }
        {}
    };
}