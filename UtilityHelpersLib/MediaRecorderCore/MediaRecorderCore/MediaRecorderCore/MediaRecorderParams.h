#pragma once
#include "IAvDxBufferFactory.h"
#include "MediaFormat/MediaFormat.h"

#include <memory>

struct MediaRecorderParams {
    MediaFormat mediaFormat;
    bool UseChunkMerger = false;
    bool chunkMergerTryRemux = true;
    std::wstring chunksPath;
    std::wstring chunksGuid;
    std::wstring targetRecordPath;
    std::shared_ptr<IAvDxBufferFactory> DxBufferFactory;
};