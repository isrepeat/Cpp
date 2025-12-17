#pragma once
#include "common.h"
#include <functional>

template<typename ...Args>
class IEvent {
public:
	typedef std::function<void(Args...)> callback;

	virtual ~IEvent() = default;

	virtual std::shared_ptr<callback> Add(callback func) = 0;
};

namespace HELPERS_NS {
	// TokenT may be weak_ptr<void>, or some UUID
	// TokenT used to allow manual unsubscription(Remove) from event
	// Auto unsubscription depends on TokenT
	// Implementations(or derived interfaces) of IEvent may define their TokenT to allow auto unsubscription
	// Auto unsubscription may be implemented in destructor of TokenT
	template<typename TokenT, typename ...Args>
	class IEvent {
	public:
		using Token = TokenT;
		using Handler = std::function<void(Args...)>;

		virtual ~IEvent() = default;

		virtual void Subscribe(Handler handler, Token token) = 0;
		virtual void Unsubscribe(Token token) = 0;
	};
}
