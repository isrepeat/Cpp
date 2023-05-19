#include <vector>
#include <string>
#include <iostream>
#include "MFLibs.h"
#include "ChunkMerger.h"
#include <Helpers/Helpers.h>

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


CodecType codec = CodecType::MP4;
StreamType streamType = StreamType::AudioVideo;
ResolutionType resolution = ResolutionType::_3840x2400;

const std::wstring testFolder = L"D:\\WORK\\TEST\\ScreenRecorder\\TestChunks";
const std::wstring chunksFolder = testFolder + L"\\" + GetStreamTypeFolder(streamType) + L"\\" + GetCodecFolder(codec) + L"\\" + GetResolutionFolder(resolution);
const std::wstring outputFolder = testFolder + L"\\" + GetStreamTypeFolder(streamType) + L"\\" + GetCodecFolder(codec);
const std::wstring containerExt = L".mp4";

int main() {
    const int chunkCount = 2;
    
    HRESULT hr = S_OK;
    hr = MFStartup(MF_VERSION);

    std::vector<std::wstring> chunks;
    chunks.reserve(chunkCount);

    for (size_t i = 0; i < chunkCount; ++i) {
        auto chunkFile = chunksFolder + L"\\chunk-" + std::to_wstring(i) + containerExt;
        chunks.push_back(std::move(chunkFile));
    }


	ChunkMerger chunkMerger(outputFolder + L"\\output" + containerExt, nullptr, nullptr, nullptr, nullptr, std::move(chunks));
    chunkMerger.Merge();

    return 0;
}