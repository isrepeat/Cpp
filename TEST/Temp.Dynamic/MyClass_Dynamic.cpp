#define __MAKE_DYNAMIC_LIBRARY__
#include "MyClass_Dynamic.h"

//#define HELPERS_INLINE_NS Dynamic
#include "../Temp.Static/MyClass_Static_copy.h"
//#undef HELPERS_INLINE_NS


namespace NUGET_NS {
	inline namespace NUGET_INLINE_NS {
		MyClass::MyClass()
			: data{new __Helpers_ns::Static::MyTemplate<int>{} }
		{
			Helpers::MyClass myClassStatic;
			//NugetHelpers::MyTemplate<int>::templateNotes;
			//NugetHelpers::MyTemplate<int> myTemplateStatic;
		}

		//MyClass::_Data GetData() {
		//	return __Helpers_ns::Static::MyTemplate<int>{};
		//}

		void DLL_API SomeFunction() {
			MyClass myClass;
		}
	}
}