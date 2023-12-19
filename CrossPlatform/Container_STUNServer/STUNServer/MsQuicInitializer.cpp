#include "MsQuicInitializer.h"
#include "System.h"


const MsQuicInitializer& MsQuicInitializer::GetInstance() {
    static auto instance = MsQuicInitializer{};
    return instance;
}

const QUIC_API_TABLE& MsQuicInitializer::GetQuicApiTable() const{
    return *msQuic;

}

const HQUIC& MsQuicInitializer::GetRegistration() const{
    return registration;
}

MsQuicInitializer::MsQuicInitializer() {
    auto hr = MsQuicOpen(const_cast<const QUIC_API_TABLE**>(&msQuic));
    H::System::ThrowIfFailed(hr);

    hr = msQuic->RegistrationOpen(nullptr, &registration);
    H::System::ThrowIfFailed(hr);
}