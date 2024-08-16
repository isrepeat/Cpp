#include "TextureDesc.h"
#include <dxgiformat.h>
#include "../../HData.h"
#include <assert.h>

ColorComponent ComponentOrder::Get(uint32_t idx){
	assert(idx < 4);
	ColorComponent component = static_cast<ColorComponent>((this->Value >> (idx * 2)) & 3);
	return component;
}

void ComponentOrder::Set(ColorComponent c0, ColorComponent c1, ColorComponent c2, ColorComponent c3){
	this->Value = static_cast<uint8_t>(c0)
		| static_cast<uint8_t>(c1) << (1 * 2)
		| static_cast<uint8_t>(c2) << (2 * 2)
		| static_cast<uint8_t>(c3) << (3 * 2);
}




bool PlaneComponentsInfo::operator == (const PlaneComponentsInfo &other) const{
	bool same = this->BitSize == other.BitSize && this->Count == other.Count && this->Order.Value == other.Order.Value;
	return same;
}

std::size_t PlaneComponentsInfoHash::operator()(const PlaneComponentsInfo &v) const{
	std::size_t h1 = std::hash<uint32_t>()(v.Count);
	std::size_t h2 = std::hash<uint32_t>()(v.BitSize);
	std::size_t h3 = std::hash<uint32_t>()(v.Order.Value);

	return HData::CombineHash(h3, HData::CombineHash(h1, h2));
}



std::vector<PlaneInfo> TextureDesc::TexDesc[(int)TextureType::Count];
std::unordered_map<PlaneComponentsInfo, DXGI_FORMAT, PlaneComponentsInfoHash> TextureDesc::TexFormatDxMap;

int TextureDesc::StaticCtorTmp = TextureDesc::StaticCtor();

int TextureDesc::StaticCtor(){
	PlaneInfo info;

	// I420
	{
		auto &vec = TextureDesc::TexDesc[(int)TextureType::I420];

		info.WidthSamplePeriod = 1;
		info.HeightSamplePeriod = 1;
		info.Component.Count = 1;
		info.Component.BitSize = 8;
		info.Component.Order.Set(ColorComponent::R, ColorComponent::R, ColorComponent::R, ColorComponent::R);

		vec.push_back(info);

		info.WidthSamplePeriod = 2;
		info.HeightSamplePeriod = 2;
		info.Component.Count = 1;
		info.Component.BitSize = 8;
		info.Component.Order.Set(ColorComponent::R, ColorComponent::R, ColorComponent::R, ColorComponent::R);

		vec.push_back(info);
		vec.push_back(info);
	}
	// NV12
	{
		auto &vec = TextureDesc::TexDesc[(int)TextureType::NV12];

		info.WidthSamplePeriod = 1;
		info.HeightSamplePeriod = 1;
		info.Component.Count = 1;
		info.Component.BitSize = 8;
		info.Component.Order.Set(ColorComponent::R, ColorComponent::R, ColorComponent::R, ColorComponent::R);

		vec.push_back(info);

		info.WidthSamplePeriod = 2;
		info.HeightSamplePeriod = 2;
		info.Component.Count = 2;
		info.Component.BitSize = 8;
		info.Component.Order.Set(ColorComponent::R, ColorComponent::R, ColorComponent::R, ColorComponent::R);

		vec.push_back(info);
	}
	// ARGB32
	{
		auto &vec = TextureDesc::TexDesc[(int)TextureType::ARGB32];

		info.WidthSamplePeriod = 1;
		info.HeightSamplePeriod = 1;
		info.Component.Count = 4;
		info.Component.BitSize = 8;
		info.Component.Order.Set(ColorComponent::R, ColorComponent::G, ColorComponent::B, ColorComponent::A);

		vec.push_back(info);
	}
	// ABGR32
	{
		auto &vec = TextureDesc::TexDesc[(int)TextureType::ABGR32];

		info.WidthSamplePeriod = 1;
		info.HeightSamplePeriod = 1;
		info.Component.Count = 4;
		info.Component.BitSize = 8;
		info.Component.Order.Set(ColorComponent::B, ColorComponent::G, ColorComponent::R, ColorComponent::A);

		vec.push_back(info);
	}
	// R8
	{
		auto &vec = TextureDesc::TexDesc[(int)TextureType::R8];

		info.WidthSamplePeriod = 1;
		info.HeightSamplePeriod = 1;
		info.Component.Count = 1;
		info.Component.BitSize = 8;
		info.Component.Order.Set(ColorComponent::R, ColorComponent::R, ColorComponent::R, ColorComponent::R);

		vec.push_back(info);
	}

	{
		PlaneComponentsInfo component;

		component.Count = 1;
		component.BitSize = 8;
		component.Order.Set(ColorComponent::R, ColorComponent::R, ColorComponent::R, ColorComponent::R);
		TextureDesc::TexFormatDxMap.insert(std::make_pair(component, DXGI_FORMAT_R8_UNORM));

		component.Count = 2;
		component.BitSize = 8;
		component.Order.Set(ColorComponent::R, ColorComponent::R, ColorComponent::R, ColorComponent::R);
		TextureDesc::TexFormatDxMap.insert(std::make_pair(component, DXGI_FORMAT_R8G8_SNORM));

		component.Count = 4;
		component.BitSize = 8;
		component.Order.Set(ColorComponent::R, ColorComponent::G, ColorComponent::B, ColorComponent::A);
		TextureDesc::TexFormatDxMap.insert(std::make_pair(component, DXGI_FORMAT_R8G8B8A8_UNORM));

		component.Count = 4;
		component.BitSize = 8;
		component.Order.Set(ColorComponent::B, ColorComponent::G, ColorComponent::R, ColorComponent::A);
		TextureDesc::TexFormatDxMap.insert(std::make_pair(component, DXGI_FORMAT_B8G8R8A8_UNORM));
	}

	return 0;
}

size_t TextureDesc::GetPlaneCount(TextureType type){
	auto &vec = TextureDesc::TexDesc[(int)type];
	return vec.size();
}

PlaneInfo TextureDesc::GetPlaneInfo(TextureType type, size_t plane){
	auto &vec = TextureDesc::TexDesc[(int)type];
	return vec[plane];
}

DXGI_FORMAT TextureDesc::GetDxFormat(TextureType type, size_t plane){
	DXGI_FORMAT fmt;
	auto &vec = TextureDesc::TexDesc[(int)type];
	auto &planeTmp = vec[plane];

	auto finded = TextureDesc::TexFormatDxMap.find(planeTmp.Component);

	if (finded != TextureDesc::TexFormatDxMap.end()){
		fmt = finded->second;
	}
	else{
		fmt = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	}

	return fmt;
}