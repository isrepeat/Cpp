#pragma once
#include "Helpers/common.h"
#include "Helpers/Meta/Concepts.h"
#include <type_traits>
#include <functional>
#include <optional>
#include <utility>

namespace STD_EXT_NS {
	template <typename T>
	class optional_ref {
	public:
		using element_type = T;

	public:
		constexpr optional_ref() noexcept
			: ptr{ nullptr } {
		}

		constexpr optional_ref(std::nullopt_t) noexcept
			: ptr{ nullptr } {
		}

		constexpr optional_ref(T& ref) noexcept
			: ptr{ ::std::addressof(ref) } {
		}

		constexpr optional_ref(::std::reference_wrapper<T> refw) noexcept
			: ptr{ ::std::addressof(refw.get()) } {
		}

		template <typename U>
		__requires_expr(
			::std::is_convertible_v<U*, T*>
		) constexpr optional_ref(optional_ref<U> other) noexcept
			: ptr{ other.get() } {
		}


		constexpr optional_ref& operator=(std::nullopt_t) noexcept {
			this->ptr = nullptr;
			return *this;
		}

		constexpr optional_ref& operator=(T& ref) noexcept {
			this->ptr = ::std::addressof(ref);
			return *this;
		}

		constexpr optional_ref& operator=(::std::reference_wrapper<T> refw) noexcept {
			this->ptr = ::std::addressof(refw.get());
			return *this;
		}

		template <typename U>
		__requires_expr(
			::std::is_convertible_v<U*, T*>
		) constexpr optional_ref& operator=(optional_ref<U> other) noexcept {
			this->ptr = other.get();
			return *this;
		}


		constexpr bool has_value() const {
			return this->ptr != nullptr;
		}

		constexpr explicit operator bool() const {
			return this->has_value();
		}

		constexpr T& value() const {
			if (this->ptr == nullptr) {
				throw ::std::bad_optional_access{};
			}
			else {
				return *(this->ptr);
			}
		}

		constexpr T& value_or(T& default_ref) const {
			if (this->ptr != nullptr) {
				return *(this->ptr);
			}
			else {
				return default_ref;
			}
		}

		constexpr T* get() const {
			return this->ptr;
		}

		constexpr T& operator*() const {
			return *(this->ptr);
		}

		constexpr T* operator->() const {
			return this->ptr;
		}

		constexpr ::std::reference_wrapper<T> as_ref() const {
			return ::std::ref(this->value());
		}

		constexpr void reset() {
			this->ptr = nullptr;
		}

	public:
		friend constexpr bool operator==(optional_ref lhs, std::nullopt_t) noexcept {
			return lhs.ptr == nullptr;
		}

		friend constexpr bool operator==(std::nullopt_t, optional_ref rhs) noexcept {
			return rhs.ptr == nullptr;
		}

		friend constexpr bool operator!=(optional_ref lhs, std::nullopt_t) noexcept {
			return lhs.ptr != nullptr;
		}

		friend constexpr bool operator!=(std::nullopt_t, optional_ref rhs) noexcept {
			return rhs.ptr != nullptr;
		}

		friend constexpr bool operator==(optional_ref lhs, optional_ref rhs) noexcept {
			return lhs.ptr == rhs.ptr;
		}

		friend constexpr bool operator!=(optional_ref lhs, optional_ref rhs) noexcept {
			return lhs.ptr != rhs.ptr;
		}

	private:
		T* ptr;
	};


	// Разрешаем только lvalue:
	template <typename T>
	constexpr optional_ref<T> make_optional_ref(T& ref) noexcept {
		return optional_ref<T>{ ref };
	}

	template <typename T>
	constexpr optional_ref<const T> make_optional_ref(const T& ref) noexcept {
		return optional_ref<const T>{ ref };
	}

	// Явно запрещаем rvalue:
	template <typename T>
	constexpr optional_ref<T> make_optional_ref(T&&) = delete;

	template <typename T>
	constexpr optional_ref<const T> make_optional_ref(const T&&) = delete;
}