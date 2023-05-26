#pragma once
#define AudioCodecType_Enum \
    Unknown,                \
    AAC,                    \
    MP3,                    \
    DolbyAC3,               \
    FLAC,                   \
    ALAC,                   \
    WMAudioV8,              \
    PCM,                    \
    AMR_NB,

enum class AudioCodecType {
    AudioCodecType_Enum

    CountHelper,
    Count = CountHelper - 1, // don't count Unknown
    First = AAC
};