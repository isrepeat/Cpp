#pragma once
#include "Callback.h"

#include <msclr\auto_gcroot.h>

template<class T, class... Types>
class GenericWeakGCRootCallback : public ICallback<Types...> {
public:
	GenericWeakGCRootCallback(T data, void(__clrcall *callbackFn)(T data, Types... args))
		: data(gcnew System::WeakReference(data)), callbackFn(callbackFn)
	{
	}

	GenericWeakGCRootCallback(const GenericWeakGCRootCallback &other)
		: callbackFn(other.callbackFn)
	{
		System::WeakReference ^weakRef = other.data.get();

		if (weakRef) {
			this->data = gcnew System::WeakReference(weakRef->Target);
		}
	}

	GenericWeakGCRootCallback(GenericWeakGCRootCallback &&other)
		: data(std::move(other.data)),
		callbackFn(std::move(other.callbackFn))
	{
	}

	virtual ~GenericWeakGCRootCallback() {}

	GenericWeakGCRootCallback &operator=(const GenericWeakGCRootCallback &other) {
		if (this != &other) {
			T tmp;
			System::WeakReference ^weakRef = other.data.get();

			if (weakRef) {
				this->data = gcnew System::WeakReference(tmp);
			}

			this->callbackFn = other.callbackFn;
		}

		return *this;
	}

	GenericWeakGCRootCallback &operator=(GenericWeakGCRootCallback &&other) {
		if (this != &other) {
			this->data = std::move(other.data);
			this->callbackFn = std::move(other.callbackFn);
		}

		return *this;
	}

	void Invoke(Types... args) override {
		System::WeakReference ^weakRef = this->data.get();

		if (weakRef) {
			T tmp = dynamic_cast<T>(weakRef->Target);

			if (tmp) {
				this->callbackFn(tmp, std::forward<Types>(args)...);
			}
		}
	}

	ICallback *Clone() const override {
		GenericWeakGCRootCallback *clone = new GenericWeakGCRootCallback(*this);
		return clone;
	}

private:
	msclr::auto_gcroot<System::WeakReference ^> data;
	void(__clrcall *callbackFn)(T data, Types... args);
};

template<class T, class... Types>
Callback<Types...> MakeWeakGCRootCallback(T data, void(__clrcall *callbackFn)(T data, Types... args)) {
	auto ptr = new GenericWeakGCRootCallback<T, Types...>(data, callbackFn);
	auto icallback = std::unique_ptr<GenericWeakGCRootCallback<T, Types...>>(ptr);
	return Callback<Types...>(std::move(icallback));
}