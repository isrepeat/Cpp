#pragma once
#include "common.h"
#include <functional>
#include <filesystem>
#include <string>

//extern "C" {
	namespace ComApi {
		std::filesystem::path GetPackageFolder(); // Absolute path to the App package LocalState folder
		std::wstring WindowsVersion();
	}
//}