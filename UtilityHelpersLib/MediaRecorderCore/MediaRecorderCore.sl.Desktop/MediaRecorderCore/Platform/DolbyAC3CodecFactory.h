#pragma once
#include "Platform/IDolbyAC3CodecFactory.h"

class DolbyAC3CodecFactory : public IDolbyAC3CodecFactory {
public:
	Microsoft::WRL::ComPtr<IMFTransform> CreateIMFTransform() override;
};
