#pragma once
#include "common.h"
#include <type_traits>
#include <cassert>
#include <memory>
#include <mutex>

namespace HELPERS_NS {
	template <typename TClass, typename TInstance = void*>
	class Singleton {
	protected:
		using SingletonInherited_t = Singleton<TClass, TInstance>;

	public:
		// Call this in constructors of singletons classes (to ensure that TClass destroy after all of them)
		// TODO: implement smth like SingletonManager (and add DependsOn api) to destroy 
		// singletons from dependents to independens.
		static void InitSingleton() {
			GetInstance();
		}

		// Work with public Ctor without args
		static TClass& GetInstance() {
			static TClass instance;
			return instance;
		}

	protected:
		friend TClass;
		Singleton() = default;
		virtual ~Singleton() = default;

	private:
		TInstance instance; // by default not used (T = void*)
	};


	//
	// SingletonUnique
	//
	template <typename TClass>
	class SingletonUnique : public Singleton<SingletonUnique<TClass>, std::unique_ptr<TClass>> {
	private:
		using MyBase_t = Singleton<SingletonUnique<TClass>, std::unique_ptr<TClass>>;

	public:
		using Instance_t = TClass&;

		SingletonUnique() = default;
		~SingletonUnique() = default;

		template <typename ...TArgs>
		static Instance_t CreateInstance(TArgs&&... args) {
			auto& _this = MyBase_t::GetInstance();
			std::unique_lock lk{ _this.mx };

			if (_this.instance == nullptr) {
				_this.instance = std::make_unique<TClass>(std::forward<TArgs>(args)...);
			}

			return *_this.instance;
		}

		static Instance_t GetInstance() {
			auto& _this = MyBase_t::GetInstance();

			if constexpr (std::is_default_constructible_v<TClass>) {
				if (!_this.instance) {
					return CreateInstance(); // default ctor
				}
			}
			else {
				assert(_this.instance);
			}

			return *_this.instance;
		}

	private:
		std::mutex mx;
	};


	//
	// SingletonShared
	//
	template <typename TClass>
	class SingletonShared : public Singleton<SingletonShared<TClass>, std::shared_ptr<TClass>> {
	private:
		using MyBase_t = Singleton<SingletonShared<TClass>, std::shared_ptr<TClass>>;

	public:
		using Instance_t = std::shared_ptr<TClass>;

		SingletonShared() = default;
		~SingletonShared() = default;

		template <typename ...TArgs>
		static Instance_t CreateInstance(TArgs&&... args) {
			auto& _this = MyBase_t::GetInstance();
			std::unique_lock lk{ _this.mx };

			if (_this.instance == nullptr) {
				_this.instance = std::make_shared<TClass>(std::forward<TArgs>(args)...);
			}

			return _this.instance;
		}


		static Instance_t GetInstance() {
			auto& _this = MyBase_t::GetInstance();

			if constexpr (std::is_default_constructible_v<TClass>) {
				if (!_this.instance) {
					return CreateInstance();
				}
			}
			else {
				assert(_this.instance);
			}

			return _this.instance;
		}

	private:
		std::mutex mx;
	};


	//
	// SingletonUnscoped
	//
	template <typename TClass>
	class SingletonUnscoped : public Singleton<SingletonUnscoped<TClass>, TClass*> {
	private:
		using MyBase_t = Singleton<SingletonUnscoped<TClass>, TClass*>;

	public:
		using Instance_t = TClass*;

		SingletonUnscoped() = default;
		~SingletonUnscoped() = default;

		template <typename ...TArgs>
		static Instance_t CreateInstance(TArgs&&... args) {
			auto& _this = MyBase_t::GetInstance();
			std::unique_lock lk{ _this.mx };

			if (_this.instance == nullptr) {
				_this.instance = new TClass(std::forward<TArgs>(args)...);
			}

			return _this.instance;
		}

		static Instance_t GetInstance() {
			auto& _this = MyBase_t::GetInstance();
			
			if constexpr (std::is_default_constructible_v<TClass>) {
				if (!_this.instance) {
					return CreateInstance();
				}
			}
			else {
				assert(_this.instance);
			}

			return _this.instance;
		}

		static void DeleteInstance() {
			auto& _this = MyBase_t::GetInstance();
			std::unique_lock lk{ _this.mx };

			delete _this.instance;
			_this.instance = nullptr;
		}

	private:
		std::mutex mx;
	};
}