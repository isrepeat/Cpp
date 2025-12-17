#pragma once
#include "Helpers/common.h"
#include <functional>

namespace HELPERS_NS {
	namespace Event {
		class ISubscriptionToken {
		public:
			virtual ~ISubscriptionToken() = default;
		};

		template<typename TSignature>
		class IEvent;

		template<typename... TArgs>
		class IEvent<void(TArgs...)> {
		public:
			//using Token_t = std::shared_ptr<ISubscriptionToken>;
			using HandlerFunc_t = std::function<void(TArgs...)>;

			virtual ~IEvent() = default;

			virtual std::shared_ptr<ISubscriptionToken> Subscribe(const HandlerFunc_t& handlerFunction) = 0;
			virtual void Unsubscribe(std::shared_ptr<ISubscriptionToken> token) = 0;
			virtual void Invoke(const TArgs&... args) = 0;
		};
	}
}