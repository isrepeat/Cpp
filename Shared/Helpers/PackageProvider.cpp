#include "PackageProvider.h"
#include "Helpers.h"
#include "String.h"
#include <vector>
#include <Windows.h>
#include <appmodel.h> // must be included after Windows.h

namespace H {
    bool PackageProvider::IsRunningUnderPackage() {
        UINT32 length = 0;
        LONG rc = GetCurrentPackageFamilyName(&length, NULL);
        if (rc == APPMODEL_ERROR_NO_PACKAGE) {
            return false;
        }
        return true;
    }

    std::wstring PackageProvider::GetPackageVersion() {
        UINT32 count = 0;
        UINT32 bufferSize = 0;
        auto rc = GetCurrentPackageInfo(PACKAGE_INFORMATION_BASIC, &bufferSize, nullptr, &count);
        if (rc != ERROR_INSUFFICIENT_BUFFER) {
            if (rc == APPMODEL_ERROR_NO_PACKAGE)
                wprintf(L"Process has no package identity\n");
            else
                wprintf(L"Error %d in GetCurrentPackageInfo\n", rc);

            return L"1.0.0.0";
        }


        std::vector<uint8_t> buffer(bufferSize);
        rc = GetCurrentPackageInfo(PACKAGE_INFORMATION_BASIC, &bufferSize, buffer.data(), &count);
        if (rc != ERROR_SUCCESS) {
            wprintf(L"Error %d retrieving PackageInfo\n", rc);
            return L"1.0.0.0";
        }

        auto pacakgeFamilyName = GetPackageFamilyName();

        PACKAGE_INFO* pkgInfoPtr = reinterpret_cast<PACKAGE_INFO*>(buffer.data());
        for (int i = 0; i < count; i++) {
            auto pkgInfo = pkgInfoPtr[i];
            if (std::wstring{ pkgInfo.packageFamilyName } == pacakgeFamilyName) {
                auto major = pkgInfo.packageId.version.Major;
                auto minor = pkgInfo.packageId.version.Minor;
                auto build = pkgInfo.packageId.version.Build;
                auto revision = pkgInfo.packageId.version.Revision;
                return H::StringFormat(L"%d.%d.%d.%d", major, minor, build, revision);
            }
        }

        return L"1.0.0.0";
    }

    std::wstring PackageProvider::GetPackageFamilyName() {
        UINT32 length = 0;
        LONG rc = GetCurrentPackageFamilyName(&length, NULL);
        if (rc != ERROR_INSUFFICIENT_BUFFER) {
            if (rc == APPMODEL_ERROR_NO_PACKAGE)
                wprintf(L"Process has no package identity\n");
            else
                wprintf(L"Error %d in GetCurrentPackageFamilyName\n", rc);

            return L"";
        }

        std::vector<wchar_t> familyName(length);
        rc = GetCurrentPackageFamilyName(&length, familyName.data());
        if (rc != ERROR_SUCCESS) {
            wprintf(L"Error %d retrieving PackageFamilyName\n", rc);
            return L"";
        }

        return H::VecToWStr(familyName);
    }

    std::wstring PackageProvider::GetApplicationUserModelId() {
        UINT32 length = 0;
        LONG rc = GetCurrentApplicationUserModelId(&length, NULL);
        if (rc != ERROR_INSUFFICIENT_BUFFER) {
            if (rc == APPMODEL_ERROR_NO_PACKAGE)
                wprintf(L"Process has no package identity\n");
            else
                wprintf(L"Error %d in GetCurrentApplicationUserModelId\n", rc);

            return L"";
        }

        std::vector<wchar_t> appModelUserId(length);
        rc = GetCurrentApplicationUserModelId(&length, appModelUserId.data());
        if (rc != ERROR_SUCCESS) {
            wprintf(L"Error %d retrieving ApplicationUserModelId\n", rc);
            return L"";
        }

        return H::VecToWStr(appModelUserId);
    }
}