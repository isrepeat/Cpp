#pragma once
#include "Helpers/common.h"
#include <string_view>
#include <functional>
#include <memory>

#if defined(ENABLE_NULLPTR_LOGGING)
#include "Logger.h"
#define __LOG_NULLPTR_OBJECT(T) LOG_DEBUG_D("'{}' is nullptr", typeid(T).name())
#else
#define __LOG_NULLPTR_OBJECT(T)
#endif

// TODO: If this header included twice may be problems with 'STD_EXT_NS' macro. Fix it.
// Extends class with "Try()" method which can be used inside SAFE_RESULT macro
#define SafeObject(T) \
	T* Try() { \
		if (this == nullptr) { \
			__LOG_NULLPTR_OBJECT(T); \
			throw STD_EXT_NS::bad_pointer(); \
		} \
		return this; \
	}


namespace STD_EXT_NS {
	struct bad_pointer : ::std::exception {
		bad_pointer()
			: ::std::exception("bad pointer")
		{}
	};


	//
	// unique_ptr
	//
	template <typename T>
	struct unique_ptr : public ::std::unique_ptr<T> {
		using Base_t = ::std::unique_ptr<T>;
		using Base_t::unique_ptr; // NOTE: спец конструкторы не наследуются, определяем мосты для них ниже.

		//PP_FORWARD_CTOR(unique_ptr, ::std::unique_ptr<T>);
		unique_ptr(Base_t&& other) noexcept
			: Base_t{ ::std::move(other) } {
		}

		unique_ptr<T>& Try() {
			if (this == nullptr) {
				throw STD_EXT_NS::bad_pointer{};
			}
			if (this->get() == nullptr) {
				throw STD_EXT_NS::bad_pointer{};
			}
			return *this;
		}

        // Проверка типа без изменения владения
        template <typename U>
        bool Is() const {
            return dynamic_cast<U*>(this->get()) != nullptr;
        }

        // Наблюдающий "view" без изменения владения (иногда удобно)
        template <typename U>
        U* AsView() const {
            return dynamic_cast<U*>(this->get());
        }

        // Безопасный up/downcast с передачей владения (успешно -> новый unique_ptr<U>, иначе -> nullptr)
        // Требует rvalue, чтобы явно показать перемещение владения.
        template <typename U>
        unique_ptr<U> As()&& {
            T* const rawPtr = this->get();
            if (U* const casted = dynamic_cast<U*>(rawPtr)) {
                (void)this->release(); // отдать владение только после успешного каста
                return unique_ptr<U>{ casted };
            }
            return {};
        }

        // Жёсткий вариант: если каст невозможен — бросаем std::bad_cast
        template <typename U>
        unique_ptr<U> Cast()&& {
            T* const rawPtr = this->get();
            if (U* const casted = dynamic_cast<U*>(rawPtr)) {
                (void)this->release();
                return unique_ptr<U>{ casted };
            }
            throw ::std::bad_cast{};
        }
	};

    // Обёртка над std::make_unique, возвращающая std::ex::unique_ptr<T>
    template <typename T, typename... Args>
    unique_ptr<T> make_unique_ex(Args&&... args) {
        return unique_ptr<T>(::std::make_unique<T>(::std::forward<Args>(args)...));
    }


	//
	// shared_ptr
	//
	template <typename T>
	struct shared_ptr : public ::std::shared_ptr<T> {
		using Base_t = ::std::shared_ptr<T>;
		using Base_t::shared_ptr;

		//PP_FORWARD_CTOR(shared_ptr, ::std::shared_ptr<T>);
		shared_ptr(const Base_t& other)
			: Base_t{ other } {
		}

		shared_ptr(Base_t&& other) noexcept
			: Base_t{ ::std::move(other) } {
		}

		shared_ptr<T>& Try() {
			if (this->get() == nullptr) {
				throw STD_EXT_NS::bad_pointer{};
			}
			return *this;
		}

		template <typename U>
		bool Is() const {
			return ::std::dynamic_pointer_cast<U>(*this) != nullptr;
		}

		template <typename U>
		shared_ptr<U> As() const {
			auto casted = ::std::dynamic_pointer_cast<U>(*this);
			return casted;
		}


		template <typename U>
		shared_ptr<U> Cast() const {
			auto casted = ::std::dynamic_pointer_cast<U>(*this);
			if (casted == nullptr) {
				throw ::std::bad_cast{};
			}
			return casted;
		}
	};

    // Обёртка над std::make_shared, возвращающая std::ex::shared_ptr<T>
    template <typename T, typename... Args>
    shared_ptr<T> make_shared_ex(Args&&... args) {
        return shared_ptr<T>(::std::make_shared<T>(::std::forward<Args>(args)...));
    }


	//
	// weak_ptr
	//
	template <typename T>
	struct weak_ptr : public ::std::weak_ptr<T> {
		using Base_t = ::std::weak_ptr<T>;
		using Base_t::weak_ptr;

		//PP_FORWARD_CTOR(weak_ptr, ::std::weak_ptr<T>);
		weak_ptr(const Base_t& other)
			: Base_t{ other } {
		}

		weak_ptr(Base_t&& other) noexcept
			: Base_t{ ::std::move(other) } {
		}

		// override to retrun extensible struct
		shared_ptr<T> lock() const {
			return ::std::weak_ptr<T>::lock();
		}

		shared_ptr<T> Try() {
			if (this == nullptr) {
				throw STD_EXT_NS::bad_pointer{};
			}
			auto sharedPtr = this->lock();
			if (sharedPtr == nullptr) {
				throw STD_EXT_NS::bad_pointer{};
			}
			return sharedPtr;
		}
	};
} // namespace STD_EXT_NS


namespace HELPERS_NS {
	inline bool SafeConditionResult(std::function<bool()> fnCondition) {
		try {
			return static_cast<bool>(fnCondition());
		}
		catch (...) {
			return false;
		}
	}

#define SAFE_RESULT(condition) HELPERS_NS::SafeConditionResult([&] { return condition;})
}