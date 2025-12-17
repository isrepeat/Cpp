#include "pch.h"
#include "MediaFormat.h"
#include "MediaContainerExtension.h"

MediaFormat::MediaFormat(
    MediaContainerType containerType,
    std::unique_ptr<IAudioCodecSettings> audioCodecSettings,
    std::unique_ptr<IVideoCodecSettings> videoCodecSettings)
    : containerType(containerType)
    , audioCodecSettings(std::move(audioCodecSettings))
    , videoCodecSettings(std::move(videoCodecSettings))
{}

MediaFormat::MediaFormat(const MediaFormat &other)
    : containerType(other.containerType)
    , audioCodecSettings(other.audioCodecSettings ? other.audioCodecSettings->Clone() : nullptr)
    , videoCodecSettings(other.videoCodecSettings ? other.videoCodecSettings->Clone() : nullptr)
{}

MediaFormat::MediaFormat(MediaFormat &&other)
    : MediaFormat()
{
    using std::swap;
    swap(*this, other);
}

MediaFormat &MediaFormat::operator=(MediaFormat other) {
    using std::swap;
    swap(*this, other);
    return *this;
}

MediaContainerType MediaFormat::GetMediaContainerType() const {
    return this->containerType;
}

const std::wstring& MediaFormat::GetMediaContainerFileExtension() const {
    return MediaContainerExtension::Instance().GetFileExtension(this->containerType);
}

IAudioCodecSettings *MediaFormat::GetAudioCodecSettings() {
    return this->audioCodecSettings.get();
}

IVideoCodecSettings *MediaFormat::GetVideoCodecSettings() {
    return this->videoCodecSettings.get();
}

const IAudioCodecSettings *MediaFormat::GetAudioCodecSettings() const {
    return this->audioCodecSettings.get();
}

const IVideoCodecSettings *MediaFormat::GetVideoCodecSettings() const {
    return this->videoCodecSettings.get();
}

void swap(MediaFormat &a, MediaFormat &b) noexcept {
    using std::swap;
    swap(a.containerType, b.containerType);
    swap(a.audioCodecSettings, b.audioCodecSettings);
    swap(a.videoCodecSettings, b.videoCodecSettings);
}
