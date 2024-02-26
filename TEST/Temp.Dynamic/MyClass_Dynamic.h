#pragma once
#include "Preprocessor.h"
#pragma message(PREPROCESSOR_FILE_INCLUDED("MyClass_Dynamic.h"))

#ifdef __MAKE_DYNAMIC_LIBRARY__
#define CLASS_API __declspec(dllexport)
#define DLL_API __declspec(dllexport)
#else
#define CLASS_API __declspec(dllimport)
#define DLL_API __declspec(dllimport)
#endif


// WARNING: Ensure that "Use Library Dependency Inputs" is set to "true"
//#include "../Temp.Static/MyClass_Static.h"

void DLL_API MethodFromDynamicLibrary();