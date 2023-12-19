#define __MAKE_DYNAMIC_LIBRARY__
#include "ClassFromDynamicLib.h"

ClassFromDynamicLib::ClassFromDynamicLib() {
}

std::string ClassFromDynamicLib::Print() {
	return "I am DynamicClass";
}
