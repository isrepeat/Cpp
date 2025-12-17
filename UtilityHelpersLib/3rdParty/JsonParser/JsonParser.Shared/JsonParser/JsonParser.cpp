#include "JsonParser.h"
#include <cassert>


namespace JS {
	bool LoggerCallback::Register(std::function<void(std::string)> callback) {
		auto& _this = GetInstance();
		if (_this.loggerCallback) {
			_this.loggerCallback("WARNING: Logger callback already registered");
			return false;
		}
		_this.loggerCallback = callback;
		return true;
	}

	void LoggerCallback::SafeInvoke(const std::string& msg) {
		auto& _this = GetInstance();
		if (_this.loggerCallback) {
			_this.loggerCallback(msg);
		}
		else {
			assert(false && " --> loggerCallback is empty");
		}
	}
}