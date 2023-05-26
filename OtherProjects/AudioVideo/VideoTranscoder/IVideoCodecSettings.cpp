#include "IVideoCodecSettings.h"

IVideoCodecSettings::IVideoCodecSettings(VideoCodecType codecType)
    : codecType(codecType)
{}

IVideoCodecSettings::~IVideoCodecSettings()
{}

bool IVideoCodecSettings::HasBasicSettings() const {
    return this->GetBasicSettings() != nullptr;
}

VideoCodecType IVideoCodecSettings::GetCodecType() const {
    return this->codecType;
}