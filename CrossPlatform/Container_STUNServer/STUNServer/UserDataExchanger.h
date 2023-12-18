#pragma once
#include "Platform.h"
#include <string>
#include "ActiveUser.h"
#include <functional>
#include <mutex>
#include <unordered_map>


class UserDataExchanger {
public:
	void AddNewUser(std::uint64_t userId, ActiveUser userAddress);
	void DeleteUser(std::uint64_t userId);
	void SetUserData(std::uint64_t userId, std::uint64_t userServerId, UserAddress localAddress);
	void FetchUser(std::uint64_t requesterId, std::uint64_t requestedUserServerId, std::function<void(const UserAddress&)>&& onDataReadyHandler);
	
	//void DeleteUserByGlobalAddress(const UserAddress& globalAddress);

private:
	std::mutex usersMutex;
	std::unordered_map<std::uint64_t, ActiveUser> activeUsers;

	std::mutex callbacksMutex;
	std::unordered_map<std::uint64_t, std::function<void(const UserAddress&)>> fetchCallbacks;
	
};

