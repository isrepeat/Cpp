#include "pch.h"
#include "HDataWinRT.h"

#if HAVE_WINRT == 1
namespace H {
	Windows::Data::Json::JsonArray ^DataWinRT::SavePoint(float x, float y) {
		Windows::Data::Json::JsonArray ^jsonPoint = ref new Windows::Data::Json::JsonArray;

		jsonPoint->Append(Windows::Data::Json::JsonValue::CreateNumberValue(x));
		jsonPoint->Append(Windows::Data::Json::JsonValue::CreateNumberValue(y));

		return jsonPoint;
	}

	void DataWinRT::LoadPoint(Windows::Data::Json::JsonArray ^json, float *x, float *y) {
		if (x) {
			*x = static_cast<float>(json->GetNumberAt(0));
		}

		if (y) {
			*y = static_cast<float>(json->GetNumberAt(1));
		}
	}
}
#endif