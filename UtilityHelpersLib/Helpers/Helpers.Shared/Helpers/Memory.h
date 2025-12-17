#pragma once
#include "common.h"
#include <algorithm>
#include <memory>

namespace HELPERS_NS {
	// https://codereview.stackexchange.com/questions/256651/dynamic-pointer-cast-for-stdunique-ptr
	// https://stackoverflow.com/questions/26377430/how-to-perform-a-dynamic-cast-with-a-unique-ptr
	template <typename To, typename From, typename Deleter>
	std::unique_ptr<To, Deleter> dynamic_unique_cast(std::unique_ptr<From, Deleter>&& ptr) {
		if (To* cast = dynamic_cast<To*>(ptr.get())) {
			std::unique_ptr<To, Deleter> result(cast, std::move(ptr.get_deleter()));
			ptr.release();
			return result;
		}
		return {};
	}

	template <typename To, typename From>
	std::unique_ptr<To> dynamic_unique_cast(std::unique_ptr<From>&& ptr) {
		if (To* cast = dynamic_cast<To*>(ptr.get())) {
			std::unique_ptr<To> result(cast);
			ptr.release();
			return result;
		}
		return {};
	}


	template<class T, class D>
	std::unique_ptr<T, D> WrapUnique(T* ptr, const D& deleter) {
		return std::unique_ptr<T, D>(ptr, deleter);
	}

	template <typename SmartPointerT>
	SmartPointerT& EmptyPointerRef() {
		static std::remove_reference_t<SmartPointerT> emptyPoiner = nullptr;
		return emptyPoiner;
	}


	template<class T, class D>
	class GetAddressOfUnique {
	public:
		GetAddressOfUnique(std::unique_ptr<T, D>& ptr)
			: ptr(ptr)
		{
			this->rawPtr = this->ptr.release();
		}

		operator T** () {
			return &this->rawPtr;
		}

		~GetAddressOfUnique() {
			this->ptr.reset(this->rawPtr);
		}

	private:
		T* rawPtr;
		std::unique_ptr<T, D>& ptr;
	};

	template<class T, class D>
	GetAddressOfUnique<T, D> GetAddressOf(std::unique_ptr<T, D>& v) {
		return GetAddressOfUnique<T, D>(v);
	}

	template<class T>
	struct CoDeleter {
		void operator()(T* ptr) {
			CoTaskMemFree(ptr);
		}
	};

	// Unique pointer for objects that were allocated with CoTaskMemAlloc
	template<class T>
	struct CoUniquePtr : public std::unique_ptr<T, CoDeleter<T>> {
		GetAddressOfUnique<T, CoDeleter<T>> GetAddressOf() {
			return GetAddressOfUnique<T, CoDeleter<T>>(*this);
		}
	};
}