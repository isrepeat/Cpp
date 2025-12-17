#pragma once
#include "Filesystem\IFilesystemFS.h"
#include "Filesystem\IWinStreamExt.h" // windows-only
#include <filesystem>

namespace H {
	namespace FS {
		inline bool CreateDirectories(const std::filesystem::path& path) {
			if (!std::filesystem::exists(path)) {
				return std::filesystem::create_directories(path);
			}

			return false;
		}
	}
}

#undef CreateFile