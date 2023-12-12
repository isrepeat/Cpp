#pragma once
#ifdef __MAKE_DYNAMIC_LIBRARY__
#define CLASS_API __declspec(dllexport) // used within Logger project
#else
#define CLASS_API __declspec(dllimport) // if nuget builds as dll redefine
#endif

// WARNING: Ensure that "Use Library Dependency Inputs" is set to "true"
#include "../Temp.Static/MyClass.h"