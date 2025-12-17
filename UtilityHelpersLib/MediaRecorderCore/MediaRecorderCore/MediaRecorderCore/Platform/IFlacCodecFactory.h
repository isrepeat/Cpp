#pragma once

#include <cstdint>
#include <libhelpers/MediaFoundation/MFInclude.h>

class IFlacCodecFactory {
public:
	IFlacCodecFactory() {}
	virtual ~IFlacCodecFactory() {}

	virtual Microsoft::WRL::ComPtr<IMFTransform> CreateIMFTransform() = 0;
};

