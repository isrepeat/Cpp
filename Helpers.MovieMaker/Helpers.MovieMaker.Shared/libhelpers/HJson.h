#pragma once
#include "libhelpers\config.h"
#include "HData.h"

#include <DirectXMath.h>
#include <d2d1.h>
#include <dwrite.h>
#include <string>
#include <vector>

#if HAVE_WINRT == 1
class HJson {
public:
	template<class T> struct TypeDict {
		typedef typename T Type; // by default no type substitution
	};

	template<> struct TypeDict<std::vector<bool>::reference> {
		typedef bool Type; // use bool instead of std::vector<bool>::reference
	};

	// serialize
	template<class T>
	static void Set(Windows::Data::Json::JsonObject ^obj, Platform::String ^name, T &&v) {
		obj->SetNamedValue(name, HJson::Serialize(std::forward<typename TypeDict<T>::Type>(v)));
	}

	static Windows::Data::Json::IJsonValue ^Serialize(bool v);
	static Windows::Data::Json::IJsonValue ^Serialize(int v);
	static Windows::Data::Json::IJsonValue ^Serialize(uint32_t v);
	static Windows::Data::Json::IJsonValue ^Serialize(int64_t v);
	static Windows::Data::Json::IJsonValue ^Serialize(uint64_t v);
	static Windows::Data::Json::IJsonValue ^Serialize(double v);
	static Windows::Data::Json::IJsonValue ^Serialize(float v);
	static Windows::Data::Json::IJsonValue ^Serialize(const std::string &str);
	static Windows::Data::Json::IJsonValue ^Serialize(const std::wstring &str);
	static Windows::Data::Json::IJsonValue ^Serialize(const D2D1_RECT_F &v);
	static Windows::Data::Json::IJsonValue ^Serialize(const DirectX::XMFLOAT3 &v);	
	static Windows::Data::Json::IJsonValue ^Serialize(const D2D1::ColorF &v);
	static Windows::Data::Json::IJsonValue ^Serialize(const D2D1_COLOR_F &v);
	static Windows::Data::Json::IJsonValue ^Serialize(const D2D1::Matrix3x2F &v);
	static Windows::Data::Json::IJsonValue ^Serialize(const D2D1_MATRIX_3X2_F &v);
	static Windows::Data::Json::IJsonValue ^Serialize(const D2D1_POINT_2F &v);
	static Windows::Data::Json::IJsonValue ^Serialize(const DirectX::XMFLOAT2 &v);
	static Windows::Data::Json::IJsonValue ^Serialize(const D2D1_BEZIER_SEGMENT &v);
	static Windows::Data::Json::IJsonValue ^Serialize(const D2D1_STROKE_STYLE_PROPERTIES &v);
	static Windows::Data::Json::IJsonValue ^Serialize(const DWRITE_TEXT_METRICS &v);
	static Windows::Data::Json::IJsonValue ^Serialize(const Windows::Foundation::Size &v);
	static Windows::Data::Json::IJsonValue^ Serialize(const D2D1_SIZE_F& obj);
	static Windows::Data::Json::IJsonValue ^Serialize(const DirectX::XMFLOAT4 &v);
	static Windows::Data::Json::IJsonValue ^Serialize(const Windows::Foundation::Point &v);
	static Windows::Data::Json::IJsonValue^ Serialize(const DirectX::XMUINT2& v);
	static Windows::Data::Json::IJsonValue^ Serialize(const DirectX::XMVECTOR& v);
	static Windows::Data::Json::IJsonValue^ Serialize(const DirectX::XMUINT3& v);

	template<class T> static Windows::Data::Json::IJsonValue ^Serialize(const std::vector<T> &v) {
		auto json = ref new Windows::Data::Json::JsonArray();

		for (auto &i : v) {
			json->Append(HJson::Serialize(i));
		}

		return json;
	}

	// deserialize
	template<class T>
	static void TryGet(Windows::Data::Json::JsonObject ^obj, Platform::String ^name, T &v) {
		if (auto tmp = obj->Lookup(name)) {
			v = HJson::Get<typename TypeDict<T>::Type>(obj, name);
		}
	}

	template<class T>
	static void Get(Windows::Data::Json::JsonObject ^obj, Platform::String ^name, T &v) {
		v = HJson::Get<typename TypeDict<T>::Type>(obj, name);
	}

	template<class T>
	static typename TypeDict<T>::Type Get(Windows::Data::Json::JsonObject ^obj, Platform::String ^name) {
		auto val = obj->GetNamedValue(name);
		auto res = HJson::Deserialize<typename TypeDict<T>::Type>(val);

		return res;
	}

	template<class T> struct Deserializer {
		static T Deserialize(Windows::Data::Json::IJsonValue ^v) {
			static_assert(false, "Not implemented");
		}
	};

	template<class T>
	static T Deserialize(Windows::Data::Json::IJsonValue ^v) {
		return Deserializer<T>::Deserialize(v);
	}

	template<> struct Deserializer<bool> {
		static bool Deserialize(Windows::Data::Json::IJsonValue ^v) {
			bool res = v->GetBoolean();
			return res;
		}
	};

	template<> struct Deserializer<int> {
		static int Deserialize(Windows::Data::Json::IJsonValue ^v) {
			auto tmpStr = HJson::Deserialize<std::string>(v);
			auto res = std::stoi(tmpStr);
			return res;
		}
	};

	template<> struct Deserializer<uint32_t> {
		static uint32_t Deserialize(Windows::Data::Json::IJsonValue ^v) {
			auto tmpStr = HJson::Deserialize<std::string>(v);
			uint32_t res = std::stoul(tmpStr);
			return res;
		}
	};

	template<> struct Deserializer<int64_t> {
		static int64_t Deserialize(Windows::Data::Json::IJsonValue ^v) {
			auto tmpStr = HJson::Deserialize<std::string>(v);
			auto res = std::stoll(tmpStr);
			return res;
		}
	};

	template<> struct Deserializer<uint64_t> {
		static uint64_t Deserialize(Windows::Data::Json::IJsonValue ^v) {
			auto tmpStr = HJson::Deserialize<std::string>(v);
			auto res = std::stoull(tmpStr);
			return res;
		}
	};

	template<> struct Deserializer<double> {
		static double Deserialize(Windows::Data::Json::IJsonValue ^v) {
			double res = v->GetNumber();
			return res;
		}
	};

	template<> struct Deserializer<float> {
		static float Deserialize(Windows::Data::Json::IJsonValue ^v) {
			float res = (float)HJson::Deserialize<double>(v);
			return res;
		}
	};

	template<> struct Deserializer<std::string> {
		static std::string Deserialize(Windows::Data::Json::IJsonValue ^v) {
			auto tmp = v->GetString();
			auto res = H::Data::Convert<std::string>(tmp);

			return res;
		}
	};

	template<> struct Deserializer<std::wstring> {
		static std::wstring Deserialize(Windows::Data::Json::IJsonValue ^v) {
			auto tmp = v->GetString();
			auto res = H::Data::Convert<std::wstring>(tmp);

			return res;
		}
	};

	template<> struct Deserializer<D2D1_RECT_F> {
		static D2D1_RECT_F Deserialize(Windows::Data::Json::IJsonValue ^v) {
			D2D1_RECT_F res;
			auto tmp = v->GetObject();

			HJson::Get(tmp, "left", res.left);
			HJson::Get(tmp, "top", res.top);
			HJson::Get(tmp, "right", res.right);
			HJson::Get(tmp, "bottom", res.bottom);

			return res;
		}
	};

	template<> struct Deserializer<DirectX::XMFLOAT3> {
		static DirectX::XMFLOAT3 Deserialize(Windows::Data::Json::IJsonValue ^v) {
			DirectX::XMFLOAT3 res;
			auto tmp = v->GetObject();

			HJson::Get(tmp, "x", res.x);
			HJson::Get(tmp, "y", res.y);
			HJson::Get(tmp, "z", res.z);

			return res;
		}
	};

	template<> struct Deserializer<D2D1::ColorF> {
		static D2D1::ColorF Deserialize(Windows::Data::Json::IJsonValue ^v) {
			float r, g, b, a;
			auto tmp = v->GetObject();

			HJson::Get(tmp, "r", r);
			HJson::Get(tmp, "g", g);
			HJson::Get(tmp, "b", b);
			HJson::Get(tmp, "a", a);

			return D2D1::ColorF(r, g, b, a);
		}
	};

	template<> struct Deserializer<D2D1_COLOR_F> {
		static D2D1_COLOR_F Deserialize(Windows::Data::Json::IJsonValue ^v) {
			return HJson::Deserialize<D2D1::ColorF>(v);
		}
	};

	template<> struct Deserializer<D2D1::Matrix3x2F> {
		static D2D1::Matrix3x2F Deserialize(Windows::Data::Json::IJsonValue ^v) {
			auto tmp = HJson::Deserialize<D2D1_MATRIX_3X2_F>(v);
			return *D2D1::Matrix3x2F::ReinterpretBaseType(&tmp);
		}
	};

	template<> struct Deserializer<D2D1_MATRIX_3X2_F> {
		static D2D1_MATRIX_3X2_F Deserialize(Windows::Data::Json::IJsonValue ^v) {
			D2D1_MATRIX_3X2_F res;
			auto tmp = v->GetArray();

			res._11 = (float)tmp->GetNumberAt(0);
			res._12 = (float)tmp->GetNumberAt(1);

			res._21 = (float)tmp->GetNumberAt(2);
			res._22 = (float)tmp->GetNumberAt(3);

			res._31 = (float)tmp->GetNumberAt(4);
			res._32 = (float)tmp->GetNumberAt(5);

			return res;
		}
	};

	template<> struct Deserializer<D2D1_POINT_2F> {
		static D2D1_POINT_2F Deserialize(Windows::Data::Json::IJsonValue ^v) {
			D2D1_POINT_2F res;
			auto tmp = v->GetObject();

			HJson::Get(tmp, "x", res.x);
			HJson::Get(tmp, "y", res.y);

			return res;
		}
	};

	template<> struct Deserializer<DirectX::XMFLOAT2> {
		static DirectX::XMFLOAT2 Deserialize(Windows::Data::Json::IJsonValue ^v) {
			auto tmp = HJson::Deserialize<D2D1_POINT_2F>(v);
			DirectX::XMFLOAT2 res(tmp.x, tmp.y);

			return res;
		}
	};

	template<> struct Deserializer<D2D1_BEZIER_SEGMENT> {
		static D2D1_BEZIER_SEGMENT Deserialize(Windows::Data::Json::IJsonValue ^v) {
			D2D1_BEZIER_SEGMENT res;
			auto tmp = v->GetObject();

			HJson::Get(tmp, "point1", res.point1);
			HJson::Get(tmp, "point2", res.point2);
			HJson::Get(tmp, "point3", res.point3);

			return res;
		}
	};

	template<> struct Deserializer<D2D1_STROKE_STYLE_PROPERTIES> {
		static D2D1_STROKE_STYLE_PROPERTIES Deserialize(Windows::Data::Json::IJsonValue ^v) {
			D2D1_STROKE_STYLE_PROPERTIES res = { D2D1_CAP_STYLE_FLAT, D2D1_CAP_STYLE_FLAT,
				D2D1_CAP_STYLE_FLAT, D2D1_LINE_JOIN_MITER, 1.0f, D2D1_DASH_STYLE_SOLID, 0.0f};
			auto tmp = v->GetObject();

			if (res.startCap != NULL && res.endCap != NULL && 
				res.dashCap != NULL && res.lineJoin != NULL && 
				res.dashStyle != NULL) 
			{
				HJson::Get(tmp, "startCap", (int &)res.startCap);
				HJson::Get(tmp, "endCap", (int &)res.endCap);
				HJson::Get(tmp, "dashCap", (int &)res.dashCap);
				HJson::Get(tmp, "lineJoin", (int &)res.lineJoin);
				HJson::Get(tmp, "miterLimit", res.miterLimit);
				HJson::Get(tmp, "dashStyle", (int &)res.dashStyle);
				HJson::Get(tmp, "dashOffset", res.dashOffset);
			}

			return res;
		}
	};

	template<> struct Deserializer<DWRITE_TEXT_METRICS> {
		static DWRITE_TEXT_METRICS Deserialize(Windows::Data::Json::IJsonValue ^v) {
			DWRITE_TEXT_METRICS res;
			auto tmp = v->GetObject();

			HJson::Get(tmp, "left", res.left);
			HJson::Get(tmp, "top", res.top);
			HJson::Get(tmp, "width", res.width);
			HJson::Get(tmp, "widthIncludingTrailingWhitespace", res.widthIncludingTrailingWhitespace);
			HJson::Get(tmp, "layoutWidth", res.layoutWidth);
			HJson::Get(tmp, "layoutHeight", res.layoutHeight);
			HJson::Get(tmp, "maxBidiReorderingDepth", res.maxBidiReorderingDepth);
			HJson::Get(tmp, "lineCount", res.lineCount);

			return res;
		}
	};

	template<> struct Deserializer<Windows::Foundation::Size> {
		static Windows::Foundation::Size Deserialize(Windows::Data::Json::IJsonValue ^v) {
			Windows::Foundation::Size res;
			auto tmp = v->GetObject();

			HJson::Get(tmp, "w", res.Width);
			HJson::Get(tmp, "h", res.Height);

			return res;
		}
	};

	template<class T> struct Deserializer<std::vector<T>> {
		static std::vector<T> Deserialize(Windows::Data::Json::IJsonValue ^v) {
			std::vector<T> res;
			auto tmp = v->GetArray();

			res.reserve(tmp->Size);

			for (uint32_t i = 0; i < tmp->Size; i++) {
				res.push_back(HJson::Deserialize<T>(tmp->GetAt(i)));
			}

			return res;
		}
	};

	template<> struct Deserializer<D2D1_SIZE_F> {
		static D2D1_SIZE_F Deserialize(Windows::Data::Json::IJsonValue ^v) {
			D2D1_SIZE_F obj;
			auto tmp = v->GetObject();

			HJson::Get(tmp, "width", obj.width);
			HJson::Get(tmp, "height", obj.height);

			return obj;
		}
	};

	template<> struct Deserializer<DirectX::XMFLOAT4> {
		static DirectX::XMFLOAT4 Deserialize(Windows::Data::Json::IJsonValue ^v) {
			DirectX::XMFLOAT4 res;
			auto tmp = v->GetObject();

			HJson::Get(tmp, "x", res.x);
			HJson::Get(tmp, "y", res.y);
			HJson::Get(tmp, "z", res.z);
			HJson::Get(tmp, "w", res.w);

			return res;
		}
	};

	template<> struct Deserializer<Windows::Foundation::Point> {
		static Windows::Foundation::Point Deserialize(Windows::Data::Json::IJsonValue ^v) {
			Windows::Foundation::Point res;
			auto tmp = v->GetObject();

			HJson::Get(tmp, "X", res.X);
			HJson::Get(tmp, "Y", res.Y);

			return res;
		}
	};

	template<> struct Deserializer<DirectX::XMUINT2> {
		static DirectX::XMUINT2 Deserialize(Windows::Data::Json::IJsonValue^ v) {
			DirectX::XMUINT2 res;
			auto tmp = v->GetObject();

			HJson::Get(tmp, "XVal", res.x);
			HJson::Get(tmp, "YVal", res.y);

			return res;
		}
	};

	template<> struct Deserializer<DirectX::XMVECTOR> {
		static DirectX::XMVECTOR Deserialize(Windows::Data::Json::IJsonValue^ v) {
			float tmp[4];
			auto json = v->GetObject();

			HJson::Get(json, "XF", tmp[0]);
			HJson::Get(json, "YF", tmp[1]);
			HJson::Get(json, "ZF", tmp[2]);
			HJson::Get(json, "WF", tmp[3]);

			DirectX::XMVECTOR res = DirectX::XMVectorSet(tmp[0], tmp[1], tmp[2], tmp[3]);
			return res;
		}
	};

	template<> struct Deserializer<DirectX::XMUINT3> {
		static DirectX::XMUINT3 Deserialize(Windows::Data::Json::IJsonValue^ v) {
			DirectX::XMUINT3 res;
			auto json = v->GetObject();

			HJson::Get(json, "X", res.x);
			HJson::Get(json, "Y", res.y);
			HJson::Get(json, "Z", res.z);

			return res;
		}
	};
};
#endif