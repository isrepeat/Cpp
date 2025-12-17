#pragma once

#include <memory>

template<class... Types>
class ICallback {
public:
	ICallback() {}
	virtual ~ICallback() {}

	virtual void Invoke(Types... args) = 0;
	virtual ICallback *Clone() const = 0;
};

template<class T, class... Types>
class GenericCallback : public ICallback<Types...>{
public:
	GenericCallback(const T &data, void(*callbackFn)(const T &data, Types... args))
		: data(data), callbackFn(callbackFn)
	{
	}

	GenericCallback(const GenericCallback &other) 
		: data(other.data), callbackFn(other.callbackFn)
	{
	}

	GenericCallback(GenericCallback &&other)
		: data(std::move(other.data)), 
		callbackFn(std::move(other.callbackFn))
	{
	}

	virtual ~GenericCallback() {}

	GenericCallback &operator=(const GenericCallback &other) {
		if (this != &other) {
			this->data = other.data;
			this->callbackFn = other.callbackFn;
		}

		return *this;
	}

	GenericCallback &operator=(GenericCallback &&other) {
		if (this != &other) {
			this->data = std::move(other.data);
			this->callbackFn = std::move(other.callbackFn);
		}

		return *this;
	}

	void Invoke(Types... args) override {
		this->callbackFn(this->data, std::forward<Types>(args)...);
	}

	ICallback *Clone() const override {
		GenericCallback *clone = new GenericCallback(*this);
		return clone;
	}

private:
	T data;
	void(*callbackFn)(const T &data, Types... args);
};

template<class... Types>
class Callback {
public:
	Callback()
	{
	}

	Callback(std::unique_ptr<ICallback<Types...>> &&callback)
		: callback(std::move(callback))
	{
	}

	Callback(const Callback &other) 
		: callback(std::unique_ptr<ICallback<Types...>>(other.callback->Clone()))
	{
	}

	Callback(Callback &&other)
		: callback(std::move(other.callback))
	{
	}

	Callback &operator=(const Callback &other) {
		if (this != &other) {
			this->callback = std::unique_ptr<ICallback<Types...>>(other.callback->Clone());
		}

		return *this;
	}

	Callback &operator=(Callback &&other) {
		if (this != &other) {
			this->callback = std::move(other.callback);
		}

		return *this;
	}

	void operator()(Types... args) {
		this->callback->Invoke(args...);
	}

	operator bool() const {
		return this->callback != nullptr;
	}

private:
	std::unique_ptr<ICallback<Types...>> callback;
};

template<class T, class... Types>
Callback<Types...> MakeCallback(const T &data, void(*callbackFn)(const T &data, Types... args)) {
	auto icallback = std::make_unique<GenericCallback<T, Types...>>(data, callbackFn);
	return Callback<Types...>(std::move(icallback));
}