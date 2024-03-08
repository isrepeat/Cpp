#include "Preprocessor.h"

#include "../Temp.Dynamic/MyClass_Dynamic.h"
#include <Spdlog/LogHelpers.h>
//#define CLASS_API
//#include "..\Temp.Static\MyClass_Static.h"


int main() {
	//Foo();

	//MyClass_Static myClass;
	//myClass.Hello();

	lg::DefaultLoggers::Init(L"");
	return 0;
}