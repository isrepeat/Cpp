#define __MAKE_DYNAMIC_LIBRARY__
#include "MyClass_Dynamic.h"

//#include <Spdlog/LogHelpers.h>
#define LOGGER_API
//#define LOGGER_API __declspec(dllimport) 
//#define LOGGER_API __declspec(dllexport) 
#include <Spdlog/LogHelpers.h>


void DLL_API MethodFromDynamicLibrary() {
	lg::DefaultLoggers::Init("");
	int xx = 9;
}