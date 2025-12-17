#pragma once

#include <cstdint>
#include <libhelpers/MediaFoundation/MFInclude.h>

class IAacCodecFactory {
public:
	IAacCodecFactory() {}
	virtual ~IAacCodecFactory() {}

	virtual Microsoft::WRL::ComPtr<IMFTransform> CreateIMFTransform() = 0;
};
