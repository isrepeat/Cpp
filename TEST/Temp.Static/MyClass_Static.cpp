#include "MyClass_Static.h"


#define LOGGER_API
//#define LOGGER_API __declspec(dllimport) 
//#define LOGGER_API __declspec(dllexport) 
#include <Spdlog/LogHelpers.h>

int MyClass_Static::Hello() {
	return 14;
}

void Foo() {
	int xx = 9;
}