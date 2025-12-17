#pragma once
#include "common.h"

#if COMPILE_FOR_DESKTOP
#include "Helpers/Localization.h"
#include <string>
#include <vector>

namespace HELPERS_NS {
	// TODO: rewrite all functions as singleton for usage in other singletons (or init other static variables)
	std::wstring GetUserNameW();
	std::wstring GetComputerNameW();
	Locale GetUserDefaultLocale();
	std::vector<Locale> GetUserPreferredUILocales();
}
#endif