#pragma once
#include "common.h"
#include "Meta/Utilities/TypeErasedDeleter.h"
#include "Std/Extensions/optional_ref.h"

#include <unordered_map>
#include <type_traits>
#include <typeindex>
#include <memory>

namespace HELPERS_NS {
	//
	// IServiceProvider: типобезопасный доступ к расширениям (сервисам)
	//
	struct IServiceProvider {
	public:
		virtual ~IServiceProvider() = default;

	public:
		// Возвращаем НEВЛАДЕЮЩУЮ ссылку через optional_ref.
		// Жизнью сервиса управляет провайдер, а не потребитель.
		template <typename TInterface>
		STD_EXT_NS::optional_ref<const TInterface> Get() const {
			const void* rawPtr = this->OnGetServiceRaw(typeid(TInterface));
			if (rawPtr == nullptr) {
				return {};
			}
			return STD_EXT_NS::optional_ref<const TInterface>{
				*static_cast<const TInterface*>(rawPtr)
			};
		}

	protected:
		// Возвращает сырой указатель на сервис (или nullptr). Владеет по-прежнему провайдер.
		virtual const void* OnGetServiceRaw(
			const std::type_info& typeInfo
		) const = 0;
	};

	
	//
	// DefaultServiceProvider: простая реализация на базе registry
	//
	struct DefaultServiceProvider final : public IServiceProvider {
	public:
		template <typename TInterface>
		void AddService(std::unique_ptr<TInterface> serviceUniquePtr) {
			auto key = std::type_index{ typeid(TInterface) };

			this->mapTypeToServiceUniquePtr[key] =
				std::unique_ptr<void, meta::TypeErasedDeleter>(
					serviceUniquePtr.release(),
					meta::TypeErasedDeleter::Make<TInterface>()
				);
		}

	protected:
		const void* OnGetServiceRaw(const std::type_info& typeInfo) const override {
			auto it = this->mapTypeToServiceUniquePtr.find(std::type_index{ typeInfo });
			if (it != this->mapTypeToServiceUniquePtr.end()) {
				return it->second.get();
			}
			return nullptr;
		}

	private:
		// Храним void* + кастомный делетер, чтобы корректно удалить реальный T
		std::unordered_map<std::type_index, std::unique_ptr<void, meta::TypeErasedDeleter>> mapTypeToServiceUniquePtr;
	};
}