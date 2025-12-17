#pragma once
#include <Helpers/common.h>
#include <Helpers/Helpers.h>

#if COMPILE_FOR_DESKTOP
	const std::filesystem::path g_shaderLoadDir = HELPERS_NS::ExePath();
#elif COMPILE_FOR_CX
	const std::filesystem::path g_shaderLoadDir = HELPERS_NS::ExePath() / L"" PP_STRINGIFY(MSBuildProject__RootNamespace);
#endif	