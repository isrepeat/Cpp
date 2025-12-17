#pragma once
#include "common.h"
#include "Std/Extensions/memoryEx.h"
#include <type_traits>

namespace HELPERS_NS {
	template <typename TBase, typename... TArgs>
	struct ICloneable {
		using ICloneableInherited_t = ICloneable<TBase, TArgs...>;

		virtual ~ICloneable() = default;
		virtual std::ex::shared_ptr<TBase> Clone(TArgs...) const = 0;

		template<typename TDerived, typename TProxyBase = TBase>
		struct DefaultCloneableImpl : TProxyBase {
			static_assert(std::is_base_of_v<TBase, TProxyBase>, "TProxyBase must derive from TBase");

			using DefaultCloneableImplInherited_t = DefaultCloneableImpl<TDerived, TProxyBase>;
			using TProxyBase::TProxyBase;

			std::ex::shared_ptr<TBase> Clone(TArgs... args) const override {
				if constexpr (sizeof...(TArgs) == 0) {
					// Безопасная полная копия.
					// ctor TDerived(const TDerived&)
					return std::ex::make_shared_ex<TDerived>(
						static_cast<const TDerived&>(*this)
					);
				}
				else if constexpr (std::is_constructible_v<TDerived, const TDerived&, TArgs...>) {
					// Копия с параметрами, без потери данных.
					// ctor TDerived(const TDerived&, TArgs...)
					return std::ex::make_shared_ex<TDerived>(
						static_cast<const TDerived&>(*this),
						std::forward<TArgs>(args)...
					);
				}
				else if constexpr (std::is_constructible_v<TDerived, const TProxyBase&, TArgs...>) {
					// Fallback: Копия с параметрами, с потенциальной потерей данных.
					// ctor TDerived(const TProxyBase&, TArgs...)
					//
					// Сработает если в TDerived есть "using DefaultCloneableImplInherited_t::DefaultCloneableImplInherited_t;",
					// а в TProxyBase есть "using TBase::TBase;" или TProxyBase предоставляет соответствующие copy Ctors.
					//
					// NOTE: проверка делается именно с TProxyBase, а не с TBase.
					// Причина: через "using TProxyBase::TProxyBase" в TDerived появляются перегрузки
					// конструкторов с параметром TProxyBase, но НЕ появляются с TBase.
					// То есть у TDerived может существовать ctor(const TProxyBase&, TArgs...),
					// но ctor(const TBase&, TArgs...) в нём не возникает автоматически, 
					// поэтому приводим TDerived к TProxyBase.
					return std::ex::make_shared_ex<TDerived>(
						static_cast<const TProxyBase&>(static_cast<const TDerived&>(*this)),
						std::forward<TArgs>(args)...
					);
				}
				else {
					static_assert(
						std::bool_constant<sizeof...(TArgs) == 0 ||
						std::is_constructible_v<TDerived, const TDerived&, TArgs...> ||
						std::is_constructible_v<TDerived, const TProxyBase&, TArgs...>
						>::value,
						"Clone: TDerived must be constructible with "
						"(const TDerived&, TArgs...) or (const TProxyBase&, TArgs...), "
						"or there must be no rebind args (copy-ctor case)."
						);
					return nullptr;
				}
			}
		};
	};
}