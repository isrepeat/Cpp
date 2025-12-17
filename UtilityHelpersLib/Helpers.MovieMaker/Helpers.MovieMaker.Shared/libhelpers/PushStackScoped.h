#pragma once
#include "Macros.h"

template<typename StackAccessorT>
class PushStackScoped {
public:
	template <typename... Args>
	PushStackScoped(StackAccessorT* stackAccessor, Args&&... args)
		: stackAccessor{ stackAccessor }
	{
		assert(stackAccessor);
		stackAccessor->Push(std::forward<Args&&>(args)...);
	}

	NO_COPY(PushStackScoped);

	PushStackScoped(PushStackScoped&& other)
		: stackAccessor(std::move(other.stackAccessor))
	{
		other.stackAccessor = nullptr;
	}

	PushStackScoped& operator=(PushStackScoped&& other) {
		if (this != &other) {
			this->stackAccessor = std::move(other.stackAccessor);
			other.stackAccessor = nullptr;
		}
		return *this;
	}

	~PushStackScoped() {
		if (stackAccessor) {
			stackAccessor->Pop();
		}
	}

private:
	StackAccessorT* stackAccessor;
};