#include "PackageProvider.h"
#include "Helpers.h"
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

    std::wstring PackageProvider::GetApplicationUserModelId() {
        UINT32 length = 0;
        LONG rc = GetCurrentApplicationUserModelId(&length, NULL);
        if (rc != ERROR_INSUFFICIENT_BUFFER)
        {
            if (rc == APPMODEL_ERROR_NO_PACKAGE)
                wprintf(L"Process has no package identity\n");
            else
                wprintf(L"Error %d in GetCurrentPackageFamilyName\n", rc);

            return L"";
        }

        std::vector<wchar_t> familyName(length);

        rc = GetCurrentApplicationUserModelId(&length, familyName.data());
        if (rc != ERROR_SUCCESS)
        {
            wprintf(L"Error %d retrieving PackageFamilyName\n", rc);
            return L"";
        }

        return H::VecToWStr(familyName);
    }
}