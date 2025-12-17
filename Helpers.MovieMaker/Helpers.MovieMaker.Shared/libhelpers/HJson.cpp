#include "pch.h"
#include "HJson.h"
#include "libhelpers\HMath.h"

#if HAVE_WINRT == 1
using namespace Windows::Data::Json;

// serialize
Windows::Data::Json::IJsonValue ^HJson::Serialize(bool v) {
	auto json = JsonValue::CreateBooleanValue(v);
	return json;
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(int v) {
	return HJson::Serialize(std::to_wstring(v));
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(uint32_t v) {
	return HJson::Serialize(std::to_wstring(v));
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(int64_t v) {
	return HJson::Serialize(std::to_wstring(v));
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(uint64_t v) {
	return HJson::Serialize(std::to_wstring(v));
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(double v) {
	auto json = JsonValue::CreateNumberValue(v);
	return json;
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(float v) {
	return HJson::Serialize((double)v);
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const std::string &str) {
	auto tmp = H::Data::Convert<Platform::String ^>(str);
	auto json = JsonValue::CreateStringValue(tmp);

	return json;
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const std::wstring &str) {
	auto tmp = H::Data::Convert<Platform::String ^>(str);
	auto json = JsonValue::CreateStringValue(tmp);

	return json;
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const D2D1_RECT_F &v) {
	auto json = ref new JsonObject();

	HJson::Set(json, "left", v.left);
	HJson::Set(json, "top", v.top);
	HJson::Set(json, "right", v.right);
	HJson::Set(json, "bottom", v.bottom);

	return json;
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const DirectX::XMFLOAT3 &v) {
	auto json = ref new JsonObject();

	HJson::Set(json, "x", v.x);
	HJson::Set(json, "y", v.y);
	HJson::Set(json, "z", v.z);

	return json;
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const D2D1::ColorF &v) {
	return HJson::Serialize((const D2D1_COLOR_F &)v);
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const D2D1_COLOR_F &v) {
	auto json = ref new JsonObject();

	HJson::Set(json, "r", v.r);
	HJson::Set(json, "g", v.g);
	HJson::Set(json, "b", v.b);
	HJson::Set(json, "a", v.a);

	return json;
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const D2D1::Matrix3x2F &v) {
	return HJson::Serialize((const D2D1_MATRIX_3X2_F &)v);
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const D2D1_MATRIX_3X2_F &v) {
	auto json = ref new JsonArray();

	json->Append(JsonValue::CreateNumberValue(v._11));
	json->Append(JsonValue::CreateNumberValue(v._12));

	json->Append(JsonValue::CreateNumberValue(v._21));
	json->Append(JsonValue::CreateNumberValue(v._22));

	json->Append(JsonValue::CreateNumberValue(v._31));
	json->Append(JsonValue::CreateNumberValue(v._32));

	return json;
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const D2D1_POINT_2F &v) {
	auto json = ref new JsonObject();

	HJson::Set(json, "x", v.x);
	HJson::Set(json, "y", v.y);

	return json;
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const DirectX::XMFLOAT2 &v) {
	return HJson::Serialize(D2D1::Point2F(v.x, v.y));
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const D2D1_BEZIER_SEGMENT &v) {
	auto json = ref new JsonObject();

	HJson::Set(json, "point1", v.point1);
	HJson::Set(json, "point2", v.point2);
	HJson::Set(json, "point3", v.point3);

	return json;
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const D2D1_STROKE_STYLE_PROPERTIES &v) {
	auto json = ref new JsonObject();

	HJson::Set(json, "startCap", (int)v.startCap);
	HJson::Set(json, "endCap", (int)v.endCap);
	HJson::Set(json, "dashCap", (int)v.dashCap);
	HJson::Set(json, "lineJoin", (int)v.lineJoin);
	HJson::Set(json, "miterLimit", v.miterLimit);
	HJson::Set(json, "dashStyle", (int)v.dashStyle);
	HJson::Set(json, "dashOffset", v.dashOffset);

	return json;
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const DWRITE_TEXT_METRICS &v) {
	auto json = ref new JsonObject();

	HJson::Set(json, "left", v.left);
	HJson::Set(json, "top", v.top);
	HJson::Set(json, "width", v.width);
	HJson::Set(json, "widthIncludingTrailingWhitespace", v.widthIncludingTrailingWhitespace);
	HJson::Set(json, "layoutWidth", v.layoutWidth);
	HJson::Set(json, "layoutHeight", v.layoutHeight);
	HJson::Set(json, "maxBidiReorderingDepth", v.maxBidiReorderingDepth);
	HJson::Set(json, "lineCount", v.lineCount);

	return json;
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const Windows::Foundation::Size &v) {
	auto json = ref new JsonObject();

	HJson::Set(json, "w", v.Width);
	HJson::Set(json, "h", v.Height);

	return json;
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const D2D1_SIZE_F& obj) {
	auto json = ref new JsonObject();

	HJson::Set(json, "width", obj.width);
	HJson::Set(json, "height", obj.height);

	return json;
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const DirectX::XMFLOAT4 &v) {
	auto json = ref new JsonObject();

	HJson::Set(json, "x", v.x);
	HJson::Set(json, "y", v.y);
	HJson::Set(json, "z", v.z);
	HJson::Set(json, "w", v.w);

	return json;
}

Windows::Data::Json::IJsonValue ^HJson::Serialize(const Windows::Foundation::Point &v) {
	auto json = ref new JsonObject();

	HJson::Set(json, "X", v.X);
	HJson::Set(json, "Y", v.Y);

	return json;
}

Windows::Data::Json::IJsonValue ^ HJson::Serialize(const DirectX::XMUINT2 & v) {
	auto json = ref new JsonObject();

	HJson::Set(json, "XVal", v.x);
	HJson::Set(json, "YVal", v.y);

	return json;
}

Windows::Data::Json::IJsonValue ^ HJson::Serialize(const DirectX::XMVECTOR & v) {
	auto json = ref new JsonObject();

	HJson::Set(json, "XF", v.XF);
	HJson::Set(json, "YF", v.YF);
	HJson::Set(json, "ZF", v.ZF);
	HJson::Set(json, "WF", v.WF);

	return json;
}

Windows::Data::Json::IJsonValue ^ HJson::Serialize(const DirectX::XMUINT3 & v) {
	auto json = ref new JsonObject();
	
	HJson::Set(json, "X", v.x);
	HJson::Set(json, "Y", v.y);
	HJson::Set(json, "Z", v.z);

	return json;
}
#endif