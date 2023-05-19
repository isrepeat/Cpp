#include "MediaFormatFactory.h"
#include "AudioCodecLosslessSettings.h"
#include "AudioCodecCompressedSettings.h"
#include "VideoCodecSimpleSettings.h"
#include "MediaFormatCodecsSupport.h"
//#include "Platform/PlatformClassFactory.h"

#include <algorithm>

MediaFormatFactory::MediaFormatFactory() {
    // encoder restrictions can be found here : https://msdn.microsoft.com/en-us/library/windows/desktop/dd742785(v=vs.85).aspx
    this->aacSettingsValues.NumChannels = { 1, 2 };
    this->aacSettingsValues.SampleRate = { 44100, 48000 };
    this->aacSettingsValues.Bitrate = { 12000 * 8, 16000 * 8, 20000 * 8, 24000 * 8 };

    this->mp3SettingsValues.NumChannels = this->aacSettingsValues.NumChannels;
    this->mp3SettingsValues.SampleRate = this->aacSettingsValues.SampleRate;
    this->mp3SettingsValues.Bitrate = { 12000 * 8, 16000 * 8, 20000 * 8, 24000 * 8, 40000 * 8 };

    this->amrNbSettingsValues.NumChannels = { 1 };
    this->amrNbSettingsValues.SampleRate = { 8000 };
    this->amrNbSettingsValues.Bitrate = { 1525 * 8 };

    // no bitrate settings
    this->losslessSettingsValues.NumChannels = this->aacSettingsValues.NumChannels;
    this->losslessSettingsValues.SampleRate = this->aacSettingsValues.SampleRate;
}

MediaFormat MediaFormatFactory::CreateMediaFormat(MediaContainerType container, AudioCodecType audioCodec) {
    return this->CreateMediaFormat(container, audioCodec, VideoCodecType::Unknown);
}

MediaFormat MediaFormatFactory::CreateMediaFormat(MediaContainerType container, VideoCodecType videoCodec) {
    return this->CreateMediaFormat(container, AudioCodecType::Unknown, videoCodec);
}

MediaFormat MediaFormatFactory::CreateMediaFormat(
    MediaContainerType container,
    AudioCodecType audioCodec,
    VideoCodecType videoCodec)
{
    this->CheckCodecs(container, audioCodec, videoCodec);

    std::unique_ptr<IAudioCodecSettings> audioSettings;
    std::unique_ptr<IVideoCodecSettings> videoSettings;

    if (audioCodec != AudioCodecType::Unknown) {
        audioSettings = this->CreateAudioCodecSettings(audioCodec);
    }

    if (videoCodec != VideoCodecType::Unknown) {
        videoSettings = this->CreateVideoCodecSettings(videoCodec);
    }

    return MediaFormat(container, std::move(audioSettings), std::move(videoSettings));
}

//AudioCodecSettingsValues MediaFormatFactory::GetSettingsValues(AudioCodecType codec) const {
//    AudioCodecSettingsValues settings;
//
//    Microsoft::WRL::ComPtr<IMFTransform> transform;
//    Microsoft::WRL::ComPtr<IMFMediaType> type;
//    Microsoft::WRL::ComPtr<IMFSinkWriter> writer;
//    uint32_t typeIdx = 0;
//    HRESULT hr;
//
//    switch (codec) {
//    case AudioCodecType::AAC:
//        transform = PlatformClassFactory::Instance()->CreateAacCodecFactory()->CreateIMFTransform();
//        break;
//    case AudioCodecType::MP3:
//        transform = PlatformClassFactory::Instance()->CreateMP3CodecFactory()->CreateIMFTransform();
//        break;
//    case AudioCodecType::DolbyAC3:
//        transform = PlatformClassFactory::Instance()->CreateDolbyAC3CodecFactory()->CreateIMFTransform();
//        break;
//    case AudioCodecType::WMAudioV8:
//        transform = PlatformClassFactory::Instance()->CreateWma8CodecFactory()->CreateIMFTransform();
//        break;
//    case AudioCodecType::AMR_NB:
//        transform = PlatformClassFactory::Instance()->CreateAmrNbCodecFactory()->CreateIMFTransform();
//        break;
//    case AudioCodecType::FLAC:
//        transform = PlatformClassFactory::Instance()->CreateFlacCodecFactory()->CreateIMFTransform();
//        break;
//    case AudioCodecType::ALAC:
//        transform = PlatformClassFactory::Instance()->CreateAlacCodecFactory()->CreateIMFTransform();
//        break;
//    case AudioCodecType::PCM:
//        return this->losslessSettingsValues;
//    default:
//        return this->unknownAudioSettings;
//    }
//
//    if (!transform) {
//        throw std::exception();
//    }
//
//    auto codecSupport = MediaFormatCodecsSupport::Instance();
//    auto subtypeGuid = codecSupport->MapAudioCodec(codec);
//
//    while (true) {
//        hr = transform->GetOutputAvailableType(0, typeIdx, type.GetAddressOf());
//        typeIdx++;
//
//        if (hr == MF_E_NO_MORE_TYPES) {
//            break;
//        }
//        if (hr != S_OK) {
//            continue;
//        }
//
//        uint32_t channels;
//        uint32_t bitRate;
//        uint32_t sampleRate;
//        GUID curSubtype;
//
//        hr = type->GetGUID(MF_MT_SUBTYPE, &curSubtype);
//        if (hr != S_OK || curSubtype != subtypeGuid) {
//            continue;
//        }
//
//        type->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels);
//        type->GetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, &bitRate);
//        type->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate);
//
//        settings.NumChannels.push_back(channels);
//        settings.Bitrate.push_back(bitRate * 8);
//        settings.SampleRate.push_back(sampleRate);
//    }
//
//    return settings;
//}

std::unique_ptr<IAudioCodecSettings> MediaFormatFactory::CreateAudioCodecSettings(AudioCodecType codec) const {
    switch (codec) {
    case AudioCodecType::AAC:
    case AudioCodecType::MP3:
    case AudioCodecType::DolbyAC3:
    case AudioCodecType::WMAudioV8:
    case AudioCodecType::AMR_NB:
        return std::make_unique<AudioCodecCompressedSettings>(codec);
    case AudioCodecType::FLAC:
    case AudioCodecType::ALAC:
    case AudioCodecType::PCM:
        return std::make_unique<AudioCodecLosslessSettings>(codec);
    default:
        break;
    }

    return nullptr;
}

std::unique_ptr<IVideoCodecSettings> MediaFormatFactory::CreateVideoCodecSettings(VideoCodecType codec) const {
    return std::make_unique<VideoCodecSimpleSettings>(codec);
}

void MediaFormatFactory::CheckCodecs(
    MediaContainerType container,
    AudioCodecType audioCodec,
    VideoCodecType videoCodec)
{
    auto codecSupport = MediaFormatCodecsSupport::Instance();
    auto &availableCodecs = codecSupport->GetCodecsSupport(container);

    if (audioCodec == AudioCodecType::Unknown && videoCodec == VideoCodecType::Unknown) {
        throw std::exception("No codec has been selected");
    }

    if (audioCodec != AudioCodecType::Unknown) {
        auto &acodecs = availableCodecs.GetAudioCodecs();

        auto find = std::find(acodecs.begin(), acodecs.end(), audioCodec);

        if (find == acodecs.end()) {
            throw std::exception("Audio codec is not supported by this container");
        }
    }

    if (videoCodec != VideoCodecType::Unknown) {
        auto &vcodecs = availableCodecs.GetVideoCodecs();

        auto find = std::find(vcodecs.begin(), vcodecs.end(), videoCodec);

        if (find == vcodecs.end()) {
            throw std::exception("Video codec is not supported by this container");
        }
    }
}