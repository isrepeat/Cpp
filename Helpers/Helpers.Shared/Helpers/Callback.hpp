#pragma once
#include "common.h"
#include "TokenContext.hpp"
#include <cassert>
#include <memory>

namespace HELPERS_NS {
	template<typename R, typename... Ts>
	class ICallback {
	public:
		ICallback() {}
		virtual ~ICallback() = default;

		virtual R Invoke(Ts... args) = 0;
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

		R operator()(Ts... args) const {
			if (!this->callback) {
				assert(false && " --> callback is empty!");
				return R();
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
		GenericCallback(typename TokenContext<T>::Weak ctx, R(*callbackFn)(typename TokenContext<T>::Data_t* data, Ts... args))
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

		R Invoke(Ts... args) override {
			if (this->ctx.token.expired()) {
				return R();
			}
			return this->callbackFn(this->ctx.data, std::forward<Ts>(args)...);
		}

		ICallback<R, Ts...>* Clone() const override {
			GenericCallback* clone = new GenericCallback(*this);
			return clone;
		}


	private:
		typename TokenContext<T>::Weak ctx;
		R(*callbackFn)(typename TokenContext<T>::Data_t* data, Ts... args);
	};


	template<typename T, typename R, typename... Ts>
	Callback<R, Ts...> MakeCallback_impl(typename TokenContext<T>::Weak ctx, R(*callbackFn)(typename TokenContext<T>::Data_t* data, Ts... args)) {
		auto icallback = std::make_unique<GenericCallback<T, R, Ts...>>(ctx, callbackFn);
		return Callback<R, Ts...>(std::move(icallback));
	}


	template<typename TknWeak, typename R, typename... Ts, typename T = TknWeak::parent_t::Data_t>
	Callback<R, Ts...> MakeCallback(TknWeak ctx, R(*callbackFn)(typename TokenContext<T>::Data_t* data, Ts... args)) {
		return MakeCallback_impl<T, R, Ts...>(ctx, callbackFn);
	}
}