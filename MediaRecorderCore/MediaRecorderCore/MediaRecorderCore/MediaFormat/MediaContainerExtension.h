#pragma once
#include "MediaContainerType.h"

#include <vector>
#include <string>

class MediaContainerExtension {
public:
	static MediaContainerExtension& Instance();

	const std::wstring& GetFileExtension(MediaContainerType type) const;
	const MediaContainerType GetContainerType(const std::wstring& fileExtension) const;

private:
	struct ExtensionInfo {
		MediaContainerType type = MediaContainerType::Unknown;
		std::wstring fileExtension;
	};

	MediaContainerExtension() = default;

	static std::vector<ExtensionInfo> MakeExtensionInfo();

	std::vector<ExtensionInfo> extensionInfo = MakeExtensionInfo();
};
