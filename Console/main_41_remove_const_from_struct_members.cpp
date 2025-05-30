#ifndef AVAKAR_DESTRUCTURE_64_H
#define AVAKAR_DESTRUCTURE_64_H

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace avakar {

	namespace _destructure_64 {

		template <std::size_t>
		struct any
		{
			template <typename T>
			operator T() const;
		};

		template <typename T, typename In, typename = void>
		struct has_n_members
			: std::false_type
		{
		};

		template <typename T, typename In>
		struct has_n_members<T&, In>
			: has_n_members<T, In>
		{
		};

		template <typename T, typename In>
		struct has_n_members<T&&, In>
			: has_n_members<T, In>
		{
		};

		template <typename T, std::size_t... In>
		struct has_n_members < T, std::index_sequence<In...>, std::void_t<decltype(T{ any<In>()... }) >>
			: std::true_type
		{
		};

		template <typename T, std::size_t N>
		constexpr bool has_n_members_v = has_n_members<T, std::make_index_sequence<N>>::value;

		struct untie
		{
			template <typename... Tn>
			std::tuple<Tn...> operator()(Tn &... an)
			{
				return std::tuple<Tn...>(std::move(an)...);
			}
		};

	}

	inline namespace destructure_64 {

		template <typename T>
		auto destructure(T& t)
		{
			using namespace _destructure_64;

			if constexpr (has_n_members_v<T, 64>)
			{
				static_assert(!has_n_members_v<T, 64>);
			}
			else if constexpr (has_n_members_v<T, 63>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54, m55, m56, m57, m58, m59, m60, m61, m62] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54, m55, m56, m57, m58, m59, m60, m61, m62);
			}
			else if constexpr (has_n_members_v<T, 62>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54, m55, m56, m57, m58, m59, m60, m61] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54, m55, m56, m57, m58, m59, m60, m61);
			}
			else if constexpr (has_n_members_v<T, 61>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54, m55, m56, m57, m58, m59, m60] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54, m55, m56, m57, m58, m59, m60);
			}
			else if constexpr (has_n_members_v<T, 60>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54, m55, m56, m57, m58, m59] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54, m55, m56, m57, m58, m59);
			}
			else if constexpr (has_n_members_v<T, 59>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54, m55, m56, m57, m58] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54, m55, m56, m57, m58);
			}
			else if constexpr (has_n_members_v<T, 58>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54, m55, m56, m57] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54, m55, m56, m57);
			}
			else if constexpr (has_n_members_v<T, 57>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54, m55, m56] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54, m55, m56);
			}
			else if constexpr (has_n_members_v<T, 56>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54, m55] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54, m55);
			}
			else if constexpr (has_n_members_v<T, 55>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53, m54);
			}
			else if constexpr (has_n_members_v<T, 54>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52, m53);
			}
			else if constexpr (has_n_members_v<T, 53>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51, m52);
			}
			else if constexpr (has_n_members_v<T, 52>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50, m51);
			}
			else if constexpr (has_n_members_v<T, 51>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49, m50);
			}
			else if constexpr (has_n_members_v<T, 50>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48, m49);
			}
			else if constexpr (has_n_members_v<T, 49>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47, m48);
			}
			else if constexpr (has_n_members_v<T, 48>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46, m47);
			}
			else if constexpr (has_n_members_v<T, 47>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45, m46);
			}
			else if constexpr (has_n_members_v<T, 46>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44, m45);
			}
			else if constexpr (has_n_members_v<T, 45>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43, m44);
			}
			else if constexpr (has_n_members_v<T, 44>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42, m43);
			}
			else if constexpr (has_n_members_v<T, 43>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41, m42);
			}
			else if constexpr (has_n_members_v<T, 42>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40, m41);
			}
			else if constexpr (has_n_members_v<T, 41>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39, m40);
			}
			else if constexpr (has_n_members_v<T, 40>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38, m39);
			}
			else if constexpr (has_n_members_v<T, 39>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37, m38);
			}
			else if constexpr (has_n_members_v<T, 38>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36, m37);
			}
			else if constexpr (has_n_members_v<T, 37>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35, m36);
			}
			else if constexpr (has_n_members_v<T, 36>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34, m35);
			}
			else if constexpr (has_n_members_v<T, 35>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33, m34);
			}
			else if constexpr (has_n_members_v<T, 34>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32, m33);
			}
			else if constexpr (has_n_members_v<T, 33>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31, m32);
			}
			else if constexpr (has_n_members_v<T, 32>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30, m31);
			}
			else if constexpr (has_n_members_v<T, 31>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29, m30);
			}
			else if constexpr (has_n_members_v<T, 30>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28, m29);
			}
			else if constexpr (has_n_members_v<T, 29>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27, m28);
			}
			else if constexpr (has_n_members_v<T, 28>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26, m27);
			}
			else if constexpr (has_n_members_v<T, 27>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25, m26);
			}
			else if constexpr (has_n_members_v<T, 26>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24, m25);
			}
			else if constexpr (has_n_members_v<T, 25>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23, m24);
			}
			else if constexpr (has_n_members_v<T, 24>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22, m23);
			}
			else if constexpr (has_n_members_v<T, 23>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21, m22);
			}
			else if constexpr (has_n_members_v<T, 22>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20, m21);
			}
			else if constexpr (has_n_members_v<T, 21>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19, m20);
			}
			else if constexpr (has_n_members_v<T, 20>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18, m19);
			}
			else if constexpr (has_n_members_v<T, 19>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17, m18);
			}
			else if constexpr (has_n_members_v<T, 18>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16, m17);
			}
			else if constexpr (has_n_members_v<T, 17>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15, m16);
			}
			else if constexpr (has_n_members_v<T, 16>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15);
			}
			else if constexpr (has_n_members_v<T, 15>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14);
			}
			else if constexpr (has_n_members_v<T, 14>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13);
			}
			else if constexpr (has_n_members_v<T, 13>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12);
			}
			else if constexpr (has_n_members_v<T, 12>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11);
			}
			else if constexpr (has_n_members_v<T, 11>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10);
			}
			else if constexpr (has_n_members_v<T, 10>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8, m9] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9);
			}
			else if constexpr (has_n_members_v<T, 9>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7, m8] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7, m8);
			}
			else if constexpr (has_n_members_v<T, 8>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6, m7] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6, m7);
			}
			else if constexpr (has_n_members_v<T, 7>)
			{
				auto&& [m0, m1, m2, m3, m4, m5, m6] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5, m6);
			}
			else if constexpr (has_n_members_v<T, 6>)
			{
				auto&& [m0, m1, m2, m3, m4, m5] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4, m5);
			}
			else if constexpr (has_n_members_v<T, 5>)
			{
				auto&& [m0, m1, m2, m3, m4] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3, m4);
			}
			else if constexpr (has_n_members_v<T, 4>)
			{
				auto&& [m0, m1, m2, m3] = std::forward<T>(t);
				return std::tie(m0, m1, m2, m3);
			}
			else if constexpr (has_n_members_v<T, 3>)
			{
				auto&& [m0, m1, m2] = std::forward<T>(t);
				return std::tie(m0, m1, m2);
			}
			else if constexpr (has_n_members_v<T, 2>)
			{
				auto&& [m0, m1] = std::forward<T>(t);
				return std::tie(m0, m1);
			}
			else if constexpr (has_n_members_v<T, 1>)
			{
				auto&& [m0] = std::forward<T>(t);
				return std::tie(m0);
			}
			else
			{
				return std::tuple{};
			}
		}

	}

	template <typename T>
	auto destructure(T&& t)
	{
		return std::apply(_destructure_64::untie(), destructure(t));
	}

}

#endif


#include <Windows.h>
#include <tuple>
#include <string>
#include <type_traits>
#include <cassert>

//template <class T, class... TArgs> decltype(void(T{ std::declval<TArgs>()... }), std::true_type{}) test_is_braces_constructible(int);
//template <class, class...> std::false_type test_is_braces_constructible(...);
//template <class T, class... TArgs> using is_braces_constructible = decltype(test_is_braces_constructible<T, TArgs...>(0));
//
//struct any_type {
//    template<class T>
//    constexpr operator T(); // non explicit
//};
//
//template<class T>
//auto to_tuple(T&& object) noexcept {
//    using type = std::decay_t<T>;
//    if constexpr (is_braces_constructible<type, any_type, any_type, any_type, any_type>{}) {
//        auto&& [p1, p2, p3, p4] = object;
//        return std::make_tuple(p1, p2, p3, p4);
//    }
//    else if constexpr (is_braces_constructible<type, any_type, any_type, any_type>{}) {
//        auto&& [p1, p2, p3] = object;
//        return std::make_tuple(p1, p2, p3);
//    }
//    else if constexpr (is_braces_constructible<type, any_type, any_type>{}) {
//        auto&& [p1, p2] = object;
//        return std::make_tuple(p1, p2);
//    }
//    else if constexpr (is_braces_constructible<type, any_type>{}) {
//        auto&& [p1] = object;
//        return std::make_tuple(p1);
//    }
//    else {
//        return std::make_tuple();
//    }
//}




template <typename T>
struct RemoveConst {
	RemoveConst(const T& constMemberRef)
		: constMemberRef{ constMemberRef }
	{
	}

	T& operator=(T newValue) {
		//T& memberRef = const_cast<T&>(constMemberRef);
		return const_cast<T&>(constMemberRef) = newValue;
	}

private:
	const T& constMemberRef;
};



class MyClass {
public:
	MyClass() = default;
	~MyClass() = default;

	struct Data {
		int a = 111;
		const bool b = true;
		const float c = 3.14f;

		//template <typename T>
		const int& operator->(int a) {
			return 2;
		}
	};

	const Data& GetData() {
		return data;
	}

private:
	const Data data;
};





template <typename T>
struct Rv {
	T value;
};

template <typename T>
void operator << (const T& constMember, const Rv<T>& valueWrapper) {
	const_cast<T&>(constMember) = valueWrapper.value;
}


struct Accessor {
	//template <typename T>
	void operator= (int newValue) {

	}
};

template <typename T>
void RewriteConstRef(const T& constRefValue, T newValue) {
	const_cast<T&>(constRefValue) = newValue;
}



int main() {
	MyClass myClass;

	auto& myDataRef = myClass.GetData();
	auto& ref = RemoveConst(myDataRef.a) = 333;

	auto myDataCopy = myClass.GetData();
	RemoveConst(myDataCopy.a) = 222;

	
	ref = 444;

	//myData.a = 777;
	//myDataRef.a = 777;

	//Data dataWithConstMembers;

	//dataWithConstMembers.a << Rv<int>{ 666 };

	//RewriteConstRef(dataWithConstMembers.a, 666);

	int xx = 9;



 //   {
 //       struct X {
 //           int p1;
 //           double p2;
 //       };

	//	//X x = X{ 1, 2.0 };

 // //      auto t = avakar::destructure(x);
 // //      //std::tuple<int, double> t = to_tuple(s{ 1, 2.0 });
 // //      static_assert(std::is_same_v<std::tuple<int&, double&>, decltype(t)>);
 // //      assert(1 == std::get<0>(t));
 // //      assert(2.0 == std::get<1>(t));
 //   }


	////{
	////	struct X
	////	{
	////		int a;
	////		int b;
	////	};

	////	X x;
	////	auto tied = avakar::destructure(x);
	////	//static_assert(std::is_same_v<decltype(tied), std::tuple<int&, int&>>);
	////}


	////Data defaultData;

 ////   auto t = to_tuple(defaultData);
 ////   static_assert(std::is_same<std::tuple<int, bool, float>, decltype(t)>{});

	return 0;
}
