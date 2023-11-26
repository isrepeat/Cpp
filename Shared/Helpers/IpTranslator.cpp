//#include "pch.h"
#define WIN32_LEAN_AND_MEAN 
#include "IpTranslator.h"
#include <Windows.h>
#include <ws2def.h>
#include <ws2tcpip.h>
#include <stdexcept>
#include <comdef.h>
#include <mstcpip.h>
#include <socketapi.h>


std::string IpTranslator::Ipv4ToIpv6(const std::string& ipv4) {
    auto hints = ADDRINFOA{ 0 };
    hints.ai_flags = AI_NUMERICHOST;
    auto addressInfo = PADDRINFOA{ 0 };
    auto hr = GetAddrInfoA(ipv4.c_str(), nullptr, &hints, &addressInfo);

    if(FAILED(hr))
        throw std::runtime_error(std::string{ reinterpret_cast<const char*>(_com_error{ hr }.ErrorMessage()) });

    auto addressIpv6 = std::string{};
    addressIpv6.resize(INET6_ADDRSTRLEN);

    if (addressInfo->ai_family == AF_INET) {
        auto ipv6Addr = SOCKADDR_IN6{ 0 };
        IN6ADDR_SETV4MAPPED(&ipv6Addr, &reinterpret_cast<sockaddr_in*>(addressInfo->ai_addr)->sin_addr, SCOPE_ID { 0 }, 54644);

        inet_ntop(AF_INET6, &ipv6Addr.sin6_addr, const_cast<char*>(addressIpv6.c_str()), INET6_ADDRSTRLEN);
    }
    else {
        addressIpv6 = ipv4;
    }
    return addressIpv6;
}
