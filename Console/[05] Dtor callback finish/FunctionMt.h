#pragma once
#include <Helpers/Scope.h>
#include <Helpers/Utils.h>
#include <functional>
#include <cassert>


namespace H {
	// Wrapper under std::function with token/cv guard for multithreading
	template<typename Ret, typename ...Args>
	class FunctionMt_impl {
	public:
		using FnType = std::function<Ret(Args...)>;

		FunctionMt_impl() = default;
		FunctionMt_impl(std::function<Ret(Args...)> callback)
			: callback{ callback }
		{}

		FunctionMt_impl(const FunctionMt_impl& other) {
			std::unique_lock<std::mutex> lock_other{ other.mx };

			callback = other.callback;
		}

		FunctionMt_impl& operator=(const FunctionMt_impl& other) {
			if (&other != this) {
				// lock both objects
				std::unique_lock<std::mutex> lock_this(mx, std::defer_lock);
				std::unique_lock<std::mutex> lock_other(other.mx, std::defer_lock);

				// ensure no deadlock
				std::lock(lock_this, lock_other);

				// safely copy the data
				callback = other.callback;
			}
			return *this;
		}

	public:
		//void SetToken(std::shared_ptr<int> callerToken) {
		//	this->callerToken = callerToken;
		//}

		//void SetCv(std::shared_ptr<std::condition_variable> callerCv) {
		//	this->callerCv = callerCv;
		//}
		std::scoped_lock<std::mutex> ScopedLock() {
			return std::scoped_lock<std::mutex> { mx };
		}


		Ret operator() (Args... args) const {
			std::unique_lock<std::mutex> lk{ mx };
			//// guard based on token
			//if (IsWeakPtrInitialized(callerToken)) {
			//	if (callerToken.expired())
			//		return Ret();
			//}

			//// guard based on cv wait
			//H::MakeScope([this] {
			//	if (auto cv = callerCv.lock()) {
			//		cv->notify_all(); // signal about finish callback
			//	}
			//	});

			if (callback) {
				return callback(std::forward<Args>(args)...);
			}
		}

	private:
		//std::weak_ptr<int> callerToken;
		//std::weak_ptr<std::condition_variable> callerCv;
		mutable std::mutex mx;
		FnType callback;
	};


	// Helper to determine type (FunctionMt_impl) from argument list (logic copied from std::function declaration of _Get_function_impl)
	template <class _Tx>
	struct _Get_function_impl {
		static_assert(_Always_false<_Tx>, "FunctionMt does not accept non-function types as template arguments.");
	};

	template <typename _Ret, typename... _Types>
	struct _Get_function_impl<_Ret(_Types...)> {
		using type = FunctionMt_impl<_Ret, _Types...>;
	};


	// NOTE: You need specify function type explicitly for FunctionMt (for example Fn = FunctionMt<void(int)>). 
	// TODO: To fix it use SFINAE Ctor like std::function to do it automatically. 

	// Wrapper under std::function with token/cv guard for multithreading
	template<typename Fn>
	class FunctionMt : public _Get_function_impl<Fn>::type { // wrapper for callable objects
		using _Mybase = typename _Get_function_impl<Fn>::type;

	public:
		FunctionMt() = default;
		FunctionMt(typename _Mybase::FnType callback)
			: _Mybase(callback)
		{
		}
	};
}