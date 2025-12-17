#pragma once
#include "IWma8CodecFactory.h"
#include "IMP3CodecFactory.h"
#include "IAmrNbCodecFactory.h"
#include "IAacCodecFactory.h"
#include "IEncoderEnumerator.h"
#include "IDolbyAC3CodecFactory.h"
#include "IAlacCodecFactory.h"
#include "IFlacCodecFactory.h"

#include <memory>

/*
Implementation must be platform-dependent. This can be implemented with preprocessor flags or
with .Shared project that is used by platform dependent projects.
*/
class PlatformClassFactory {
public:
    static PlatformClassFactory *Instance();

    std::unique_ptr<IWma8CodecFactory> CreateWma8CodecFactory();
    std::unique_ptr<IMP3CodecFactory> CreateMP3CodecFactory();
    std::unique_ptr<IAmrNbCodecFactory> CreateAmrNbCodecFactory();
	std::unique_ptr<IAacCodecFactory> CreateAacCodecFactory();
	std::unique_ptr<IDolbyAC3CodecFactory> CreateDolbyAC3CodecFactory();
    std::unique_ptr<IEncoderEnumerator> CreateEncoderEnumerator();
	std::unique_ptr<IAlacCodecFactory> CreateAlacCodecFactory();
	std::unique_ptr<IFlacCodecFactory> CreateFlacCodecFactory();

private:
    PlatformClassFactory();
    ~PlatformClassFactory();
};