#pragma once
#include "ImportExport.h"
#include <string>

#ifdef __MAKE_DYNAMIC_LIBRARY__
#define CLASS_API EXPORT
#else
#define CLASS_API IMPORT
#endif

class CLASS_API ClassFromDynamicLib {
public:
	ClassFromDynamicLib();
	~ClassFromDynamicLib() = default;

	std::string Print();
};