#pragma once
#include <mutex>
#include <string>
#include <vector>
#include <functional>
#include "FunctionMt.h"

// TODO; replace std::string to std::wstring
struct ServerCallbacks {
	H::FunctionMt<void()> connected = std::function([] {});
	H::FunctionMt<void()> adaptersChanged = std::function([] {});
	H::FunctionMt<bool()> capturerChangingConfirmation = std::function([] { return false; });
	H::FunctionMt<void(int)> frameData = std::function([](int) {});

	ServerCallbacks() {
		Sleep(1);
	}

	~ServerCallbacks() {
		Sleep(1);
	}

	//// TODO: rewrite this logic when we set token/cv manually every instance ...
	//// TODO: mb use variant to set either token or cv?

	//// Set caller token to skip operator() when token expired
	//void SetToken(std::shared_ptr<int>& callerToken) {
	//	//connected.SetToken(callerToken);
	//	//adaptersChanged.SetToken(callerToken);
	//	//capturerChangingConfirmation.SetToken(callerToken);
	//	useTokenGuard = true;
	//}

	//// Set caller cv to nitify caller when operaotr() finished
	//void SetCv(std::shared_ptr<std::condition_variable>& callerCv) {
	//	//connected.SetCv(callerCv);
	//	//adaptersChanged.SetCv(callerCv);
	//	//capturerChangingConfirmation.SetCv(callerCv);
	//	useCvGuard = true;
	//}

	//bool IsUseTokenGuard() {
	//	return useTokenGuard;
	//}

	//bool IsUseCvGuard() {
	//	return useCvGuard;
	//}
//
//private:
//	std::atomic<bool> useTokenGuard = false;
//	std::atomic<bool> useCvGuard = false;
};
