#pragma once

#include <cstdint>
#include <libhelpers/MediaFoundation/MFInclude.h>

class IAlacCodecFactory {
public:
	IAlacCodecFactory() {}
	virtual ~IAlacCodecFactory() {}

	virtual Microsoft::WRL::ComPtr<IMFTransform> CreateIMFTransform() = 0;
};
