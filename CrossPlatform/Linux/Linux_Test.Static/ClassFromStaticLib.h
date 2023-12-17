#pragma once
#include <string>

class ClassFromStaticLib {
public:
	ClassFromStaticLib();
	~ClassFromStaticLib() = default;

	std::string Print();
};