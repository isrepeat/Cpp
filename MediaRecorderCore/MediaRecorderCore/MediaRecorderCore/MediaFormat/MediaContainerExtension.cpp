#include "pch.h"
#include "MediaContainerExtension.h"

#include <algorithm>

MediaContainerExtension& MediaContainerExtension::Instance() {
    static MediaContainerExtension instance;
    return instance;
}

const std::wstring& MediaContainerExtension::GetFileExtension(MediaContainerType type) const {
    auto it = std::find_if(this->extensionInfo.begin(), this->extensionInfo.end(),
        [&type](const ExtensionInfo& i)
        {
            return type == i.type;
        });

    if (it == this->extensionInfo.end()) {
        const static std::wstring emptyStr;
        return emptyStr;
    }

    return it->fileExtension;
}

const MediaContainerType MediaContainerExtension::GetContainerType(const std::wstring& fileExtension) const {
    auto it = std::find_if(this->extensionInfo.begin(), this->extensionInfo.end(),
        [&fileExtension](const ExtensionInfo& i)
        {
            return fileExtension == i.fileExtension;
        });

    if (it == this->extensionInfo.end()) {
        return MediaContainerType::Unknown;
    }

    return it->type;
}

std::vector<MediaContainerExtension::ExtensionInfo> MediaContainerExtension::MakeExtensionInfo() {
    std::vector<ExtensionInfo> res =
    {
        ExtensionInfo{ MediaContainerType::MP4, L".mp4" },
        ExtensionInfo{ MediaContainerType::M4V, L".m4v" },
        ExtensionInfo{ MediaContainerType::WMV, L".wmv" },
        ExtensionInfo{ MediaContainerType::ASF, L".asf" },
        ExtensionInfo{ MediaContainerType::MOV, L".mov" },
        ExtensionInfo{ MediaContainerType::ThreeGP, L".3gp" },

        ExtensionInfo{ MediaContainerType::MP3, L".mp3" },
        ExtensionInfo{ MediaContainerType::M4A, L".m4a" },
        ExtensionInfo{ MediaContainerType::FLAC, L".flac" },
        ExtensionInfo{ MediaContainerType::WAV, L".wav" },
        ExtensionInfo{ MediaContainerType::WMA, L".wma" },
        ExtensionInfo{ MediaContainerType::ThreeGPP, L".3gpp" },
    };

    return res;
}
