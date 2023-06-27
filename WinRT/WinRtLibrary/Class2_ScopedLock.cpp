//#include "Class2_ScopedLock.h"
//#include "../../Shared/HelpersWinRT/Helpers.h"
//#include "../../Shared/HelpersWinRT/HTime.h"
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
////Class2_ScopedLock::Class2_ScopedLock() {
////}
////
////Class2_ScopedLock::~Class2_ScopedLock() {
////	int xxx = 9;
////}
//
//
//Class2_Progress::Class2_Progress() {
//	HelpersWinRT::Timer::Once(std::chrono::milliseconds(3'000), [this] {
//		std::unique_lock<std::mutex> lk(this->mxData);
//		Sleep(100);
//		Sleep(100);
//		Sleep(100);
//		});
//}
//
//Class2_Progress::~Class2_Progress() {
//	int xxx = 9;
//}
//
//
////Class2_ScopedLock^ Class2_Progress::LockScoped() {
////	return ref new Class2_ScopedLock();
////}
//
//AAA::Class2_RefScopedLockMtx^ Class2_Progress::LockScoped() {
//	return ref new AAA::Class2_RefScopedLockMtx(this->mxData);
//}