#pragma once
#include "Preprocessor.h"
#pragma message(PREPROCESSOR_FILE_INCLUDED("MyClass_Static.h"))

#ifndef CLASS_API
#define CLASS_API __declspec(dllexport) // Compile this library with "dllexport" to export symbols through dll
#endif

class CLASS_API MyClass_Static {
public:
	MyClass_Static() = default;
	~MyClass_Static() = default;

	int Hello();
};

void Foo();