#pragma once

// using this version in order to copy only references
template<class Proto, class ParentData>
struct ArgsTemplate {
	const Proto &proto;
	ParentData &parentData;

	ArgsTemplate(const Proto &proto, ParentData &parentData)
		: proto(proto), parentData(parentData)
	{
	}

	template<class Proto2, class ParentData2>
	ArgsTemplate(const ArgsTemplate<Proto2, ParentData2> &other)
		: proto(other.proto), parentData(other.parentData)
	{
	}
};