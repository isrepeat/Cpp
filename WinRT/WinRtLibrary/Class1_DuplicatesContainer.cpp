#include "Class1_DuplicatesContainer.h"
#include <Windows.h>
#include <ppltasks.h>
#include <shellapi.h>
#include <errhandlingapi.h>
#include <filesystem>
#include <fstream>
#include <string>

using namespace WinRtLibrary;
using namespace Platform;

Class1_DuplicatesContainer::Class1_DuplicatesContainer()
{		
	duplicatesCountByFileType = ref new Platform::Collections::Map<Platform::String^, Temp^>();
	duplicatesCountByFileType->Insert("AAA", ref new Temp{});
	duplicatesCountByFileType->Insert("BBB", ref new Temp{});


	Sleep(2000);
	Sleep(2000);
	int xx = 9;
}

Class1_DuplicatesContainer::~Class1_DuplicatesContainer() {
	int xxx = 9;
}