//#include "Preprocessor.h"
//#include "../Temp.Static/MyClass_Static.h"
//namespace Helpers = __Helpers_ns::Static;

#include "../Temp.Dynamic/MyClass_Dynamic.h"
//namespace LibraryHelpers = __Helpers_ns::Dynamic;


int main() {
	//Helpers::MyClass myClassStatic;
	////__Helpers_ns::MyClass myClassStatic; // FAIL: MyClass is ambigous
	//Helpers::MyTemplate<int>::templateNotes;
	//Helpers::MyTemplate<int> myTemplateStatic;
	
	Nuget::MyClass myClassDLL;
	Nuget::MyTemplate<int>::templateNotes;
	myClassDLL.data->;
	//auto data = myClassDLL.GetData();

	return 0;
}