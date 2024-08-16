#pragma once
#include "TextureType.h"

#include <dxgiformat.h>
#include <cstdint>
#include <vector>
#include <unordered_map>

enum class ColorComponent : uint32_t{
	R,
	G,
	B,
	A,
};

struct ComponentOrder{
	union{
		uint8_t Value;
		struct{
			ColorComponent C0 : 2;
			ColorComponent C1 : 2;
			ColorComponent C2 : 2;
			ColorComponent C3 : 2;
		};
	};
	
	ColorComponent Get(uint32_t idx);
	void Set(ColorComponent c0, ColorComponent c1, ColorComponent c2, ColorComponent c3);
};

struct PlaneComponentsInfo{
	uint32_t Count;
	uint32_t BitSize;
	ComponentOrder Order;

	bool operator==(const PlaneComponentsInfo &other) const;
};

struct PlaneComponentsInfoHash{
	std::size_t operator()(const PlaneComponentsInfo &v) const;
};

struct PlaneInfo{
	uint32_t WidthSamplePeriod;
	uint32_t HeightSamplePeriod;

	PlaneComponentsInfo Component;
};

class TextureDesc{
	static std::vector<PlaneInfo> TexDesc[(int) TextureType::Count];
	static std::unordered_map<PlaneComponentsInfo, DXGI_FORMAT, PlaneComponentsInfoHash> TexFormatDxMap;
	static int StaticCtorTmp;
	static int StaticCtor();
public:

	static size_t GetPlaneCount(TextureType type);
	static PlaneInfo GetPlaneInfo(TextureType type, size_t plane);
	static DXGI_FORMAT GetDxFormat(TextureType type, size_t plane);
};