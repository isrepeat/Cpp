#pragma once
#include "common.h"
#include <type_traits>
#include <functional>
#include <utility>
#include <memory>

namespace HELPERS_NS {
	//
	// move_lambda, movable_function (https://gist.github.com/jbandela/4208898)
	//
	template<class T>
	using moved_value = std::reference_wrapper<T>;

	template<class T, class F>
	struct move_lambda {
	private:
		T val;
		F fn;
	public:
		move_lambda(T&& v, F f)
			: val(std::move(v))
			, fn(f)
		{}
		move_lambda(move_lambda&& other) = default;
		move_lambda& operator=(move_lambda&& other) = default;

		template<class... Args>
		auto operator()(Args&& ...args) -> decltype(this->fn(moved_value<T>(this->val), std::forward<Args>(args)...)) {
			moved_value<T> mv(val);
			return fn(mv, std::forward<Args>(args)...);
		}

		move_lambda() = delete;
		move_lambda(const move_lambda&) = delete;
		move_lambda& operator=(const move_lambda&) = delete;
	};

	template<class T, class F>
	move_lambda<T, F> create_move_lambda(T&& t, F f) {
		return move_lambda<T, F>(std::move(t), f);
	}

	// Unfortunately, std::function does not seem to support move-only callables
	// See § 20.8.11.2.1 point 7 where it requires F be CopyConstructible 
	// From draft at http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2011/n3242.pdf
	// Here is our movable replacement for std::function
	template< class ReturnType, class... ParamTypes>
	struct movable_function_base {
		virtual ReturnType callFunc(ParamTypes&&... p) = 0;
		virtual ~movable_function_base() = default;
	};


	template<class F, class ReturnType, class... ParamTypes>
	struct movable_function_imp : public movable_function_base<ReturnType, ParamTypes...> {
		F fn;
		virtual ReturnType callFunc(ParamTypes&&... p) {
			return fn(std::forward<ParamTypes>(p)...);
		}
		explicit movable_function_imp(F&& f) 
			: fn(std::move(f)) 
		{};

		movable_function_imp() = delete;
		movable_function_imp(const movable_function_imp&) = delete;
		movable_function_imp& operator=(const movable_function_imp&) = delete;
	};


	template<class FuncType>
	struct movable_function {};

	template<class ReturnType, class... ParamTypes>
	struct movable_function<ReturnType(ParamTypes...)> {
		std::unique_ptr<movable_function_base<ReturnType, ParamTypes...>> ptr;

		template<class F>
		/*explicit*/ movable_function(F&& f)
			: ptr(std::make_unique<movable_function_imp<F, ReturnType, ParamTypes...>>(std::move(f)))
		{}

		movable_function(movable_function&& other) = default;
		movable_function& operator=(movable_function&& other) = default;

		ReturnType operator()(ParamTypes&& ...args) {
			return ptr->callFunc(std::forward<ParamTypes>(args)...);
		}

		explicit operator bool() const {
			return static_cast<bool>(this->ptr);
		}

		movable_function() = default; // delete;
		movable_function(const movable_function&) = delete;
		movable_function& operator=(const movable_function&) = delete;
	};
}