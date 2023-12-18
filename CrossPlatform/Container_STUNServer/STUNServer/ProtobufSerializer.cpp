#include "ProtobufSerializer.h"
#ifdef __WINDOWS_OS__
#include "../Shared/ProtobufTypes/GeneratedFiles/ClientSTUNServerAuthMessage.pb.h"
#include "../Shared/ProtobufTypes/GeneratedFiles/STUNServerMessage.pb.h"
#else
#include "ClientSTUNServerAuthMessage.pb.h"
#include "STUNServerMessage.pb.h"
#endif
#include "Logger.h"

std::string ProtobufSerializer::SerializeIpPort(const UserAddress& userAddress) {
    auto lock = std::lock_guard{ serializeMesssageMutex };

    serializedMessage.Clear();

    auto stunServerResponse = new STUNServerMessage{};
    stunServerResponse->set_ip(userAddress.ip);
    stunServerResponse->set_port(userAddress.port);
    stunServerResponse->set_is_same_network(userAddress.sameNetworkWithPartner);
    serializedMessage.set_allocated_stun_server_message(stunServerResponse);
    serializedMessage.set_type(MessageType::STUNServerResponse);

    return serializedMessage.SerializeAsString();
}

UserAuthMessage ProtobufSerializer::DeserializeUserAuthMessage(const std::string& message) {

    auto lock = std::lock_guard{ deserializeMesssageMutex };
    deserializedMessage.Clear();
    if (!this->deserializedMessage.ParseFromString(message) && deserializedMessage.type() != MessageType::STUNServerAuth) {
        WriteError("ProtobufSerializer DeserializeUserAuthMessage can't parse from string %s\n", message.c_str());
        return UserAuthMessage{};
    }

    auto protobufUserAuthData = deserializedMessage.client_stun_server_auth_message();
    
    auto userAuthData = UserAuthMessage{};
    userAuthData.localAddress.ip = protobufUserAuthData.local_ip();
    userAuthData.localAddress.port = protobufUserAuthData.local_port();
    userAuthData.jwt = protobufUserAuthData.jwt();
    userAuthData.requestedUserServerId = protobufUserAuthData.requested_user_id();
    userAuthData.requesterUserServerId = protobufUserAuthData.requester_id();

    return userAuthData;
}