#pragma once
#include "Platform.h"
#ifdef __WINDOWS_OS__
#include "../Shared/ProtobufTypes/GeneratedFiles/MessageWrapper.pb.h"
#else
#include "MessageWrapper.pb.h"
#endif
#include "UserAuthMessage.h"

class ProtobufSerializer {
public:
	std::string SerializeIpPort(const UserAddress& userAddress);
	UserAuthMessage DeserializeUserAuthMessage(const std::string& message);

private:
	std::mutex deserializeMesssageMutex;
	MessageWrapper deserializedMessage;
	std::mutex serializeMesssageMutex;
	MessageWrapper serializedMessage;
};

