#pragma once
#include "libhelpers\Metainfo\Signature.h"

#include <memory>
#include <functional>

namespace thread {
	// class T - type of progress data
	template<class T>
	class IProgress {
	public:
		virtual ~IProgress() = default;

		virtual void Report(const T &progress) = 0;
	};

	// class T - type of progress data
	// class Fn - type of handling object(std::function, function pointer), void(T);
	// Use MakeGenericProgress function to create IGenericProgress instance
	template<class T, class Fn>
	class IGenericProgress : public IProgress<T> {
	public:
		IGenericProgress(Fn fn) 
			: fn(fn)
		{}

		void Report(const T &progress) override {
			fn(progress);
		}

	private:
		Fn fn;
	};

	template<class Fn>
	IGenericProgress<typename Signature<Fn>::A1, Fn> MakeGenericProgress(Fn fn) {
		return IGenericProgress<typename Signature<Fn>::A1, Fn>(fn);
	}

    template<class Fn>
    std::unique_ptr<IGenericProgress<typename Signature<Fn>::A1, Fn>> MakeGenericProgressUnique(Fn fn) {
        return std::make_unique<IGenericProgress<typename Signature<Fn>::A1, Fn>>(fn);
    }

    template<class T>
    std::unique_ptr<IProgress<T>> MakeEmptyProgressUnique() {
        return MakeGenericProgressUnique([](T) {});
    }
}