#include "MediaFormatCodecsEnumerator.h"
#include "MediaFormatCodecsSupport.h"
//#include "Platform/PlatformClassFactory.h"

#include <Helpers/System.h>
//#include <libhelpers/CoUniquePtr.h>
//#include <libhelpers/Containers/ComPtrArray.h>
#include "MFInclude.h"

MediaFormatCodecsEnumerator::MediaFormatCodecsEnumerator() {
}

//std::vector<MediaFormatCodecs> MediaFormatCodecsEnumerator::EnumerateMediaFormatCodecs() const {
//    std::vector<AudioCodecType> audioCodecs;
//    std::vector<VideoCodecType> videoCodecs;
//    auto codecSup = MediaFormatCodecsSupport::Instance();
//
//    auto enumerator = PlatformClassFactory::Instance()->CreateEncoderEnumerator();
//
//    enumerator->EnumAudio([&](const GUID &codec) {
//        auto mapped = codecSup->MapAudioCodec(codec);
//
//        if (mapped == AudioCodecType::Unknown) {
//            return;
//        }
//
//        audioCodecs.push_back(mapped);
//    });
//
//    // always available
//    audioCodecs.push_back(AudioCodecType::PCM);
//
//    enumerator->EnumVideo([&](const GUID &codec) {
//        auto mapped = codecSup->MapVideoCodec(codec);
//
//        if (mapped == VideoCodecType::Unknown) {
//            return;
//        }
//
//        videoCodecs.push_back(mapped);
//    });
//
//    std::vector<MediaFormatCodecs> codecs;
//
//    for (size_t i = 0; i < (size_t)MediaContainerType::Count; i++) {
//        auto container = (MediaContainerType)((size_t)MediaContainerType::First + i);
//        auto sup = codecSup->GetCodecsSupport(container);
//
//        auto &supACodecs = sup.GetAudioCodecs();
//        auto &supVCodecs = sup.GetVideoCodecs();
//
//        auto availACodecs = MediaFormatCodecsEnumerator::Intersect(supACodecs, audioCodecs);
//        auto availVCodecs = MediaFormatCodecsEnumerator::Intersect(supVCodecs, videoCodecs);
//
//        if (availACodecs.empty() && availVCodecs.empty()) {
//            continue;
//        }
//
//        codecs.push_back(MediaFormatCodecs(container, std::move(availACodecs), std::move(availVCodecs)));
//    }
//
//    return codecs;
//}

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