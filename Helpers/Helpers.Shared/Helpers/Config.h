#pragma once
#include "common.h"
#include "Meta/Diagnostics.h"
#include "Meta/Concepts.h"
#include "ThreadSafeObject.hpp"
#include "Singleton.hpp"

namespace HELPERS_NS {
	template <typename T>
	struct ConfigData {
		static_assert(meta::dependent_false<T>::value, "You must use specialization");
		struct Data {
		};
	};

	template <typename DerivedT>
	__requires_expr(
		meta::concepts::has_type<typename ConfigData<DerivedT>::Data>
	) class ConfigBase : public HELPERS_NS::Singleton<typename DerivedT> {
	private:
		using _MyBase = HELPERS_NS::Singleton<typename DerivedT>;

	protected:
		using _Base = ConfigBase<DerivedT>;

	public:
		using _Data = typename ConfigData<DerivedT>::Data;

		ConfigBase(const _Data& data)
			: data{ data }
		{}
		virtual ~ConfigBase() = default;

		static HELPERS_NS::ThreadSafeObject<std::recursive_mutex, const _Data>::_Locked GetDataLocked() {
			return _MyBase::GetInstance().data.Lock();
		}
		static _Data GetDataCopy() {
			auto locked = GetDataLocked();
			auto copy = locked.Get();
			return copy;
		}

	private:
		HELPERS_NS::ThreadSafeObject<std::recursive_mutex, const _Data> data;
	};
}