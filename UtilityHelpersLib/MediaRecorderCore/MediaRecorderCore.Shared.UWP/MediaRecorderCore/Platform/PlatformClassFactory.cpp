#include "pch.h"
#include "Platform/PlatformClassFactory.h"
#include "Wma8CodecFactory.h"
#include "MP3CodecFactory.h"
#include "AmrNbCodecFactory.h"
#include "EncoderEnumerator.h"
#include "AacCodecFactory.h"
#include "DolbyAC3CodecFactory.h"
#include "AlacCodecFactory.h"
#include "FlacCodecFactory.h"

#include <libhelpers/MediaFoundation/MFUser.h>

PlatformClassFactory *PlatformClassFactory::Instance() {
    static PlatformClassFactory instance;
    return &instance;
}

std::unique_ptr<IWma8CodecFactory> PlatformClassFactory::CreateWma8CodecFactory() {
    return std::make_unique<Wma8CodecFactory>();
}

std::unique_ptr<IMP3CodecFactory> PlatformClassFactory::CreateMP3CodecFactory() {
    return std::make_unique<MP3CodecFactory>();
}

std::unique_ptr<IAmrNbCodecFactory> PlatformClassFactory::CreateAmrNbCodecFactory() {
    return std::make_unique<AmrNbCodecFactory>();
}

std::unique_ptr<IEncoderEnumerator> PlatformClassFactory::CreateEncoderEnumerator() {
    return std::make_unique<EncoderEnumerator>();
}

std::unique_ptr<IAacCodecFactory> PlatformClassFactory::CreateAacCodecFactory() {
	return std::make_unique<AacCodecFactory>();
}

std::unique_ptr<IAlacCodecFactory> PlatformClassFactory::CreateAlacCodecFactory() {
	return std::make_unique<AlacCodecFactory>();
}

std::unique_ptr<IFlacCodecFactory> PlatformClassFactory::CreateFlacCodecFactory() {
	return std::make_unique<FlacCodecFactory>();
}

std::unique_ptr<IDolbyAC3CodecFactory> PlatformClassFactory::CreateDolbyAC3CodecFactory() {
	return std::make_unique<DolbyAC3CodecFactory>();
}

PlatformClassFactory::PlatformClassFactory() {
    MFUser mfuser;
}

PlatformClassFactory::~PlatformClassFactory() {
}