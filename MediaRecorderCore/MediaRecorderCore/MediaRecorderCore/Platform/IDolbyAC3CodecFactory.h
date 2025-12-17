#pragma once

#include <cstdint>
#include <libhelpers/MediaFoundation/MFInclude.h>

class IDolbyAC3CodecFactory {
public:
	IDolbyAC3CodecFactory() {}
	virtual ~IDolbyAC3CodecFactory() {}

	virtual Microsoft::WRL::ComPtr<IMFTransform> CreateIMFTransform() = 0;
};
