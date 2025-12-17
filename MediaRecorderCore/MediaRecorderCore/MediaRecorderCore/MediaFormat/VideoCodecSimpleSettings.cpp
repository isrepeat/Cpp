#include "pch.h"
#include "VideoCodecSimpleSettings.h"

VideoCodecSimpleSettings::VideoCodecSimpleSettings(VideoCodecType codecType)
    : IVideoCodecSettings(codecType)
{}

const VideoCodecBasicSettings *VideoCodecSimpleSettings::GetBasicSettings() const {
    return &this->basicSettings;
}

void VideoCodecSimpleSettings::SetBasicSettings(const VideoCodecBasicSettings &v) {
    this->basicSettings = v;
}

std::unique_ptr<IVideoCodecSettings> VideoCodecSimpleSettings::Clone() {
    return std::make_unique<VideoCodecSimpleSettings>(*this);
}

std::shared_ptr<IVideoCodecSettings> VideoCodecSimpleSettings::CloneShared() {
    return std::make_shared<VideoCodecSimpleSettings>(*this);
}