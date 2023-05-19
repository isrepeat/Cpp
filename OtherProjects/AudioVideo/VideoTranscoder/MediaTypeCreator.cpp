#include "MediaTypeCreator.h"
#include "MediaFormatCodecsSupport.h"
#include <Helpers/System.h>


Microsoft::WRL::ComPtr<IMFMediaType> MediaTypeCreator::CreateAudioInMediaType(
    const IAudioCodecSettings* settings,
    uint32_t bitsPerSample)
{
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

    if (settings->HasBasicSettings()) {
        auto tmp = settings->GetBasicSettings();
        auto numChannels = tmp->numChannels;
        auto sampleRate = tmp->sampleRate;

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

    return mediaType;
}


Microsoft::WRL::ComPtr<IMFMediaType> MediaTypeCreator::CreateAudioOutMediaType(
    const IAudioCodecSettings* settings,
    uint32_t bitsPerSample)
{
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

    if (codecType == AudioCodecType::AMR_NB) {
        int stop = 423;
    }

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
    //case AudioCodecType::MP3: {
    //    if (!basicSettings || !bitrateSettings) {
    //        break;
    //    }

    //    auto factory = PlatformClassFactory::Instance()->CreateMP3CodecFactory();
    //    auto transform = factory->CreateIMFTransform();

    //    mediaType = MediaRecorder::GetBestOutputType(
    //        transform.Get(), mediaType.Get(), acodecId,
    //        basicSettings->numChannels, basicSettings->sampleRate,
    //        bitsPerSample, bitrateSettings->bitrate);
    //    break;
    //}
    //case AudioCodecType::WMAudioV8: {
    //    if (!basicSettings || !bitrateSettings) {
    //        break;
    //    }

    //    auto factory = PlatformClassFactory::Instance()->CreateWma8CodecFactory();
    //    auto transform = factory->CreateIMFTransform();

    //    mediaType = MediaRecorder::GetBestOutputType(
    //        transform.Get(), mediaType.Get(), acodecId,
    //        basicSettings->numChannels, basicSettings->sampleRate,
    //        bitsPerSample, bitrateSettings->bitrate);
    //    break;
    //}
    //case AudioCodecType::AMR_NB: {
    //    if (!basicSettings || !bitrateSettings) {
    //        break;
    //    }

    //    auto factory = PlatformClassFactory::Instance()->CreateAmrNbCodecFactory();
    //    auto transform = factory->CreateIMFTransform();

    //    if (transform) {
    //        mediaType = MediaRecorder::GetBestOutputType(
    //            transform.Get(), mediaType.Get(), acodecId,
    //            basicSettings->numChannels, basicSettings->sampleRate,
    //            bitsPerSample, bitrateSettings->bitrate);
    //    }

    //    break;
    //}
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

    return mediaType;
}


Microsoft::WRL::ComPtr<IMFMediaType> MediaTypeCreator::CreateVideoInMediaType(
    const IVideoCodecSettings* settings, bool nv12VideoSamples)
{
    if (settings == nullptr)
        return nullptr;

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFMediaType> mediaType;

    hr = MFCreateMediaType(mediaType.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    H::System::ThrowIfFailed(hr);

    if (nv12VideoSamples)
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

    return mediaType;
}


Microsoft::WRL::ComPtr<IMFMediaType> MediaTypeCreator::CreateVideoOutMediaType(
    const IVideoCodecSettings* settings)
{
    if (settings == nullptr)
        return nullptr;

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFMediaType> mediaType;

    hr = MFCreateMediaType(mediaType.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    H::System::ThrowIfFailed(hr);

    auto codecSup = MediaFormatCodecsSupport::Instance();
    auto vcodecId = codecSup->MapVideoCodec(settings->GetCodecType());

    hr = mediaType->SetGUID(MF_MT_SUBTYPE, vcodecId);
    H::System::ThrowIfFailed(hr);

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

    return mediaType;
}