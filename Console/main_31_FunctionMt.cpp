#include <mutex>
#include <iostream>
#include <functional>
#include <Helpers/FunctionTraits.h>

// Wrapper under std::function with mutex guard for multithreading
template<typename Ret, typename ...Args>
class CallbackMt_impl {
protected:
	using FnType = std::function<Ret(Args...)>;

	CallbackMt_impl() = default;
	CallbackMt_impl(std::function<Ret(Args...)> callback)
		: callback{ callback }
	{
	}

public:
	void SetMutex(std::shared_ptr<std::mutex> sharedMx) {
		weakMx = sharedMx;
	}

	Ret operator() (Args... args) const {
		if (callback) {
			if (auto sharedMx = weakMx.lock()) {
				std::unique_lock lk{ sharedMx.get() };
			}
			return callback(std::forward<Args>(args)...);
		}
	}

private:
	std::weak_ptr<std::mutex> weakMx;
	FnType callback;
};


// Helper to determine type (CallbackMt_impl) from argument list (logic copied from std::function declaration of _Get_function_impl)
template <class _Tx>
struct _Get_callback_impl {
	static_assert(_Always_false<_Tx>, "CallbackMt does not accept non-function types as template arguments.");
};

template <typename _Ret, typename... _Types>
struct _Get_callback_impl<_Ret(_Types...)> { 
	using type = CallbackMt_impl<_Ret, _Types...>;
};


// NOTE: You need specify function type explicitly (for example Fn = void(int)). 
// TODO: To fix it use SFINAE Ctor like std::function to do it automatically. 
template<typename Fn>
class CallbackMt : public _Get_callback_impl<Fn>::type { // wrapper for callable objects
	using _Mybase = typename _Get_callback_impl<Fn>::type;

public:
	CallbackMt() = default;
	CallbackMt(typename _Mybase::FnType callback)
		: _Mybase(callback)
	{
	}
};



void main() {
	CallbackMt<void(int)> callbackOutter;
	callbackOutter(0);

	{
		CallbackMt<void(int)> callback = std::function([](int a) {
			int xxx = 9;
			});
		callbackOutter = callback;
		callback(1);
	}


	std::mutex mx;
	callbackOutter(mx, 2);

	return;
}
