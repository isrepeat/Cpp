#include "EncoderEnumerator.h"
#include "PlatformHelpers.h"
#include "MediaRecorder.h"
#include "MediaFormat/MediaFormatCodecsSupport.h"
#include "MediaFormat/MediaFormatFactory.h"

#include <algorithm>
#include <libhelpers/as_optional.h>
#include <libhelpers/HSystem.h>

class EncoderEnumeratorVideoSettings : public IVideoCodecSettings {
public:
    EncoderEnumeratorVideoSettings(VideoCodecType codecType)
        : IVideoCodecSettings(codecType)
    {
        this->basicSettings.width = 1280;
        this->basicSettings.height = 720;
        this->basicSettings.fps = 30;
        this->basicSettings.bitrate = 1024 * 1024;
    }

    const VideoCodecBasicSettings* GetBasicSettings() const override {
        return &this->basicSettings;
    }

    void SetBasicSettings(const VideoCodecBasicSettings &v) override {
        this->basicSettings = v;
    }

    std::unique_ptr<IVideoCodecSettings> Clone() override {
        return std::make_unique<EncoderEnumeratorVideoSettings>(*this);
    }

    std::shared_ptr<IVideoCodecSettings> CloneShared() override {
        return std::make_shared<EncoderEnumeratorVideoSettings>(*this);
    }

private:
    VideoCodecBasicSettings basicSettings;
};

void EncoderEnumerator::EnumAudio(std::function<void(const GUID &)> fn) {
    auto codecSupport = MediaFormatCodecsSupport::Instance();

    MediaFormatFactory mediaFormatFactory;

    for (uint32_t i = 0; i < (uint32_t)AudioCodecType::Count; i++) {
        auto codecType = (AudioCodecType)(i + (uint32_t)AudioCodecType::First);

        if (codecType == AudioCodecType::PCM || codecType == AudioCodecType::ALAC) {
            continue; // MFTEnum doesn't report this codec but MediaFoundation supports it
        }

        for (uint32_t j = 0; j < (uint32_t)MediaContainerType::Count; j++) {
            auto containerType = (MediaContainerType)(j + (uint32_t)MediaContainerType::First);
            auto &support = codecSupport->GetCodecsSupport(containerType);
            auto &codecs = support.GetAudioCodecs();

            auto find = std::find(codecs.begin(), codecs.end(), codecType);
            if (find != codecs.end()) {
                try {
                    auto stream = PlatformHelpers::CreateMemoryStream();
                    MediaRecorderParams params;

                    params.mediaFormat = mediaFormatFactory.CreateMediaFormat(containerType, codecType);

                    auto settingVals = mediaFormatFactory.GetSettingsValues(codecType);
                    auto audioSettings = params.mediaFormat.GetAudioCodecSettings();

                    if (settingVals.empty()) {
                        // may be logic error in GetSettingsValues
                        assert(false);
                        continue;
                    }

                    if (auto settings = audioSettings->GetBasicSettings(); settings && settingVals[0].basicSettings) {
                        audioSettings->SetBasicSettings(*settingVals[0].basicSettings);
                    }
                    else {
                        // all audio codecs must have basic settings
                        assert(false);
                        continue;
                    }

                    if (auto settings = audioSettings->GetBitrateSettings(); settings && settingVals[0].bitrateSettings) {
                        audioSettings->SetBitrateSettings(*settingVals[0].bitrateSettings);
                    }
                    else {
                        if (settings && !settingVals[0].bitrateSettings) {
                            // check why mediaFormat.GetAudioCodecSettings() have bitrate and GetSettingsValues does not have
                            assert(false);
                            continue;
                        }
                    }

                    // try to create recorder in order to check codec support
                    MediaRecorder recorder(stream.Get(), std::move(params), UseHardwareTransformsForEncoding{ true }, UseNv12VideoSamples{ true });

                    auto codecGuid = codecSupport->MapAudioCodec(codecType);

                    if (codecGuid == GUID_NULL) {
                        continue;
                    }

                    fn(codecGuid);
                    break;
                }
                catch (...) {
                    continue;
                }
            }
        }
    }
}

void EncoderEnumerator::EnumVideo(std::function<void(const GUID &)> fn) {
    auto codecSupport = MediaFormatCodecsSupport::Instance();

    for (uint32_t i = 0; i < (uint32_t)VideoCodecType::Count; i++) {
        auto codecType = (VideoCodecType)(i + (uint32_t)VideoCodecType::First);

        for (uint32_t j = 0; j < (uint32_t)MediaContainerType::Count; j++) {
            auto containerType = (MediaContainerType)(j + (uint32_t)MediaContainerType::First);
            auto &support = codecSupport->GetCodecsSupport(containerType);
            auto &codecs = support.GetVideoCodecs();

            auto find = std::find(codecs.begin(), codecs.end(), codecType);
            if (find != codecs.end()) {
                try {
                    auto stream = PlatformHelpers::CreateMemoryStream();
                    MediaRecorderParams params;

                    params.mediaFormat = MediaFormat(containerType, nullptr, std::make_unique<EncoderEnumeratorVideoSettings>(codecType));

                    // try to create recorder in order to check codec support
                    MediaRecorder recorder(stream.Get(), std::move(params), UseHardwareTransformsForEncoding{ true }, UseNv12VideoSamples{ true });

                    auto codecGuid = codecSupport->MapVideoCodec(codecType);

                    if (codecGuid == GUID_NULL) {
                        continue;
                    }

                    fn(codecGuid);
                    break;
                }
                catch (...) {
                    continue;
                }
            }
        }
    }
}
