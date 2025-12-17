#pragma once
#include "common.h"
#include "IEvent.h"

#include <memory>

namespace HELPERS_NS {
	using IWeakEventToken = std::weak_ptr<void>;

	template<typename ...Args>
	using IWeakEvent = IEvent<IWeakEventToken, Args...>;
}
