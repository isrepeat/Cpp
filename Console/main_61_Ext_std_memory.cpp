#include <Windows.h>
#include <functional>
#include <algorithm>
#include <iostream>
#include <optional>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <regex>
#include <type_traits>

#include <Helpers/Logger.h>


//#define __LOG_NULLPTR_OBJECT(T) LOG_DEBUG_D("'{}' is nullptr", typeid(T).name())
#define __LOG_NULLPTR_OBJECT(T)

// Extends class with "Try()" method which can be used inside SAFE_RESULT macro
#define SafeObject(T) \
	T* Try() { \
		if (this == nullptr) { \
			__LOG_NULLPTR_OBJECT(T); \
			throw std::ex::bad_pointer(); \
		} \
		return this; \
	}


// This is an alternative to "using _MyBase::_MyBase"
#define PP_FORWARD_CTOR(From, To) \
	template <typename... _Args> \
	From(_Args&&... args) \
		: To(::std::forward<_Args&&>(args)...) \
	{}

namespace std {
	namespace ex {
		struct bad_pointer {};

		template <typename T>
		struct unique_ptr : public ::std::unique_ptr<T> {
			PP_FORWARD_CTOR(unique_ptr, ::std::unique_ptr<T>);

			unique_ptr<T>& Try() {
				if (this == nullptr) {
					throw std::ex::bad_pointer{};
				}
				if (this->get() == nullptr) {
					throw std::ex::bad_pointer{};
				}
				return *this;
			}
		};

		template <typename T>
		struct shared_ptr : public ::std::shared_ptr<T> {
			PP_FORWARD_CTOR(shared_ptr, ::std::shared_ptr<T>);

			shared_ptr<T>& Try() {
				if (this == nullptr) {
					throw std::ex::bad_pointer{};
				}
				if (this->get() == nullptr) {
					throw std::ex::bad_pointer{};
				}
				return *this;
			}
		};

		template <typename T>
		struct weak_ptr : public ::std::weak_ptr<T> {
			PP_FORWARD_CTOR(weak_ptr, ::std::weak_ptr<T>);

			shared_ptr<T> Try() {
				if (this == nullptr) {
					throw std::ex::bad_pointer{};
				}
				auto sharedPtr = this->lock();
				if (sharedPtr == nullptr) {
					throw std::ex::bad_pointer{};
				}
				return sharedPtr;
			}
		};

		// Also you can expose std methods to be available outside through this namespace.
		//using ::std::make_unique;
	}
}

namespace Helpers {
	struct ObjectA {
		SafeObject(ObjectA);

		bool MethodA() {
			return this->condition;
		}

	private:
		bool condition = true;
	};

	struct ObjectB {
		ObjectA* GetObjectA() {
			return nullptr;
			//return std::make_unique<ObjectA>();
		}
	};

	struct ObjectC {
		std::ex::weak_ptr<ObjectB> GetObjectB() {
			//return nullptr;
			return this->objB;
		}

	private:
		std::shared_ptr<ObjectB> objB = std::make_shared<ObjectB>();
	};

	struct ObjectD {
		std::ex::unique_ptr<ObjectC>& GetObjectC() {
			//return nullptr;
			return this->objC;
		}

	private:
		std::ex::unique_ptr<ObjectC> objC = std::make_unique<ObjectC>();
	};

	bool SafeConditionResult(::std::function<bool()> fnCondition) {
		try {
			return static_cast<bool>(fnCondition());
		}
		catch (std::ex::bad_pointer&) {
			return false;
		}
		catch (...) {
			return false;
		}
	}
}

#define SAFE_RESULT(condition) Helpers::SafeConditionResult([&] { return condition;})

int main() {
	std::ex::shared_ptr<Helpers::ObjectD> objectD = std::shared_ptr<Helpers::ObjectD>{new Helpers::ObjectD };

	if (!SAFE_RESULT(objectD->GetObjectC().Try()->GetObjectB().Try()->GetObjectA()->Try()->MethodA())) {
		NOOP;
	}
	return 0;
}