//#include <imapi2.h>
//#include <imapi2fs.h>
//#include <imapi2error.h>
//#include <imapi2fserror.h>
//#include <atlbase.h> // CComBstr
//#include <wrl.h>
//#include <memory>
//
//#include "main_47_Utils.h"
//
//
//class DiscMaster {
//public:
//	DiscMaster() {
//		HRESULT hr = CoCreateInstance(__uuidof(MsftDiscMaster2), nullptr, CLSCTX_ALL, IID_PPV_ARGS(discMaster.GetAddressOf()));
//		DBREAK_IF_FAIL(hr);
//	}
//
//	LONG GetDeviceNumber() {
//		LONG deviceCount;
//		HRESULT hr = discMaster->get_Count(&deviceCount);
//		DBREAK_IF_FAIL(hr);
//		return deviceCount;
//	}
//
//	ATL::CComBSTR GetUniqueIdForDriveIndex(LONG driveIndex) {
//		ATL::CComBSTR uniqueId;
//
//		if (driveIndex < GetDeviceNumber()) {
//			HRESULT hr = discMaster->get_Item(driveIndex, &uniqueId);
//			DBREAK_IF_FAIL(hr);
//		}
//		else {
//			dbreak;
//		}
//
//		return uniqueId;
//	}
//
//private:
//	Microsoft::WRL::ComPtr<IDiscMaster2> discMaster;
//};