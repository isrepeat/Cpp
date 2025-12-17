#pragma once
#include "common.h"
#include "JSONLoader.hpp"
#include "Singleton.hpp"
#include "Logger.h"

namespace HELPERS_NS {
	template <typename AppFeaturesImplT>
	class AppFeaturesBase : public HELPERS_NS::_Singleton<AppFeaturesImplT> {
	private:
		using _MyBase = HELPERS_NS::_Singleton<AppFeaturesImplT>;
	protected:
		using _Inherited = AppFeaturesBase<AppFeaturesImplT>;
		using _InheritedBase = _MyBase;

	protected:
		AppFeaturesBase() {
			LOGGER_NS::DefaultLoggers::InitSingleton();
		}
		~AppFeaturesBase() = default;

		template <typename ConfigJsonT>
		bool LoadConfig(std::filesystem::path configFile) {
			if (HELPERS_NS::JSONLoader<ConfigJsonT>::Load(configFile)) {
				return true;
			}
			return false;
		}

		template <typename ConfigJsonT>
		void SaveConfig(std::filesystem::path configFile, const ConfigJsonT& configJsonObject) {
			HELPERS_NS::JSONLoader<ConfigJsonT>::Save(configFile, configJsonObject);
		}
	};
}