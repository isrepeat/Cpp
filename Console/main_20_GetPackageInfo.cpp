//#include <vector>
//#include <iostream>
//#include <Windows.h>
//#include <appmodel.h>
//#include "../Shared/Helpers/String.h"
//#include "../Shared/Helpers/Helpers.h"
//
//std::wstring GetPackageFamilyNameInternal() {
//	UINT32 length = 0;
//	LONG rc = GetCurrentPackageFamilyName(&length, NULL);
//	if (rc != ERROR_INSUFFICIENT_BUFFER)
//	{
//		if (rc == APPMODEL_ERROR_NO_PACKAGE)
//			wprintf(L"Process has no package identity\n");
//		else
//			wprintf(L"Error %d in GetCurrentPackageFamilyName\n", rc);
//
//		return L"";
//	}
//
//	std::vector<wchar_t> familyName(length);
//
//	rc = GetCurrentPackageFamilyName(&length, familyName.data());
//	if (rc != ERROR_SUCCESS)
//	{
//		wprintf(L"Error %d retrieving PackageFamilyName\n", rc);
//		return L"";
//	}
//
//	return H::VecToWStr(familyName);
//}
//
//
//std::wstring GetPackageVersion() {
//	UINT32 count = 0;
//	UINT32 bufferSize = 0;
//	auto rc = GetCurrentPackageInfo(PACKAGE_INFORMATION_BASIC, &bufferSize, nullptr, &count);
//	if (rc != ERROR_INSUFFICIENT_BUFFER) {
//		if (rc == APPMODEL_ERROR_NO_PACKAGE)
//			wprintf(L"Process has no package identity\n");
//		else
//			wprintf(L"Error %d in GetCurrentPackageInfo\n", rc);
//
//		return L"1.0.0.0";
//	}
//
//	std::vector<uint8_t> buffer(bufferSize);
//	rc = GetCurrentPackageInfo(PACKAGE_INFORMATION_BASIC, &bufferSize, buffer.data(), &count);
//	if (rc != ERROR_SUCCESS) {
//		wprintf(L"Error %d retrieving PackageInfo\n", rc);
//		return L"1.0.0.0";
//	}
//
//
//	//std::vector<PACKAGE_INFO> packagesInfo(count);
//	//auto ptr = reinterpret_cast<uint8_t*>(packagesInfo.data());
//	//std::copy(buffer.begin(), buffer.end(), ptr);
//
//	
//	auto pacakgeFamilyName = GetPackageFamilyNameInternal();
//
//	PACKAGE_INFO* pkgInfoPtr = reinterpret_cast<PACKAGE_INFO*>(buffer.data());
//
//	for (int i = 0; i < count; i++) {
//		auto pkgInfo = pkgInfoPtr[i];
//		if (std::wstring{ pkgInfo.packageFamilyName } == pacakgeFamilyName) {
//			auto major = pkgInfo.packageId.version.Major;
//			auto minor = pkgInfo.packageId.version.Minor;
//			auto build = pkgInfo.packageId.version.Build;
//			auto revision = pkgInfo.packageId.version.Revision;
//			return H::StringFormat(L"%d.%d.%d.%d", major, minor, build, revision);
//		}
//	}
//	
//	int xxx = 9;
//
//
//	//std::vector<PACKAGE_INFO> pacakgesInfo(count);
//	//pacakgesInfo.data() = reinterpret_cast<PACKAGE_INFO*>(buffer.data());
//	//PACKAGE_INFO* packagesInfo[2]{};
//	//packagesInfo = reinterpret_cast<PACKAGE_INFO*>(buffer.data());
//	//std::copy(buffer.begin(), buffer.begin()+400, ptr);
//	//std::copy(buffer.begin(), buffer.end(), ptr);
//
//	//std::vector<uint8_t> bufferZeroes(bufferSize * 2, 0);
//	//auto ptr2 = reinterpret_cast<uint8_t*>(pacakgesInfo.data());
//	//std::copy(bufferZeroes.begin(), bufferZeroes.end(), ptr2);
//
//
//	//PACKAGE_INFO pacakgesInfo[10]{0};
//	//auto ptr = reinterpret_cast<uint8_t*>(&(pacakgesInfo[0]));
//	//std::copy(buffer.begin(), buffer.end(), ptr);
//
//	//std::vector<uint8_t> bufferZeroes(bufferSize*2, 0);
//	//auto ptr2 = reinterpret_cast<uint8_t*>(&(pacakgesInfo[0]));
//	//std::copy(bufferZeroes.begin(), bufferZeroes.end(), ptr2);
//
//
//	//std::vector<uint64_t> buffer2(bufferSize);
//	//auto ptr = reinterpret_cast<uint8_t*>(buffer2.data());
//
//	//pacakgesInfo.clear();
//
//	//return L"1.1.1.1";
//	//pacakgesInfo.pop_back(); // truncate trash
//
//
//	//auto pacakgeFamilyName = GetPackageFamilyNameInternal();
//
//	//for (auto& pacakgeInfo : pacakgesInfo) {
//	//	if (std::wstring{ pacakgeInfo.packageFamilyName } == pacakgeFamilyName) {
//	//		auto major = pacakgeInfo.packageId.version.Major;
//	//		auto minor = pacakgeInfo.packageId.version.Minor;
//	//		auto build = pacakgeInfo.packageId.version.Build;
//	//		auto revision = pacakgeInfo.packageId.version.Revision;
//	//		//#if _HAS_CXX20 == 1
//	//		//                return std::format(L"{}.{}.{}.{}", major, minor, build, revision);
//	//		//#else
//	//						//return std::to_wstring(major) + L"." + std::to_wstring(minor) + L"." + std::to_wstring(build) + L"." + std::to_wstring(revision);
//	//		return L"1.1.1.1"; 
//	//		//H::StringFormat(L"%d.%d.%d.%d", major, minor, build, revision);
//	//		//#endif
//	//	}
//	//}
//}
//
//
//int main() {
//	//auto familyName = GetPackageFamilyNameInternal();
//
//	//UINT32 count = 0;
//	//UINT32 bufferSize = 0;
//	//auto rc = GetCurrentPackageInfo(PACKAGE_INFORMATION_BASIC, &bufferSize, nullptr, &count);
//	//if (rc != ERROR_INSUFFICIENT_BUFFER) {
//	//	if (rc == APPMODEL_ERROR_NO_PACKAGE)
//	//		wprintf(L"Process has no package identity\n");
//	//	else
//	//		wprintf(L"Error %d in GetCurrentPackageInfo\n", rc);
//	//}
//
//
//	//std::vector<uint8_t> buffer(bufferSize);
//	//rc = GetCurrentPackageInfo(PACKAGE_INFORMATION_BASIC, &bufferSize, buffer.data(), &count);
//	//if (rc != ERROR_SUCCESS) {
//	//	wprintf(L"Error %d retrieving PackageInfo\n", rc);
//	//}
//
//	//std::vector<PACKAGE_INFO> pacakgesInfo(count+1);
//	//auto ptr = reinterpret_cast<uint8_t*>(pacakgesInfo.data());
//	//std::copy(buffer.begin(), buffer.end(), ptr);
//	//pacakgesInfo.pop_back(); // truncate
//
//	//for (auto& pacakgeInfo : pacakgesInfo) {
//	//	auto pkgFamilyName = pacakgeInfo.packageFamilyName;
//	//	if (pkgFamilyName == familyName) {
//	//		
//	//	}
//	//}
//
//	auto version = GetPackageVersion();
//
//
//
//	return 0;
//}