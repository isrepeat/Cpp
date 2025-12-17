#include "pch.h"
#include "MediaFormatFactory.h"
#include "AudioCodecLosslessSettings.h"
#include "AudioCodecCompressedSettings.h"
#include "VideoCodecSimpleSettings.h"
#include "MediaFormatCodecsSupport.h"
#include "Platform/PlatformClassFactory.h"
#include "AudioCodecSettingsItemSort\AudioCodecSettingsItemCmp.h"

#include <cassert>
#include <algorithm>
#include <libhelpers\as_optional.h>

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

AudioCodecSettingsValues MediaFormatFactory::GetSettingsValues(AudioCodecType codec) const {
    Microsoft::WRL::ComPtr<IMFTransform> transform;
    Microsoft::WRL::ComPtr<IMFMediaType> type;
    Microsoft::WRL::ComPtr<IMFSinkWriter> writer;
    uint32_t typeIdx = 0;
    HRESULT hr;

    switch (codec) {
    case AudioCodecType::AAC:
        transform = PlatformClassFactory::Instance()->CreateAacCodecFactory()->CreateIMFTransform();
        break;
    case AudioCodecType::MP3:
        transform = PlatformClassFactory::Instance()->CreateMP3CodecFactory()->CreateIMFTransform();
        break;
    case AudioCodecType::DolbyAC3:
        transform = PlatformClassFactory::Instance()->CreateDolbyAC3CodecFactory()->CreateIMFTransform();
        break;
    case AudioCodecType::WMAudioV8:
        transform = PlatformClassFactory::Instance()->CreateWma8CodecFactory()->CreateIMFTransform();
        break;
    case AudioCodecType::AMR_NB:
        transform = PlatformClassFactory::Instance()->CreateAmrNbCodecFactory()->CreateIMFTransform();
        break;
    case AudioCodecType::FLAC:
        transform = PlatformClassFactory::Instance()->CreateFlacCodecFactory()->CreateIMFTransform();
        break;
    case AudioCodecType::ALAC:
        transform = PlatformClassFactory::Instance()->CreateAlacCodecFactory()->CreateIMFTransform();
        break;
    case AudioCodecType::PCM:
        return this->pcmSettingsValues;
    default:
        return {};
    }

    if (!transform) {
        throw std::exception("Can't create MFTransform");
    }

    AudioCodecSettingsValues settings;
    auto codecSupport = MediaFormatCodecsSupport::Instance();
    auto subtypeGuid = codecSupport->MapAudioCodec(codec);

    while (true) {
        hr = transform->GetOutputAvailableType(0, typeIdx, &type);
        typeIdx++;

        if (hr == MF_E_NO_MORE_TYPES) {
            break;
        }
        if (hr != S_OK) {
            continue;
        }

        if (!type) {
            // check logic and hr result handling
            assert(false);
            throw std::exception("Can't get output available type");
        }

        GUID curSubtype = GUID_NULL;

        hr = type->GetGUID(MF_MT_SUBTYPE, &curSubtype);
        if (hr != S_OK || curSubtype != subtypeGuid) {
            continue;
        }

        AudioCodecSettingsItem item;

        item.basicSettings = MediaFormatFactory::GetAudioCodecBasicSettings(*type.Get());
        item.bitrateSettings = MediaFormatFactory::GetAudioCodecBitrateSettings(*type.Get());

        if (!item.basicSettings) {
            // basicSettings must be present for all IMFMediaType
            assert(false);
            hr = E_FAIL;
        }

        if (hr == S_OK) {
            settings.push_back(std::move(item));
        }
        else {
            // check logic
            assert(false);
        }
    }

    return settings;
}

AudioCodecSettingsValues MediaFormatFactory::GetClosestSettingsValues(const IAudioCodecSettings& audioCodecSettings) const {
    auto values = this->GetSettingsValues(audioCodecSettings.GetCodecType());

    AudioCodecSettingsItem settingsItem;

    settingsItem.basicSettings = as_optional(audioCodecSettings.GetBasicSettings());
    if (!settingsItem.basicSettings) {
        // check logic
        assert(false);
        return values;
    }

    settingsItem.bitrateSettings = as_optional(audioCodecSettings.GetBitrateSettings());

    std::sort(values.begin(), values.end(), AudioCodecSettingsItemCmp::Make(settingsItem));

    return values;
}

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

std::optional<AudioCodecBasicSettings> MediaFormatFactory::GetAudioCodecBasicSettings(IMFMediaType& type) {
    HRESULT hr = S_OK;
    AudioCodecBasicSettings basicSettings;

    hr = type.GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &basicSettings.numChannels);
    if (FAILED(hr)) {
        return {};
    }

    hr = type.GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &basicSettings.sampleRate);
    if (FAILED(hr)) {
        return {};
    }

    return basicSettings;
}

std::optional<AudioCodecBitrateSettings> MediaFormatFactory::GetAudioCodecBitrateSettings(IMFMediaType& type) {
    HRESULT hr = S_OK;
    AudioCodecBitrateSettings bitrateSettings;

    hr = type.GetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, &bitrateSettings.bitrate);
    if (FAILED(hr)) {
        return {};
    }

    // MF_MT_AUDIO_AVG_BYTES_PER_SECOND - byte rate, convert to bits by * 8
    bitrateSettings.bitrate *= 8;

    return bitrateSettings;
}

AudioCodecSettingsValues MediaFormatFactory::CreatePcmSettingsValues() {
    // encoder restrictions can be found here : https://msdn.microsoft.com/en-us/library/windows/desktop/dd742785(v=vs.85).aspx
    AudioCodecSettingsValues pcmSettingsValues;

    for (uint32_t numChannels : { 1, 2 }) {
        for (uint32_t sampleRate : { 44100, 48000 }) {
            AudioCodecSettingsItem item;

            item.basicSettings.emplace();
            item.basicSettings->numChannels = numChannels;
            item.basicSettings->sampleRate = sampleRate;

            // no bitrate settings
            // pcm is lossless, it doesn't have bitrate settings

            pcmSettingsValues.push_back(std::move(item));
        }
    }

    return pcmSettingsValues;
}
