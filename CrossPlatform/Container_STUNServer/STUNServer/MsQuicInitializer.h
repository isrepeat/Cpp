#pragma once
#include "Platform.h"
#ifdef __WINDOWS_OS__
#include <msquic.hpp>
#else
#include <msquic.h>
#endif
#include <memory>

// TODO return unique pointers
class MsQuicInitializer {
public:
	MsQuicInitializer(const MsQuicInitializer&) = delete;
	MsQuicInitializer(MsQuicInitializer&&) = delete;

	static const MsQuicInitializer& GetInstance();

	const QUIC_API_TABLE& GetQuicApiTable() const;
	const HQUIC& GetRegistration() const;

private:
	MsQuicInitializer();


private:

	//std::unique_ptr<QUIC_API_TABLE, QuicApiTableDeleter> msQuic;
	//std::unique_ptr<HQUIC, QuicRegistrationDeleter> registration;	
	const QUIC_API_TABLE* msQuic;
	HQUIC registration;
};

