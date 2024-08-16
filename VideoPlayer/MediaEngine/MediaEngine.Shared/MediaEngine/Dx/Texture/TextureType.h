#pragma once

enum class TextureType : int{
	Unknown = -1,

	I420,
	NV12,
	RGB32,
	ARGB32 = RGB32,
	ABGR32,
	R8,

	Count
};