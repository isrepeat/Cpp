#pragma once
#include "Action.h"
#include "Logger.h"

namespace {
	struct CompleteException {};
}

namespace HELPERS_NS {
	PerformActionWithAttempts::PerformActionWithAttempts(int attempts, std::function<void()> actionCallback) {
		try {
			while (attempts-- > 0) {
				LOG_DEBUG_D("PerformAction, attempts left = {}", attempts);
				actionCallback(); // may throw CompleteException
			}
		}
		catch (const CompleteException&) {
			LOG_DEBUG_D("Action completed");
		}
		catch (std::exception& ex) {
			LOG_ERROR_D("Catch std::exception = {}", ex.what());
		}
		catch (...) {
			LOG_DEBUG_D("Catch unhandled exception");
		}
	}

	void PerformActionWithAttempts::Break() {
		throw CompleteException{};
	}
}