#include "AudioCodecCompressedSettings.h"
#include "VideoCodecSimpleSettings.h"
#include "MediaTypeCreator.h"
#include <Helpers/Helpers.h>
#include "ChunkMerger.h"
#include "MFLibs.h"

// TODO: use constexpr string_view, find out how concat string_view
enum class CodecType {
    MP4,
    HEVC,
};
enum class StreamType {
    //Audio,
    Video,
    AudioVideo,
};
enum class ResolutionType {
    _1920x1080,
    _3840x2400,
};

std::wstring GetCodecFolder(CodecType codec) {
    switch (codec) {
    case CodecType::MP4:
        return L"MP4";
    case CodecType::HEVC:
        return L"HEVC";
    }
    return L"";
}

std::wstring GetStreamTypeFolder(StreamType streamType) {
    switch (streamType) {
    //case StreamType::Audio:
    //    return L"Audio";
    case StreamType::Video:
        return L"Video";
    case StreamType::AudioVideo:
        return L"AudioVideo";
    }
    return L"";
}

std::wstring GetResolutionFolder(ResolutionType resolution) {
    switch (resolution) {
    case ResolutionType::_1920x1080:
        return L"1920x1080";
    case ResolutionType::_3840x2400:
        return L"3840x2400";
    }
    return L"";
}


CodecType codec = CodecType::HEVC;
StreamType streamType = StreamType::AudioVideo;
ResolutionType resolution = ResolutionType::_1920x1080;

// default capture audio settings:
constexpr int NUM_CHANNELS = 1;
constexpr int SAMPLE_RATE = 44100;
constexpr int AUDIO_BIT_RATE = 192000;
const uint32_t audioSampleBits = 16;

// default capture video settings:
constexpr int FPS = 30;
//constexpr int WIDTH = 3840;
//constexpr int HEIGHT = 2400;
constexpr int WIDTH = 1920;
constexpr int HEIGHT = 1080;
constexpr int VIDEO_BIT_RATE = 5 * 1024 * 1024;


const std::wstring testFolder = L"D:\\WORK\\TEST\\ScreenRecorder\\TestChunks";
const std::wstring chunksFolder = testFolder + L"\\" + GetStreamTypeFolder(streamType) + L"\\" + GetCodecFolder(codec) + L"\\" + GetResolutionFolder(resolution);
const std::wstring outputFolder = testFolder + L"\\" + GetStreamTypeFolder(streamType) + L"\\" + GetCodecFolder(codec);
const std::wstring containerExt = L".mp4";

int main() {
    HRESULT hr = S_OK;
    hr = MFStartup(MF_VERSION);

    AudioCodecType audioCodec = AudioCodecType::AAC;
    VideoCodecType videoCodec = VideoCodecType::HEVC;

    AudioCodecBasicSettings audioCodecBasicSettings;
    audioCodecBasicSettings.numChannels = NUM_CHANNELS;
    audioCodecBasicSettings.sampleRate = SAMPLE_RATE;

    AudioCodecBitrateSettings audioCodecBitrateSettings;
    audioCodecBitrateSettings.bitrate = AUDIO_BIT_RATE;

    VideoCodecBasicSettings videoCodecBasicSettings;
    videoCodecBasicSettings.fps = FPS;
    videoCodecBasicSettings.width = WIDTH;
    videoCodecBasicSettings.height = HEIGHT;
    videoCodecBasicSettings.bitrate = VIDEO_BIT_RATE;


    auto audioSettings = std::make_unique<AudioCodecCompressedSettings>(audioCodec);
    audioSettings->SetBasicSettings(audioCodecBasicSettings);
    audioSettings->SetBitrateSettings(audioCodecBitrateSettings);
    //audioSettings = nullptr;

    auto videoSettings = std::make_unique<VideoCodecSimpleSettings>(videoCodec);
    videoSettings->SetBasicSettings(videoCodecBasicSettings);
    //videoSettings = nullptr;
    
    auto mediaTypeAudioIn = MediaTypeCreator::CreateAudioInMediaType(audioSettings.get(), audioSampleBits);
    auto mediaTypeAudioOut = MediaTypeCreator::CreateAudioAACOutMediaType(audioSettings.get()); // AAC

    auto mediaTypeVideoIn = MediaTypeCreator::CreateVideoInMediaType(videoSettings.get(), true);
    auto mediaTypeVideoOut = MediaTypeCreator::CreateVideoOutMediaType(videoSettings.get());

    
    const int chunkCount = 1;
    std::vector<std::wstring> chunks;
    chunks.reserve(chunkCount);

    for (size_t i = 0; i < chunkCount; ++i) {
        auto chunkFile = chunksFolder + L"\\chunk-" + std::to_wstring(i) + containerExt;
        chunks.push_back(std::move(chunkFile));
    }


	//ChunkMerger chunkMerger(outputFolder + L"\\output" + containerExt, nullptr, nullptr, nullptr, nullptr, std::move(chunks));
	ChunkMerger chunkMerger(outputFolder + L"\\output" + containerExt, mediaTypeAudioIn, mediaTypeAudioOut, mediaTypeVideoIn, mediaTypeVideoOut, std::move(chunks));
    chunkMerger.Merge();

    return 0;
}