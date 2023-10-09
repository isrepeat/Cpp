#include <Windows.h>
#include <functional>
#include <iostream>
#include <cassert>
#include <string>
#include <type_traits>
#include <Helpers/FunctionTraits.h>



template <typename T>
struct TokenContextWeak { // use TokenCotext<T>::GetWeak()
	using Data_t = T;

	Data_t* data = nullptr;
	std::weak_ptr<int> token;
};


template <typename T>
class TokenContext { // place as class member
public:
	using Data_t = T;

	TokenContext(Data_t* data)
		: data{ data }
	{
	}

	TokenContextWeak<T> GetWeak() const {
		return { data, token };
	};

private:
	Data_t* data = nullptr;
	std::shared_ptr<int> token = std::make_shared<int>();
};



template<typename R, typename... Ts>
class ICallback {
public:
	ICallback() {}
	virtual ~ICallback() = default;

	virtual H::Result_t<R> Invoke(Ts... args) = 0;
	virtual ICallback* Clone() const = 0;
};


template<typename R, typename... Ts>
class Callback {
public:
	Callback()
	{
	}

	Callback(std::unique_ptr<ICallback<R, Ts...>>&& callback)
		: callback(std::move(callback))
	{
	}

	Callback(const Callback& other)
		: callback(other.callback ? std::unique_ptr<ICallback<R, Ts...>>(other.callback->Clone()) : nullptr)
	{
	}

	Callback(Callback&& other)
		: callback(std::move(other.callback))
	{
	}

	Callback& operator=(const Callback& other) {
		if (this != &other) {
			this->callback = other.callback ? std::unique_ptr<ICallback<R, Ts...>>(other.callback->Clone()) : nullptr;
		}

		return *this;
	}

	Callback& operator=(Callback&& other) {
		if (this != &other) {
			this->callback = std::move(other.callback);
		}

		return *this;
	}

	H::Result_t<R> operator()(Ts... args) {
		if (!this->callback) {
			assert(false && " --> callback is empty!");
			return {};
		}

		return this->callback->Invoke(args...);
	}

	operator bool() const {
		return this->callback != nullptr;
	}

private:
	std::unique_ptr<ICallback<R, Ts...>> callback;
};



template<typename T, typename R, typename... Ts>
class GenericCallback : public ICallback<R, Ts...> {
public:

	GenericCallback(TokenContextWeak<T> ctx, R(*callbackFn)(typename TokenContextWeak<T>::Data_t* data, Ts... args))
		: ctx(ctx)
		, callbackFn(callbackFn)
	{
	}

	GenericCallback(const GenericCallback& other)
		: ctx(other.ctx)
		, callbackFn(other.callbackFn)
	{
	}

	virtual ~GenericCallback() = default;

	GenericCallback& operator=(const GenericCallback& other) {
		if (this != &other) {
			this->ctx = other.ctx;
			this->callbackFn = other.callbackFn;
		}

		return *this;
	}

	GenericCallback& operator=(GenericCallback&& other) {
		if (this != &other) {
			this->ctx = std::move(other.ctx);
			this->callbackFn = std::move(other.callbackFn);
		}

		return *this;
	}

	H::Result_t<R> Invoke(Ts... args) override {
		if (this->ctx.token.expired()) {
			return {};
		}
		return H::InvokeHelper(this->callbackFn, this->ctx.data, std::forward<Ts>(args)...);
	}

	ICallback<R, Ts...>* Clone() const override {
		GenericCallback* clone = new GenericCallback(*this);
		return clone;
	}


private:
	TokenContextWeak<T> ctx;
	R(*callbackFn)(typename TokenContextWeak<T>::Data_t* data, Ts... args);
};


template<typename T, typename R, typename... Ts>
Callback<R, Ts...> MakeCallback(TokenContextWeak<T> ctx, R(*callbackFn)(typename TokenContextWeak<T>::Data_t* data, Ts... args)) {
	auto icallback = std::make_unique<GenericCallback<T, R, Ts...>>(ctx, callbackFn);
	return Callback<R, Ts...>(std::move(icallback));
}



class A {
public:
    A() {
        printf("A() \n");
    }
    ~A() {
        printf("~A() \n");
    }

	TokenContextWeak<A> GetContext() {
		return ctx.GetWeak();
	}

private:
	const TokenContext<A> ctx = this;
};

void ProgressCallback(A* _this, float progress) {
	printf("progress = %f [%d] \n", progress, static_cast<int>(progress * 100));
	//return 17;
}



void Test() {
	{
		A a;
		auto callback = std::make_shared<Callback<void, float>>(MakeCallback(a.GetContext(), &ProgressCallback));
		auto res = (*callback)(12);
		
		//auto callback = MakeCallback(a.GetContext(), &ProgressCallback);
		//auto res = callback(12);
		Sleep(1);
		Sleep(1);
		
	}
	Sleep(1);
	Sleep(1);
    return;
}


void main() {
    Test();
    return;
}


