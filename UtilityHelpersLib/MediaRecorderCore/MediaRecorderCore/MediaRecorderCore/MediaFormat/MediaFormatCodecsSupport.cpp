#include "pch.h"
#include "MediaFormatCodecsSupport.h"

#include <libhelpers/MediaFoundation/MFInclude.h>

#include <algorithm>

#ifndef _WIN32_WINNT_WINTHRESHOLD
#include <initguid.h>
#define  WAVE_FORMAT_FLAC                       0xF1AC /* flac.sourceforge.net */
#define  WAVE_FORMAT_ALAC                       0x6C61 /* Apple Lossless */

DEFINE_MEDIATYPE_GUID(MFAudioFormat_FLAC, WAVE_FORMAT_FLAC);
DEFINE_MEDIATYPE_GUID(MFAudioFormat_ALAC, WAVE_FORMAT_ALAC);
#endif

const MediaFormatCodecs MediaFormatCodecsSupport::UnknownFmt;

MediaFormatCodecsSupport *MediaFormatCodecsSupport::Instance() {
    static MediaFormatCodecsSupport instance;
    return &instance;
}

const MediaFormatCodecs &MediaFormatCodecsSupport::GetCodecsSupport(MediaContainerType container) const {
    if (container == MediaContainerType::Unknown) {
        return MediaFormatCodecsSupport::UnknownFmt;
    }

    return this->codecsSupport[MediaFormatCodecsSupport::MediaContainerTypeToIdx(container)];
}

AudioCodecType MediaFormatCodecsSupport::MapAudioCodec(const GUID &codecGuid) const {
    auto find = std::find_if(this->audioCodecMap.begin(), this->audioCodecMap.end(),
        [&](const std::pair<GUID, AudioCodecType> &i)
    {
        return i.first == codecGuid;
    });

    if (find != this->audioCodecMap.end()) {
        return find->second;
    }

    return AudioCodecType::Unknown;
}

VideoCodecType MediaFormatCodecsSupport::MapVideoCodec(const GUID &codecGuid) const {
    auto find = std::find_if(this->videoCodecMap.begin(), this->videoCodecMap.end(),
        [&](const std::pair<GUID, VideoCodecType> &i)
    {
        return i.first == codecGuid;
    });

    if (find != this->videoCodecMap.end()) {
        return find->second;
    }

    return VideoCodecType::Unknown;
}

GUID MediaFormatCodecsSupport::MapAudioCodec(AudioCodecType type) const {
    auto find = std::find_if(this->audioCodecMap.begin(), this->audioCodecMap.end(),
        [&](const std::pair<GUID, AudioCodecType> &i)
    {
        return i.second == type;
    });

    if (find != this->audioCodecMap.end()) {
        return find->first;
    }

    return GUID_NULL;
}

GUID MediaFormatCodecsSupport::MapVideoCodec(VideoCodecType type) const {
    auto find = std::find_if(this->videoCodecMap.begin(), this->videoCodecMap.end(),
        [&](const std::pair<GUID, VideoCodecType> &i)
    {
        return i.second == type;
    });

    if (find != this->videoCodecMap.end()) {
        return find->first;
    }

    return GUID_NULL;
}

MediaFormatCodecsSupport::MediaFormatCodecsSupport() {
    /* Video containers */
    // H.264-supporting formats
    std::vector mp4AudioCodecs{
        AudioCodecType::AAC, AudioCodecType::MP3,
        AudioCodecType::DolbyAC3, AudioCodecType::ALAC,
        // AudioCodecType::FLAC, (doesn't work in VLC)
    };

    this->Add(MediaContainerType::MP4, mp4AudioCodecs, {
            VideoCodecType::H264, VideoCodecType::HEVC 
        });

    this->Add(MediaContainerType::M4V, mp4AudioCodecs, {VideoCodecType::H264});
    this->Add(MediaContainerType::MOV, mp4AudioCodecs, {VideoCodecType::H264});
    this->Add(MediaContainerType::ThreeGP, {
        AudioCodecType::AMR_NB, AudioCodecType::AAC
        }, {
            VideoCodecType::H264
        });

    // WMV / ASF
	std::vector<AudioCodecType> wmvAudioCodecs{
		AudioCodecType::WMAudioV8, AudioCodecType::AAC,
		AudioCodecType::DolbyAC3, AudioCodecType::PCM,
	};

	std::vector<VideoCodecType> wmvVideoCodecs{
		VideoCodecType::WMV3, VideoCodecType::WMV1
	};

    this->Add(MediaContainerType::WMV, wmvAudioCodecs, wmvVideoCodecs);
    this->Add(MediaContainerType::ASF, wmvAudioCodecs, wmvVideoCodecs);

    /* Audio containers */
    this->Add(MediaContainerType::MP3, {
        AudioCodecType::MP3,
        },
        {});

    this->Add(MediaContainerType::M4A, {
        AudioCodecType::AAC, AudioCodecType::DolbyAC3, AudioCodecType::ALAC,
        },
        {});

    this->Add(MediaContainerType::FLAC, {
        AudioCodecType::FLAC, 
        },
        {});

    this->Add(MediaContainerType::WMA, {
        AudioCodecType::WMAudioV8, 
        },
        {});

    this->Add(MediaContainerType::WAV, {
        AudioCodecType::PCM 
        },
        {});

    this->Add(MediaContainerType::ThreeGPP, {
        AudioCodecType::AMR_NB
        },
        {});

    this->audioCodecMap = {
        std::make_pair(MFAudioFormat_MP3, AudioCodecType::MP3),
        std::make_pair(MFAudioFormat_AAC, AudioCodecType::AAC),
        std::make_pair(MFAudioFormat_Dolby_AC3, AudioCodecType::DolbyAC3),
        std::make_pair(MFAudioFormat_WMAudioV8, AudioCodecType::WMAudioV8),
        std::make_pair(MFAudioFormat_ALAC, AudioCodecType::ALAC),
        std::make_pair(MFAudioFormat_FLAC, AudioCodecType::FLAC),
        std::make_pair(MFAudioFormat_PCM, AudioCodecType::PCM),
        std::make_pair(MFAudioFormat_AMR_NB, AudioCodecType::AMR_NB),
    };

    this->videoCodecMap = {
        std::make_pair(MFVideoFormat_H264, VideoCodecType::H264),
        std::make_pair(MFVideoFormat_HEVC, VideoCodecType::HEVC),
        std::make_pair(MFVideoFormat_WMV1, VideoCodecType::WMV1),
        std::make_pair(MFVideoFormat_WMV3, VideoCodecType::WMV3),
    };
}

void MediaFormatCodecsSupport::Add(
    MediaContainerType containerType,
    std::vector<AudioCodecType> audioCodecs,
    std::vector<VideoCodecType> videoCodecs)
{
    auto idx = MediaFormatCodecsSupport::MediaContainerTypeToIdx(containerType);

    if (idx >= this->codecsSupport.size()) {
        this->codecsSupport.resize(idx + 1);
    }

    this->codecsSupport[idx] = MediaFormatCodecs(containerType, std::move(audioCodecs), std::move(videoCodecs));
}

size_t MediaFormatCodecsSupport::MediaContainerTypeToIdx(MediaContainerType containerType) {
    return (size_t)containerType - (size_t)MediaContainerType::First;
}