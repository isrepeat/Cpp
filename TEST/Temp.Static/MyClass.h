#pragma once

#ifndef CLASS_API
#define CLASS_API __declspec(dllexport) // Compile this library with "dllexport" to export symbols through dll
#endif

class CLASS_API MyClass {
public:
	MyClass() = default;
	~MyClass() = default;

	int Hello();
};