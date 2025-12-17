#include "pch.h"
#include "MediaFormatCodecsEnumerator.h"
#include "Platform/PlatformClassFactory.h"

#include <libhelpers/HSystem.h>
#include <libhelpers/CoUniquePtr.h>
#include <libhelpers/Containers/ComPtrArray.h>
#include <libhelpers/MediaFoundation/MFInclude.h>

MediaFormatCodecsEnumerator::MediaFormatCodecsEnumerator()
    : codecSup(MediaFormatCodecsSupport::Instance())
{
    auto enumerator = PlatformClassFactory::Instance()->CreateEncoderEnumerator();

    enumerator->EnumAudio([&](const GUID &codec) {
        auto mapped = codecSup->MapAudioCodec(codec);

        if (mapped == AudioCodecType::Unknown) {
            return;
        }

        audioCodecs.push_back(mapped);
    });

    // always available
    audioCodecs.push_back(AudioCodecType::PCM);

    enumerator->EnumVideo([&](const GUID &codec) {
        auto mapped = codecSup->MapVideoCodec(codec);

        if (mapped == VideoCodecType::Unknown) {
            return;
        }

        videoCodecs.push_back(mapped);
    });
}

MediaFormatCodecs MediaFormatCodecsEnumerator::GetMediaFormatCodecs(MediaContainerType containerType) const {
    auto sup = codecSup->GetCodecsSupport(containerType);

    auto& supACodecs = sup.GetAudioCodecs();
    auto& supVCodecs = sup.GetVideoCodecs();

    auto availACodecs = MediaFormatCodecsEnumerator::Intersect(supACodecs, audioCodecs);
    auto availVCodecs = MediaFormatCodecsEnumerator::Intersect(supVCodecs, videoCodecs);

    if (availACodecs.empty() && availVCodecs.empty()) {
        return {};
    }

    MediaFormatCodecs codec(containerType, std::move(availACodecs), std::move(availVCodecs));
    return codec;
}

std::vector<MediaFormatCodecs> MediaFormatCodecsEnumerator::EnumerateMediaFormatCodecs() const {
    std::vector<MediaFormatCodecs> codecs;

    for (size_t i = 0; i < (size_t)MediaContainerType::Count; i++) {
        auto container = (MediaContainerType)((size_t)MediaContainerType::First + i);
        auto codec = GetMediaFormatCodecs(container);
        if (codec.GetMediaContainerType() != MediaContainerType::Unknown) {
            codecs.push_back(codec);
        }
    }

    return codecs;
}

template<class T>
static std::vector<T> MediaFormatCodecsEnumerator::Intersect(const std::vector<T> &a, const std::vector<T> &b) {
    std::vector<T> result;

    for (auto &i : a) {
        auto find = std::find(b.begin(), b.end(), i);
        if (find != b.end()) {
            result.push_back(*find);
        }
    }

    return result;
}