#include "pch.h"
#include "IVideoCodecSettings.h"

IVideoCodecSettings::IVideoCodecSettings(VideoCodecType codecType)
    : codecType(codecType)
{}

VideoCodecType IVideoCodecSettings::GetCodecType() const {
    return this->codecType;
}
