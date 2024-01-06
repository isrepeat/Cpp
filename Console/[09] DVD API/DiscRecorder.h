#pragma once
#include <imapi2.h>
#include <imapi2fs.h>
#include <imapi2error.h>
#include <imapi2fserror.h>
#include <atlbase.h> // CComBstr
#include <cassert>
#include <memory>
#include <filesystem>
#include <string>
#include <wrl.h>

#include "main_47_Utils.h"


class DiscRecorder {
public:
	DiscRecorder(BSTR uniqueId) {
		HRESULT hr = CoCreateInstance(__uuidof(MsftDiscRecorder2), nullptr, CLSCTX_ALL, IID_PPV_ARGS(discRecorder.GetAddressOf()));
		DBREAK_IF_FAIL(hr);

		hr = discRecorder->InitializeDiscRecorder(uniqueId);
		DBREAK_IF_FAIL(hr);

		activeVolumePathName = GetActiveVolumePathName();
	}

	Microsoft::WRL::ComPtr<IDiscRecorder2> GetRecorder() const {
		return discRecorder;
	}

	std::filesystem::path GetActiveVolumePath() const {
		return activeVolumePathName;
	}

	wchar_t GetActiveVolumeDriveLetter() const {
		return PathGetDriveNumberW(activeVolumePathName.c_str());
	}

private:
	std::filesystem::path GetActiveVolumePathName() {
		HRESULT hr = S_OK;
		
		SAFEARRAYUnique volumePathNames;
		hr = discRecorder->get_VolumePathNames(GetAddressOf(volumePathNames));
		DBREAK_IF_FAIL(hr);

		VARIANT HUGEP* pbstr = nullptr;
		LONG iLBound;
		LONG iUBound;
		
		hr = SafeArrayAccessData(volumePathNames.get(), (void**)&pbstr);
		DBREAK_IF_FAIL(hr);

		hr = SafeArrayGetLBound(volumePathNames.get(), 1, &iLBound);
		DBREAK_IF_FAIL(hr);

		hr = SafeArrayGetUBound(volumePathNames.get(), 1, &iUBound);
		DBREAK_IF_FAIL(hr);

		if (pbstr->vt == VT_BSTR) {
			std::wstring dataStr;
			dataStr = pbstr[0].bstrVal;
			return std::filesystem::path(dataStr);
		}
		else {
			assert(false && "variant wrong type");
			return L"";
		}
	}

private:
	Microsoft::WRL::ComPtr<IDiscRecorder2> discRecorder;
	std::filesystem::path activeVolumePathName;
	//ATL::CComBSTR volumeName;
};