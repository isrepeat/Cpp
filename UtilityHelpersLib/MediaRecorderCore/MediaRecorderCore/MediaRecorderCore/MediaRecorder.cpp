#include "pch.h"
#include "MediaRecorder.h"
#include "CodecsTable.h"
#include "MediaFormat/MediaFormatCodecsSupport.h"
#include "Platform/PlatformClassFactory.h"
#include <Helpers/MediaFoundation/MediaTypeInfo.h>

#include <limits>
#include <filesystem>
#include <libhelpers/MediaFoundation/MFHelpers.h>
#include <libhelpers/HardDrive.h>
#include <libhelpers/HSystem.h>
#include <libhelpers/HMathCP.h>
#include <libhelpers/HTime.h>
#include <mfapi.h>
#include <codecapi.h>
#if COMPILE_FOR_CX_or_WINRT
#include <icodecapi.h>
#endif
#include <strmif.h>


// encoder restrictions can be found here : https://msdn.microsoft.com/en-us/library/windows/desktop/dd742785(v=vs.85).aspx
const uint32_t MediaRecorder::AllowedNumChannels[] = { 1 , 2 };
const uint32_t MediaRecorder::AllowedSampleRate[] = { 44100, 48000 };
const uint32_t MediaRecorder::AllowedBytesPerSecond[] = { 12000, 16000, 20000, 24000 };

MediaRecorder::MediaRecorder(
    IMFByteStream* outputStream,
    MediaRecorderParams params,
    UseHardwareTransformsForEncoding hardwareTransformsForEncoding,
    UseNv12VideoSamples nv12VideoSamples,
    std::shared_ptr<IEvent<Native::MediaRecorderEventArgs>> recordEventCallback)
    : stream(outputStream)
    , params(std::move(params))
    , hardwareTransformsForEncoding{ hardwareTransformsForEncoding }
    , nv12VideoSamples{ nv12VideoSamples }
    , containerExt{ this->params.mediaFormat.GetMediaContainerFileExtension() }
    , lastChunkCreatedTime{ H::Time::GetCurrentLibTime() }
    , recordEventCallback{ recordEventCallback }
{
    if (this->params.UseChunkMerger) {
        targetRecordDisk = H::HardDrive::GetDiskLetterFromPath(this->params.targetRecordPath);
    }

    auto newStream = this->params.UseChunkMerger ? StartNewChunk() : this->stream.Get();
    this->InitializeSinkWriter(newStream, hardwareTransformsForEncoding, nv12VideoSamples);
}

bool MediaRecorder::IsChunkMergerEnabled() {
    return params.UseChunkMerger;
}

bool MediaRecorder::HasAudio() const {
    bool has = this->audioStreamIdx != MediaRecorder::GetInvalidStreamIdx();
    return has;
}

bool MediaRecorder::HasVideo() const {
    bool has = this->videoStreamIdx != MediaRecorder::GetInvalidStreamIdx();
    return has;
}

int64_t MediaRecorder::LastVideoPtsHns() const {
    return this->videoPtsHns;
}

int64_t MediaRecorder::LastAudioPtsHns() const {
    return this->audioPtsHns;
}

int64_t MediaRecorder::LastPtsHns() const {
    auto ptsHns = (std::max)(this->audioPtsHns, this->videoPtsHns);
    return ptsHns;
}

MF::SampleInfo MediaRecorder::LastWritedAudioSample() const {
    if (!this->lastWritedAudioSample)
        return {};

    return *this->lastWritedAudioSample;
}

MF::SampleInfo MediaRecorder::LastWritedVideoSample() const {
    if (!this->lastWritedVideoSample)
        return {};

    return *this->lastWritedVideoSample;
}

bool MediaRecorder::ChunkAudioSamplesWritten() const {
    bool written = this->samplesNumber > 0;
    return written;
}

bool MediaRecorder::ChunkVideoSamplesWritten() const {
    bool written = this->framesNumber > 0;
    return written;
}

void MediaRecorder::StartRecord() {
    HRESULT hr = S_OK;

    hr = this->sinkWriter->BeginWriting();
    H::System::ThrowIfFailed(hr);
#if SPDLOG_ENABLED
    LOG_DEBUG("MediaRecorder::StartRecord started recording");
#endif
}

// TODO: Add guard for case when EndRecord called from other thread during record
// NOTE: we can't use mutex because it need custom copy Ctor(mb add smth like ObjectLocker)
void MediaRecorder::Record(const Microsoft::WRL::ComPtr<IMFSample> &sample, bool audio) {
    if (this->recordingErrorOccured) {
        // don't write samples after recording error
        // because it can throw/report more different exceptions
        // than the one that caused the initial recording error
        return;
    }

    HRESULT hr = S_OK;
    int64_t samplePts = 0;
    int64_t sampleDuration = 0;

    if (params.UseChunkMerger) {
        if (audio) {
            //https://stackoverflow.com/questions/33401149/ffmpeg-adding-0-05-seconds-of-silence-to-transcoded-aac-file
            if (samplesNumber > (int)params.mediaFormat.GetAudioCodecSettings()->GetBasicSettings()->sampleRate * 10 && samplesNumber % 1024 == 0) {
                ResetSinkWriterOnNewChunk();
            }
        } else {
            if (framesNumber > (int)params.mediaFormat.GetVideoCodecSettings()->GetBasicSettings()->fps * 5) {
                ResetSinkWriterOnNewChunk();
            }
        }
    }

    hr = sample->GetSampleTime(&samplePts);
    H::System::ThrowIfFailed(hr);

    hr = sample->GetSampleDuration(&sampleDuration);
    H::System::ThrowIfFailed(hr);

    DWORD streamIdx = (DWORD)-1;
    int64_t *ptsPtr = nullptr;

    if (audio) {
        Microsoft::WRL::ComPtr<IMFMediaBuffer> mediaBuffer;
        hr = sample->ConvertToContiguousBuffer(&mediaBuffer);
      
        DWORD len;
        hr = mediaBuffer->GetCurrentLength(&len);
        auto samples = len / (AudioSampleBits / 8 * params.mediaFormat.GetAudioCodecSettings()->GetBasicSettings()->numChannels);

        samplesNumber += samples;
        streamIdx = this->audioStreamIdx;
        ptsPtr = &this->audioPtsHns;
        sample->SetSampleTime(chunkAudioPtsHns);
        chunkAudioPtsHns += sampleDuration;
    }
    else {
        streamIdx = this->videoStreamIdx;
        ptsPtr = &this->videoPtsHns;
        sample->SetSampleTime(chunkVideoPtsHns);
        chunkVideoPtsHns += sampleDuration;
        ++framesNumber;
    }

    this->WriteSample(sample, streamIdx);
    *ptsPtr = samplePts + sampleDuration;
}


void MediaRecorder::RecordVideoSample(const Microsoft::WRL::ComPtr<IMFSample>& sample) {
    if (this->recordingErrorOccured) {
        LOG_ERROR_D("Was recording error before, ignore");
        // don't write samples after recording error
        // because it can throw/report more different exceptions
        // than the one that caused the initial recording error
        return;
    }

    HRESULT hr = S_OK;
    hr = sample->SetSampleTime((int64_t)this->LastWritedVideoSample().nextSamplePts);
    H::System::ThrowIfFailed(hr);

    // NOTE: sample duration is set outside because it depend on frameRate

    this->WriteSample(sample, this->videoStreamIdx);
    this->framesNumber++;
}


void MediaRecorder::RecordAudioBuffer(const float* audioSamples, size_t samplesCountForAllChannels) {
    if (this->recordingErrorOccured) {
        LOG_ERROR_D("Was recording error before, ignore");
        return;
    }

    HRESULT hr = S_OK;

    assert(this->HasAudio());
    assert(this->params.mediaFormat.GetAudioCodecSettings());
    assert(this->params.mediaFormat.GetAudioCodecSettings()->GetBasicSettings());

    // 'samplesCountForAllChannels' must be multiple of 'numChannels' in order to write full audio frame
    auto basicSettings = this->params.mediaFormat.GetAudioCodecSettings()->GetBasicSettings();
    assert(samplesCountForAllChannels % basicSettings->numChannels == 0);

    // TODO: Rewrite without int16_t
    // sample size = 2 byte == AudioSampleBits / 8 (AudioSampleBits == 16 for PCM)
    const DWORD bufferByteSize = (uint32_t)(samplesCountForAllChannels * sizeof(int16_t)); 
    
    Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer;
    hr = MFCreateMemoryBuffer(bufferByteSize, buffer.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    BYTE* bufferData = nullptr;
    hr = buffer->Lock(&bufferData, NULL, NULL);

    // TODO: rewrite without int16_t
    if (SUCCEEDED(hr)) {
        auto src = audioSamples;
        auto dst = reinterpret_cast<int16_t*>(bufferData);

        for (size_t i = 0; i < samplesCountForAllChannels; i++) {
            dst[i] = (int16_t)(src[i] * INT16_MAX);
        }
    }

    buffer->Unlock();
    H::System::ThrowIfFailed(hr);

    hr = buffer->SetCurrentLength(bufferByteSize);
    H::System::ThrowIfFailed(hr);


    int64_t samplesCountPerChannel = samplesCountForAllChannels / basicSettings->numChannels;

    int64_t durationHns = H::MathCP::Convert2(
        samplesCountPerChannel,
        (int64_t)basicSettings->sampleRate,
        (int64_t)H::Time::HNSResolution);

    this->WriteSample(buffer, (int64_t)this->LastWritedAudioSample().nextSamplePts, durationHns, this->audioStreamIdx);
    this->samplesNumber += samplesCountPerChannel;
}


void MediaRecorder::EndRecord() {
    if (params.UseChunkMerger) {
        // when finishing record do not report recording error messages, exception is enough
        try {
            FinalizeRecord(false);
        }
        catch(...) {}
        // ignore errors with finalization of last chunk, try to merge

        std::vector<std::wstring> chunks;
        chunks.reserve(chunkNumber);
        for (size_t i = 0; i < chunkNumber; ++i) {
            auto chunkFilePath = GetChunkFilePath(i);
            if (std::filesystem::file_size(chunkFilePath) > 0)
                chunks.push_back(std::move(chunkFilePath));
        }

        if (chunks.empty()) {
            H::System::ThrowIfFailed(E_FAIL);
        }

        // Ignore last chunk to avoid MF_E_INVALIDSTREAMNUMBER when not enough space on disk 
        // But left at least 1 chunk to try to merge and finalize file
        if (chunks.size() > 1 && recordedChunksSize >= H::HardDrive::GetFreeMemory(targetRecordDisk)) {
            chunks.pop_back();
        }

        MergeChunks(this->stream.Get(), std::move(chunks));
    }
    else {
        // when finishing record do not report recording error messages, exception is enough
        FinalizeRecord(false);
    }
}

std::wstring MediaRecorder::GetChunkFilePath(size_t chunkIndex)
{
    //TODO remove extension
    return params.chunksPath + params.chunksGuid + L"-" + std::to_wstring(chunkIndex) + containerExt;
}

void MediaRecorder::Restore(IMFByteStream* outputStream, std::vector<std::wstring>&& chunks) {
    if (!params.UseChunkMerger)
        H::System::ThrowIfFailed(E_NOTIMPL);

    MergeChunks(outputStream, std::move(chunks));
}

const MediaRecorderParams &MediaRecorder::GetParams() const {
    return this->params;
}

Microsoft::WRL::ComPtr<IMFMediaType> MediaRecorder::GetVideoTypeIn() const {
    return this->videoTypeIn;
}

Microsoft::WRL::ComPtr<IMFMediaType> MediaRecorder::GetVideoTypeOut() const {
    return this->videoTypeOut;
}

void MediaRecorder::Write(const float* audioSamples, size_t valuesCount, int64_t hns) {
    HRESULT hr = S_OK;
    BYTE *bufferData = nullptr;
    Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer;
    const DWORD bufferByteSize = (uint32_t)(valuesCount * sizeof(int16_t));

    // <valuesCount> must be multiple of <NumAudioChannels> in order to write full audio frame
    assert(this->params.mediaFormat.GetAudioCodecSettings());
    assert(this->params.mediaFormat.GetAudioCodecSettings()->GetBasicSettings());
    assert(valuesCount % this->params.mediaFormat.GetAudioCodecSettings()->GetBasicSettings()->numChannels == 0);
    assert(this->HasAudio());

    hr = MFCreateMemoryBuffer(bufferByteSize, buffer.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = buffer->Lock(&bufferData, NULL, NULL);

    if (SUCCEEDED(hr)) {
        auto src = audioSamples;
        auto dst = reinterpret_cast<int16_t *>(bufferData);

        for (size_t i = 0; i < valuesCount; i++) {
            dst[i] = (int16_t)(src[i] * INT16_MAX);
        }
    }

    buffer->Unlock();
    H::System::ThrowIfFailed(hr);

    hr = buffer->SetCurrentLength(bufferByteSize);
    H::System::ThrowIfFailed(hr);

    auto basicSettings = this->params.mediaFormat.GetAudioCodecSettings()->GetBasicSettings();

    int64_t sampleCount = valuesCount / basicSettings->numChannels;

    int64_t durationHns = H::MathCP::Convert2(
        sampleCount,
        (int64_t)basicSettings->sampleRate,
        (int64_t)H::Time::HNSResolution);

    if (hns >= 0) {
        this->audioPtsHns = hns;
    }

    this->WriteSample(buffer, this->audioPtsHns, durationHns, this->audioStreamIdx);

    this->audioPtsHns += durationHns;
}


void MediaRecorder::Write(ID3D11DeviceContext* d3dCtx, ID3D11Texture2D* tex, int64_t hns, int64_t durationHns) {
    assert(this->params.DxBufferFactory);

    Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer = this->params.DxBufferFactory->CreateBuffer(d3dCtx, tex);

    this->WriteVideoSample(buffer, hns, durationHns);
}

void MediaRecorder::Write(const void* videoData, size_t rowPitch, int64_t hns, int64_t durationHns) {
    assert(this->params.mediaFormat.GetVideoCodecSettings());
    assert(this->params.mediaFormat.GetVideoCodecSettings()->GetBasicSettings());

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer;
    BYTE *bufferData = nullptr;
    auto basicSettings = this->params.mediaFormat.GetVideoCodecSettings()->GetBasicSettings();
    DWORD bufferByteSize = basicSettings->width * basicSettings->height * 4;
    hr = MFCreateMemoryBuffer(bufferByteSize, buffer.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = buffer->Lock(&bufferData, NULL, NULL);

    if (SUCCEEDED(hr)) {
        if (basicSettings->width * 4 != rowPitch) {
            auto src = static_cast<const uint8_t*>(videoData);
            auto dst = bufferData;

            for (uint32_t y = 0; y < basicSettings->height; y++, src += rowPitch, dst += basicSettings->height * 4) {
                std::memcpy(dst, src, basicSettings->width * 4);
            }
        }
        else {
            std::memcpy(bufferData, videoData, basicSettings->width * basicSettings->height * 4);
        }
    }

    buffer->Unlock();
    H::System::ThrowIfFailed(hr);

    hr = buffer->SetCurrentLength(bufferByteSize);
    H::System::ThrowIfFailed(hr);

    this->WriteVideoSample(buffer, hns, durationHns);
}

void MediaRecorder::InitializeSinkWriter(
    IMFByteStream* outputStream,
    UseHardwareTransformsForEncoding useHardwareTransformsForEncoding,
    UseNv12VideoSamples useNv12VideoSamples)
{
    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFByteStream> byteStream = outputStream;
    Microsoft::WRL::ComPtr<IMFAttributes> sinkAttr;

    hr = MFCreateAttributes(sinkAttr.GetAddressOf(), 4);
    H::System::ThrowIfFailed(hr);

    // disable throttling for cases when many samples accumulated during overall high system load
    // and then trying to write many samples in short period of time
    // https://learn.microsoft.com/en-us/windows/win32/medfound/mf-sink-writer-disable-throttling
    hr = sinkAttr->SetUINT32(MF_SINK_WRITER_DISABLE_THROTTLING, TRUE);
    H::System::ThrowIfFailed(hr);

    if (static_cast<bool>(useHardwareTransformsForEncoding)) {
        hr = sinkAttr->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
        H::System::ThrowIfFailed(hr);
    }

    if (this->params.DxBufferFactory) {
        this->params.DxBufferFactory->SetAttributes(sinkAttr.Get());
    }

    hr = MFCreateSinkWriterFromURL(containerExt.c_str(), byteStream.Get(), sinkAttr.Get(), this->sinkWriter.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    if (this->params.mediaFormat.GetAudioCodecSettings()) {

        auto settings = this->params.mediaFormat.GetAudioCodecSettings();
        Microsoft::WRL::ComPtr<IMFMediaType> typeOut, typeIn;

        typeIn = MediaRecorder::CreateAudioInMediaType(settings, AudioSampleBits);

        switch (settings->GetCodecType()) {
        case AudioCodecType::AAC:
            typeOut = CreateAudioAACOutMediaType();
            break;

        default:
            typeOut = CreateAudioOutMediaType(settings, AudioSampleBits);
        }

        //typeOut = CreateAudioFlacOutMediaType();
        //typeOut = MediaRecorder::CreateAudioInMediaType(settings, AudioSampleBits);

        hr = this->sinkWriter->AddStream(typeOut.Get(), &this->audioStreamIdx);
        H::System::ThrowIfFailed(hr);

        hr = this->sinkWriter->SetInputMediaType(this->audioStreamIdx, typeIn.Get(), NULL);
        H::System::ThrowIfFailed(hr);
    }

    if (this->params.mediaFormat.GetVideoCodecSettings()) {
        auto settings = this->params.mediaFormat.GetVideoCodecSettings();
        auto basicSettings = settings->GetBasicSettings();
        Microsoft::WRL::ComPtr<IMFMediaType> typeOut, typeIn;

        if (basicSettings) {
            if (!static_cast<bool>(nv12VideoSamples) && basicSettings->height > 1080 && settings->GetCodecType() == VideoCodecType::HEVC) {
                // TODO check why crash with nv12VideoSamples == false and >1080(2k, 4k) HEVC
                assert(false);
                H::System::ThrowIfFailed(E_FAIL);
            }
        }

        typeIn = MediaRecorder::CreateVideoInMediaType(settings, useNv12VideoSamples);
        typeOut = MediaRecorder::CreateVideoOutMediaType(settings, this->params.mediaFormat.GetMediaContainerType(), params.UseChunkMerger);

        hr = this->sinkWriter->AddStream(typeOut.Get(), &this->videoStreamIdx);
        H::System::ThrowIfFailed(hr);

        hr = this->sinkWriter->SetInputMediaType(this->videoStreamIdx, typeIn.Get(), nullptr);
        H::System::ThrowIfFailed(hr);

		if (basicSettings && settings->GetCodecType() == VideoCodecType::H264) {
			Microsoft::WRL::ComPtr<ICodecAPI> codecApi;
			hr = this->sinkWriter->GetServiceForStream(this->videoStreamIdx, GUID_NULL, IID_PPV_ARGS(&codecApi));
			H::System::ThrowIfFailed(hr);

			eAVEncCommonRateControlMode rateControlMode{eAVEncCommonRateControlMode_UnconstrainedVBR};
			switch (basicSettings->rateControlMode) {
			case VideoRateControlMode::CBR:
				rateControlMode = eAVEncCommonRateControlMode_CBR;
				break;

            case VideoRateControlMode::PeakConstrainedVBR: {
                rateControlMode = eAVEncCommonRateControlMode_PeakConstrainedVBR;
                auto bitrate = MFHelpers::MakeVariantUINT(basicSettings->bitrate);
                codecApi->SetValue(&CODECAPI_AVEncCommonMaxBitRate, &bitrate);
                break;
            }

			default:
				break;
			}

            // Ignore default value
            if (rateControlMode != eAVEncCommonRateControlMode_UnconstrainedVBR) {
                auto rateControlModeVal = MFHelpers::MakeVariantUINT(rateControlMode);
                codecApi->SetValue(&CODECAPI_AVEncCommonRateControlMode, &rateControlModeVal);
            }
		}

        this->videoTypeIn = typeIn;
        this->videoTypeOut = typeOut;
    }
}

Microsoft::WRL::ComPtr<IMFMediaType> MediaRecorder::CreateAudioInMediaType(
    const IAudioCodecSettings *settings,
    uint32_t bitsPerSample)
{
    LOG_FUNCTION_SCOPE_VERBOSE("CreateAudioInMediaType()");

    if (settings == nullptr)
        return nullptr;

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFMediaType> mediaType;

    hr = MFCreateMediaType(mediaType.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    H::System::ThrowIfFailed(hr);

    hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    H::System::ThrowIfFailed(hr);

    hr = mediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample);
    H::System::ThrowIfFailed(hr);

    if (auto basicSettings = settings->GetBasicSettings()) {
        auto numChannels = basicSettings->numChannels;
        auto sampleRate = basicSettings->sampleRate;

        hr = mediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, numChannels);
        H::System::ThrowIfFailed(hr);

        hr = mediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sampleRate);
        H::System::ThrowIfFailed(hr);

        hr = mediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, (bitsPerSample / 8) * numChannels * sampleRate);
        H::System::ThrowIfFailed(hr);

        hr = mediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, (bitsPerSample / 8) * numChannels);
        H::System::ThrowIfFailed(hr);
    }

    hr = mediaType->SetUINT32(MF_MT_COMPRESSED, FALSE);
    H::System::ThrowIfFailed(hr);

    hr = mediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
    H::System::ThrowIfFailed(hr);

    MF::Helpers::PrintAudioMediaTypeInfo(mediaType, "[audioTypeIn]");
    return mediaType;
}

Microsoft::WRL::ComPtr<IMFMediaType> MediaRecorder::CreateAudioOutMediaType(
    const IAudioCodecSettings *settings,
    uint32_t bitsPerSample)
{
    LOG_FUNCTION_SCOPE_VERBOSE("CreateAudioOutMediaType()");

    if (settings == nullptr)
        return nullptr;

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFMediaType> mediaType;

    // Output type
    hr = MFCreateMediaType(mediaType.ReleaseAndGetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    H::System::ThrowIfFailed(hr);

    auto codecSup = MediaFormatCodecsSupport::Instance();
    auto codecType = settings->GetCodecType();
    auto acodecId = codecSup->MapAudioCodec(codecType);

    hr = mediaType->SetGUID(MF_MT_SUBTYPE, acodecId);
    H::System::ThrowIfFailed(hr);

    auto basicSettings = settings->GetBasicSettings();
    auto bitrateSettings = settings->GetBitrateSettings();

    if (basicSettings) {
        hr = mediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, basicSettings->numChannels);
        H::System::ThrowIfFailed(hr);

        hr = mediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, basicSettings->sampleRate);
        H::System::ThrowIfFailed(hr);
    }

    switch (codecType) {
    case AudioCodecType::MP3: {
        if (!basicSettings || !bitrateSettings) {
            break;
        }

        auto factory = PlatformClassFactory::Instance()->CreateMP3CodecFactory();
        auto transform = factory->CreateIMFTransform();

        mediaType = MediaRecorder::GetBestOutputType(
            transform.Get(), mediaType.Get(), acodecId,
            basicSettings->numChannels, basicSettings->sampleRate,
            bitsPerSample, bitrateSettings->bitrate);
        break;
    }
    case AudioCodecType::WMAudioV8: {
        if (!basicSettings || !bitrateSettings) {
            break;
        }

        auto factory = PlatformClassFactory::Instance()->CreateWma8CodecFactory();
        auto transform = factory->CreateIMFTransform();

        mediaType = MediaRecorder::GetBestOutputType(
            transform.Get(), mediaType.Get(), acodecId,
            basicSettings->numChannels, basicSettings->sampleRate,
            bitsPerSample, bitrateSettings->bitrate);
        break;
    }
    case AudioCodecType::AMR_NB: {
        if (!basicSettings || !bitrateSettings) {
            break;
        }

        auto factory = PlatformClassFactory::Instance()->CreateAmrNbCodecFactory();
        auto transform = factory->CreateIMFTransform();

        if (transform) {
            mediaType = MediaRecorder::GetBestOutputType(
                transform.Get(), mediaType.Get(), acodecId,
                basicSettings->numChannels, basicSettings->sampleRate,
                bitsPerSample, bitrateSettings->bitrate);
        }

        break;
    }
    case AudioCodecType::FLAC:
    case AudioCodecType::ALAC:
    case AudioCodecType::PCM: {
        if (!basicSettings) {
            break;
        }

        hr = mediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, (bitsPerSample / 8) * basicSettings->numChannels * basicSettings->sampleRate);
        H::System::ThrowIfFailed(hr);

        hr = mediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, (bitsPerSample / 8) * basicSettings->numChannels);
        H::System::ThrowIfFailed(hr);

        hr = mediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample);
        H::System::ThrowIfFailed(hr);

        /*hr = mediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
        H::System::ThrowIfFailed(hr);*/

        break;
    }
    }

    if (mediaType->GetItem(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, nullptr) != S_OK && bitrateSettings) {
        hr = mediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, bitrateSettings->bitrate / 8);
        H::System::ThrowIfFailed(hr);
    }
    
    MF::Helpers::PrintAudioMediaTypeInfo(mediaType, "[audioTypeOut]");
    return mediaType;
}

//TODO maybe replace codec
Microsoft::WRL::ComPtr<IMFMediaType> MediaRecorder::CreateAudioAACOutMediaType() {
    LOG_FUNCTION_SCOPE_VERBOSE("CreateAudioAACOutMediaType()");

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFMediaType> mediaType;

    // Output type
    hr = MFCreateMediaType(mediaType.ReleaseAndGetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    H::System::ThrowIfFailed(hr);

    hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
    H::System::ThrowIfFailed(hr);

    auto basicSettings = this->params.mediaFormat.GetAudioCodecSettings()->GetBasicSettings();
    auto bitrateSettings = this->params.mediaFormat.GetAudioCodecSettings()->GetBitrateSettings();

    if (basicSettings) {
        hr = mediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, basicSettings->numChannels);
        H::System::ThrowIfFailed(hr);

        hr = mediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, basicSettings->sampleRate);
        H::System::ThrowIfFailed(hr);
    }

    if (mediaType->GetItem(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, nullptr) != S_OK && bitrateSettings) {
        hr = mediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, bitrateSettings->bitrate / 8);
        H::System::ThrowIfFailed(hr);
    }
    
    MF::Helpers::PrintAudioMediaTypeInfo(mediaType, "[audioAacTypeOut]");
    return mediaType;
}

Microsoft::WRL::ComPtr<IMFMediaType> MediaRecorder::CreateAudioFlacOutMediaType() {
    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFMediaType> mediaType;
    
    // Output type
    hr = MFCreateMediaType(mediaType.ReleaseAndGetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    H::System::ThrowIfFailed(hr);

    hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_FLAC);
    H::System::ThrowIfFailed(hr);

    auto basicSettings = this->params.mediaFormat.GetAudioCodecSettings()->GetBasicSettings();
    
    if (basicSettings) {
        hr = mediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, basicSettings->numChannels);
        H::System::ThrowIfFailed(hr);

        hr = mediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, (AudioSampleBits / 8) * basicSettings->numChannels);
        H::System::ThrowIfFailed(hr);

        hr = mediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, AudioSampleBits);
        H::System::ThrowIfFailed(hr);

        hr = mediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, basicSettings->sampleRate);
        H::System::ThrowIfFailed(hr);

        hr = mediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, (AudioSampleBits / 8) * basicSettings->numChannels * basicSettings->sampleRate);
        H::System::ThrowIfFailed(hr);
    }
    
    return mediaType;
}

Microsoft::WRL::ComPtr<IMFMediaType> MediaRecorder::CreateVideoInMediaType(
    const IVideoCodecSettings *settings, UseNv12VideoSamples nv12VideoSamples)
{
    LOG_FUNCTION_SCOPE_VERBOSE("CreateVideoInMediaType()");

    if (settings == nullptr)
        return nullptr;

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFMediaType> mediaType;

    hr = MFCreateMediaType(mediaType.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    H::System::ThrowIfFailed(hr);

    if (static_cast<bool>(nv12VideoSamples))
    {
        hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
        H::System::ThrowIfFailed(hr);
    }
    else
    {
        hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32);
        H::System::ThrowIfFailed(hr);
    }

    hr = mediaType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    H::System::ThrowIfFailed(hr);

    auto basicSettings = settings->GetBasicSettings();

    if (basicSettings) {
        hr = MFSetAttributeSize(mediaType.Get(), MF_MT_FRAME_SIZE, basicSettings->width, basicSettings->height);
        H::System::ThrowIfFailed(hr);

        hr = MFSetAttributeRatio(mediaType.Get(), MF_MT_FRAME_RATE, basicSettings->fps, 1);
        H::System::ThrowIfFailed(hr);
    }

    hr = MFSetAttributeRatio(mediaType.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    H::System::ThrowIfFailed(hr);

    MF::Helpers::PrintVideoMediaTypeInfo(mediaType, "[videoTypeIn]");
    return mediaType;
}

Microsoft::WRL::ComPtr<IMFMediaType> MediaRecorder::CreateVideoOutMediaType(
    const IVideoCodecSettings *settings, MediaContainerType containerType, bool useChunkMerger)
{
    LOG_FUNCTION_SCOPE_VERBOSE("CreateVideoOutMediaType()");

    if (settings == nullptr)
        return nullptr;

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFMediaType> mediaType;

    hr = MFCreateMediaType(mediaType.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    H::System::ThrowIfFailed(hr);

    if (useChunkMerger && containerType == MediaContainerType::MP4) {
        hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264); // WORKAROUND for merging HEVC [see ChangeMerger notes]
        H::System::ThrowIfFailed(hr);
    }
    else {
        auto codecSup = MediaFormatCodecsSupport::Instance();
        auto vcodecId = codecSup->MapVideoCodec(settings->GetCodecType());
        hr = mediaType->SetGUID(MF_MT_SUBTYPE, vcodecId);
        H::System::ThrowIfFailed(hr);
    }

    // default profile of H264 can fail on sink->Finalize with video bitrate > 80 mbits.
    //uint32_t avgVideoBitrate = (std::min)(videoParams->AvgBitrate, uint32_t(80 * 1000000));

    hr = mediaType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    H::System::ThrowIfFailed(hr);

    hr = MFSetAttributeRatio(mediaType.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    H::System::ThrowIfFailed(hr);

    auto basicSettings = settings->GetBasicSettings();

    if (basicSettings) {
        hr = mediaType->SetUINT32(MF_MT_AVG_BITRATE, basicSettings->bitrate);
        H::System::ThrowIfFailed(hr);

        hr = MFSetAttributeSize(mediaType.Get(), MF_MT_FRAME_SIZE, basicSettings->width, basicSettings->height);
        H::System::ThrowIfFailed(hr);

        hr = MFSetAttributeRatio(mediaType.Get(), MF_MT_FRAME_RATE, basicSettings->fps, 1);
        H::System::ThrowIfFailed(hr);
    }

    MF::Helpers::PrintVideoMediaTypeInfo(mediaType, "[videoTypeOut]");
    return mediaType;
}

Microsoft::WRL::ComPtr<IMFMediaType> MediaRecorder::GetBestOutputType(
    IMFTransform *transform,
    IMFMediaType *defaultType,
    const GUID &subtypeGuid,
    uint32_t numChannels,
    uint32_t sampleRate,
    uint32_t bitsPerSample,
    uint32_t bitrate)
{
    LOG_FUNCTION_SCOPE_VERBOSE("GetBestOutputType()");

    Microsoft::WRL::ComPtr<IMFMediaType> outType;
    uint32_t outTypeIdx = 0;
    uint32_t outTypeBitrate = 0;
    HRESULT hr = S_OK;

    while (true) {
        Microsoft::WRL::ComPtr<IMFMediaType> curType;

        hr = transform->GetOutputAvailableType(0, outTypeIdx, curType.GetAddressOf());
        outTypeIdx++;
        if (hr == MF_E_NO_MORE_TYPES) {
            break;
        }
        if (hr != S_OK) {
            continue;
        }

        GUID curSubtype;

        hr = curType->GetGUID(MF_MT_SUBTYPE, &curSubtype);
        if (hr != S_OK || curSubtype != subtypeGuid) {
            continue;
        }

        uint32_t curChannels = 0, curSampleRate = 0;

        hr = curType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &curChannels);
        if (hr != S_OK) {
            continue;
        }

        hr = curType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &curSampleRate);
        if (hr != S_OK) {
            continue;
        }

        if (curChannels != numChannels || curSampleRate != sampleRate) {
            continue;
        }

        uint32_t curBitsPerSample = 0;

        hr = curType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &curBitsPerSample);
        if (hr == S_OK && !outType && curBitsPerSample == bitsPerSample) {
            outType = curType;
        }

        uint32_t curBitrate = 0;

        hr = curType->GetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, &curBitrate);
        if (hr == S_OK) {
            curBitrate *= 8;

            int outBitrateDist = (int)outTypeBitrate - (int)bitrate;
            int curBitrateDist = (int)curBitrate - (int)bitrate;

            if ((curBitrate > outTypeBitrate && curBitrate <= bitrate) ||                           // set max from [min ... bitrate]
                (curBitrate > bitrate && (curBitrateDist < outBitrateDist || outBitrateDist < 0))   // set min or first from [bitrate ... max]
                )
            {
                outTypeBitrate = curBitrate;
                outType = curType;
            }
        }
    }

    if (!outType) {
#if SPDLOG_ENABLED
        LOG_DEBUG("MediaRecorder::GetBestOutputType can't find best output type, using default");
#endif
        outType = defaultType;
    }

    MF::Helpers::PrintAudioMediaTypeInfo(outType, "[audioBestTypeOut]");
    return outType;
}

void MediaRecorder::WriteVideoSample(const Microsoft::WRL::ComPtr<IMFMediaBuffer> &buffer, int64_t hns, int64_t durationHns) {
    if (hns >= 0) {
        this->videoPtsHns = hns;
    }

    if (durationHns < 0) {
        durationHns = this->GetDefaultVideoFrameDuration();
    }

    this->WriteSample(buffer, this->videoPtsHns, durationHns, this->videoStreamIdx);

    this->videoPtsHns += durationHns;
}

void MediaRecorder::WriteSample(const Microsoft::WRL::ComPtr<IMFMediaBuffer> &buffer, int64_t positionHNS, int64_t durationHNS, DWORD streamIndex) {
    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFSample> sample;

    hr = MFCreateSample(sample.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = sample->AddBuffer(buffer.Get());
    H::System::ThrowIfFailed(hr);

    hr = sample->SetSampleTime(positionHNS);
    H::System::ThrowIfFailed(hr);

    hr = sample->SetSampleDuration(durationHNS);
    H::System::ThrowIfFailed(hr);

    this->WriteSample(sample, streamIndex);
}

void MediaRecorder::WriteSample(const Microsoft::WRL::ComPtr<IMFSample> &sample, DWORD streamIndex) {
    try {
        HRESULT hr = S_OK;

        LONGLONG samplePts;
        hr = sample->GetSampleTime(&samplePts);
        H::System::ThrowIfFailed(hr);

        LONGLONG sampleDuration;
        hr = sample->GetSampleDuration(&sampleDuration);
        H::System::ThrowIfFailed(hr);

        if (streamIndex == this->videoStreamIdx) {
            //LOG_DEBUG_D("WriteSample VIDEO: samplePts = {}, sampleDuration = {}  (videoFrameNumber = {})", samplePts, sampleDuration, framesNumber);
            lastWritedVideoSample.emplace(HH::Chrono::Hns{ samplePts }, HH::Chrono::Hns{ sampleDuration });
        }
        else {
            //LOG_DEBUG_D("WriteSample AUDIO: samplePts = {}, sampleDuration = {}  (audioSampleNumber = {})", samplePts, sampleDuration, samplesNumber);
            lastWritedAudioSample.emplace(HH::Chrono::Hns{ samplePts }, HH::Chrono::Hns{ sampleDuration });
        }

        hr = this->sinkWriter->WriteSample(streamIndex, sample.Get());
        H::System::ThrowIfFailed(hr);
    }
    catch (const HResultException& ex) {
        if (ex.GetHRESULT() == HRESULT_FROM_WIN32(ERROR_DISK_FULL)) {
            if (recordEventCallback) {
                Native::MediaRecorderEventArgs eventArgs;

                if (params.UseChunkMerger) {
                    eventArgs.message = Native::MediaRecorderMessageEnum::NotEnoughSpaceOnDiskWithChunks;
                }
                else {
                    eventArgs.message = Native::MediaRecorderMessageEnum::NotEnoughSpaceOnTargetRecordPath;
                }

                recordEventCallback->call(eventArgs);
                this->recordingErrorOccured = true;
            }
            else {
                throw;
            }
        }
        else {
            throw;
        }
    }
}

int64_t MediaRecorder::GetDefaultVideoFrameDuration() const {
    assert(this->params.mediaFormat.GetVideoCodecSettings());
    assert(this->params.mediaFormat.GetVideoCodecSettings()->GetBasicSettings());

    auto basicSettings = this->params.mediaFormat.GetVideoCodecSettings()->GetBasicSettings();
    int64_t hns = static_cast<int64_t>(H::Time::HNSResolution / basicSettings->fps);
    return hns;
}

IMFByteStream* MediaRecorder::StartNewChunk() {
#if HAVE_WINRT
    H::System::ThrowIfFailed(E_NOTIMPL);
    return nullptr;
#else
    if (!params.UseChunkMerger) {
        assert(false);
        return nullptr;
    }

    chunkFile = GetChunkFilePath(chunkNumber++);

    auto hr = MFCreateFile(MF_ACCESSMODE_READWRITE, MF_OPENMODE_DELETE_IF_EXIST, MF_FILEFLAGS_NONE, chunkFile.c_str(), currentOutputStream.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    return currentOutputStream.Get();
#endif
}

void MediaRecorder::FinalizeRecord(bool useRecordEventCallback) {
    HRESULT hr = S_OK;
    framesNumber = 0;
    samplesNumber = 0;
    chunkAudioPtsHns = 0;
    chunkVideoPtsHns = 0;
#if SPDLOG_ENABLED
    LOG_DEBUG("MediaRecorder::FinalizeRecord started");
#endif
    hr = this->sinkWriter->Finalize();
    if (hr != MF_E_SINK_NO_SAMPLES_PROCESSED) { // occured when was called BeginWritting but not calls WriteSample yet
#if SPDLOG_ENABLED
        LOG_DEBUG("MediaRecorder::FinalizeRecord error occured during finalization");
#endif
        H::System::ThrowIfFailed(hr);
    }

    this->currentOutputStream.Reset();
    this->sinkWriter.Reset();

    if (useRecordEventCallback && params.UseChunkMerger && recordEventCallback) {
        Native::MediaRecorderEventArgs eventArgs;

        auto lastChunkSize = H::HardDrive::GetFilesize(chunkFile);
        recordedChunksSize += lastChunkSize;
        auto freeSpaceTragetDisk = H::HardDrive::GetFreeMemory(targetRecordDisk);
        auto freeSpaceAfterWriteChunks = freeSpaceTragetDisk - recordedChunksSize;

        if (recordedChunksSize >= freeSpaceTragetDisk) {
            eventArgs.message = Native::MediaRecorderMessageEnum::NotEnoughSpaceOnTargetRecordPath;
            recordEventCallback->call(eventArgs);
            return;
        }

        auto dtSecCreatedChunk = (H::Time::GetCurrentLibTime() - this->lastChunkCreatedTime) / H::Time::HNSResolution;
        int remainingTime = (int)((float)freeSpaceAfterWriteChunks / lastChunkSize * dtSecCreatedChunk);

        eventArgs.message = Native::MediaRecorderMessageEnum::RemainingTime;
        eventArgs.remainingTime = remainingTime;
        recordEventCallback->call(eventArgs);
    }
#if SPDLOG_ENABLED
    LOG_DEBUG("MediaRecorder::FinalizeRecord done");
#endif
}

void MediaRecorder::MergeChunks(IMFByteStream* outputStream, std::vector<std::wstring>&& chunks)
{
    ChunkMerger merger{
        outputStream,
        MediaRecorder::CreateAudioInMediaType(this->params.mediaFormat.GetAudioCodecSettings(), AudioSampleBits),
        MediaRecorder::CreateAudioOutMediaType(this->params.mediaFormat.GetAudioCodecSettings(), AudioSampleBits),
        MediaRecorder::CreateVideoInMediaType(this->params.mediaFormat.GetVideoCodecSettings(), nv12VideoSamples),
        MediaRecorder::CreateVideoOutMediaType(this->params.mediaFormat.GetVideoCodecSettings(), this->params.mediaFormat.GetMediaContainerType(), params.UseChunkMerger),
        this->params.mediaFormat.GetVideoCodecSettings(),
        std::move(chunks),
        containerExt,
        this->params.mediaFormat.GetMediaContainerType(),
        this->params.chunkMergerTryRemux
    };

    merger.Merge();
}

void MediaRecorder::ResetSinkWriterOnNewChunk() {
    if (!params.UseChunkMerger) {
        assert(false);
        return;
    }

    FinalizeRecord(true);
    auto newStream = StartNewChunk();
    lastChunkCreatedTime = H::Time::GetCurrentLibTime();
    this->InitializeSinkWriter(newStream, hardwareTransformsForEncoding, nv12VideoSamples);
    StartRecord();
}

constexpr DWORD MediaRecorder::GetInvalidStreamIdx() {
    return (std::numeric_limits<DWORD>::max)();
}
