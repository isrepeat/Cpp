//#include "Class1_DuplicatesContainer.h"
//#include <Windows.h>
//#include <ppltasks.h>
//#include <shellapi.h>
//#include <errhandlingapi.h>
//#include <filesystem>
//#include <fstream>
//#include <string>
//
//using namespace WinRtLibrary;
//using namespace Platform;
//
//Class1_DuplicatesContainer::Class1_DuplicatesContainer()
//{		
//	duplicatesCountByFileType = ref new Platform::Collections::Map<Platform::String^, Platform::IBox<uint64_t>^>();
//	//auto at1 = duplicatesCountByFileType->Lookup(".pdf");
//	
//
//	for (int i = 0; i < 10; i++) {
//		uint64_t prevValue = 0;
//		if (duplicatesCountByFileType->HasKey(".pdf")) {
//			prevValue = duplicatesCountByFileType->Lookup(".pdf")->Value;
//		}
//		duplicatesCountByFileType->Insert(".pdf", ref new Platform::Box<uint64_t>(prevValue + 1));
//	}
//	
//	auto at3 = duplicatesCountByFileType->Lookup(".pdf");
//	auto res3 = at3->Value;
//
//
//	duplicatesCountByFileType->Insert(".mp3", ref new Platform::Box<uint64_t>(0));
//
//	duplicatesCountByFileType->Lookup("AAA");
//
//	Sleep(2000);
//	Sleep(2000);
//	int xx = 9;
//}
//
//Class1_DuplicatesContainer::~Class1_DuplicatesContainer() {
//	int xxx = 9;
//}
//
//
//Windows::Foundation::Collections::IMap<Platform::String^, Platform::IBox<uint64_t>^>^ Class1_DuplicatesContainer::DuplicatesCountByFileType::get() {
//	return this->duplicatesCountByFileType;
//}
