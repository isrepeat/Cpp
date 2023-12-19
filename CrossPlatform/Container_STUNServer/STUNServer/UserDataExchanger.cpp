#include "UserDataExchanger.h"
#include "Logger.h"

void UserDataExchanger::AddNewUser(std::uint64_t userId, ActiveUser userAddress) {
	auto lock = std::lock_guard{ usersMutex };
	activeUsers[userId] = userAddress;
	WriteDebug("UserDataExchanger::AddNewUser added user with id %u, global ip: %s:%p", userId,
		userAddress.globalAddress.ip.c_str(), userAddress.globalAddress.port);
	WriteDebug("UserDataExchanger::AddNewUser Current users number to exchange: %d", activeUsers.size());
}

void UserDataExchanger::DeleteUser(std::uint64_t userId) {
	{
		auto lock = std::lock_guard{ usersMutex };
		//activeUsers.erase(std::remove(activeUsers.begin(), activeUsers.end(), userId), activeUsers.end());
		activeUsers.erase(userId);
	}
	
	{
		auto lockCallbacks = std::lock_guard{ callbacksMutex };
		fetchCallbacks.erase(userId);
	}
	WriteDebug("UserDataExchanger::DeleteUser deleted user with id %u", userId);
	WriteDebug("UserDataExchanger::DeleteUser Current users number to exchange: %d", activeUsers.size());
}

//void UserDataExchanger::DeleteUserByGlobalAddress(const UserAddress& globalAddress) {
//
//	decltype(activeUsers)::key_type id;
//	{
//		auto lock = std::lock_guard{ usersMutex };
//		const auto& requestedUserData = std::find_if(activeUsers.begin(), activeUsers.end(), [globalAddress](const auto& keyValue) {
//			return keyValue.second.globalAddress.port == globalAddress.port && keyValue.second.globalAddress.ip == globalAddress.ip;
//			});
//		if (requestedUserData == activeUsers.end())
//			return;
//
//		id = requestedUserData->first;
//		activeUsers.erase(id);
//	}
//
//	{
//		auto lockCallbacks = std::lock_guard{ callbacksMutex };
//		fetchCallbacks.erase(id);
//	}
//}

void UserDataExchanger::SetUserData(std::uint64_t userId, std::uint64_t userServerId, UserAddress localAddress) {
	auto lock = std::lock_guard{ usersMutex };
	activeUsers[userId].idOnServer = userServerId;
	activeUsers[userId].localAddress = localAddress;
	WriteDebug("UserDataExchanger::SetUserData update data for user with id %u: id on server - %u, local address:%s:%p",
		userId, userServerId, localAddress.ip.c_str(), localAddress.port);
}

void UserDataExchanger::FetchUser(std::uint64_t requesterId, std::uint64_t requestedUserServerId, std::function<void(const UserAddress&)>&& onDataReadyHandler) {
	//auto lock = std::lock_guard{ callbacksMutex };
	//auto isOtherSideWaits = fetchCallbacks.contains()
	WriteDebug("UserDataExchanger::FetchUser Start fetching user with server id %u for user with internal id %u(server id %u)", requestedUserServerId, requesterId, activeUsers[requesterId].idOnServer);
	auto lock = std::lock_guard{ usersMutex };
	auto callbacksLock = std::lock_guard{ callbacksMutex };

	const auto& requestedUserData = std::find_if(activeUsers.begin(), activeUsers.end(), [requestedUserServerId](const auto& keyValue) {
		return keyValue.second.idOnServer == requestedUserServerId;
		});
	
	//if (requestedUserData == activeUsers.end())
	//	WriteDebug("UserDataExchanger::FetchUser no active user");

#ifdef __WINDOWS_OS__
	if (requestedUserData == activeUsers.end() || !fetchCallbacks.contains(requestedUserData->first)) {
#else
	//if(fetchCallbacks.find(requestedUserData->first)== fetchCallbacks.end())
	//	printf("no callback from %i\n", requestedUserData->first);
	if (requestedUserData == activeUsers.end() || fetchCallbacks.find(requestedUserData->first) == fetchCallbacks.end()) {

		if (requestedUserData != activeUsers.end() && fetchCallbacks.find(requestedUserData->first) == fetchCallbacks.end())
			WriteDebug("UserDataExchanger::FetchUser No callback from %u", requestedUserData->first);
#endif
		fetchCallbacks[requesterId] = std::move(onDataReadyHandler);
		WriteDebug("UserDataExchanger::FetchUser Requested user %u not connected", requestedUserServerId);
		return;
	}

	auto isSameNetwork = requestedUserData->second.globalAddress.ip == activeUsers[requesterId].globalAddress.ip;

	WriteDebug("UserDataExchanger::FetchUser Sending to user %u with ip: %s:%d data about user %u with ip: %s:%d",
		requestedUserData->first,
		isSameNetwork ? requestedUserData->second.localAddress.ip.c_str() : requestedUserData->second.globalAddress.ip.c_str(),
		isSameNetwork ? requestedUserData->second.localAddress.port : requestedUserData->second.globalAddress.port,
		requesterId,
		isSameNetwork ? activeUsers[requesterId].localAddress.ip.c_str() : activeUsers[requesterId].globalAddress.ip.c_str(),
		isSameNetwork ? activeUsers[requesterId].localAddress.port : activeUsers[requesterId].globalAddress.port);

	fetchCallbacks[requestedUserData->first](isSameNetwork ? activeUsers[requesterId].localAddress : activeUsers[requesterId].globalAddress);
	fetchCallbacks.erase(requestedUserData->first);
	WriteDebug("UserDataExchanger::FetchUser erased fetch callback for %u, callbacks left: %u", requestedUserData->first,
		fetchCallbacks.size());

	WriteDebug("UserDataExchanger::FetchUser Sending to user %u with ip: %s:%d data about user %u with ip: %s:%d",
		requesterId,
		isSameNetwork ? activeUsers[requesterId].localAddress.ip.c_str() : activeUsers[requesterId].globalAddress.ip.c_str(),
		isSameNetwork ? activeUsers[requesterId].localAddress.port : activeUsers[requesterId].globalAddress.port,
		requestedUserData->first,
		isSameNetwork ? requestedUserData->second.localAddress.ip.c_str() : requestedUserData->second.globalAddress.ip.c_str(),
		isSameNetwork ? requestedUserData->second.localAddress.port : requestedUserData->second.globalAddress.port);
	onDataReadyHandler(isSameNetwork ? requestedUserData->second.localAddress : requestedUserData->second.globalAddress);

	WriteDebug("UserDataExchanger::FetchUser fetching user %u(server id %u) for %u(server id %u) done",requestedUserData->first,
		requestedUserServerId, requesterId, activeUsers[requesterId].idOnServer);
}
