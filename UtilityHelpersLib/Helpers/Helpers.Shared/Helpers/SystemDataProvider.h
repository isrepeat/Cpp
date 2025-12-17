#pragma once
#include "common.h"
#if COMPILE_FOR_DESKTOP
#include "Singleton.hpp"
#include <string>

namespace HELPERS_NS {
	enum DataType {
		Architecture,
		OS,
		Region,
		Language,
		RAM
	};

	class SystemDataProvider : public Singleton<class SystemDataProvider> {
	private:
		friend SingletonInherited_t; // to have access to private Ctor

		SystemDataProvider();
	public:
		~SystemDataProvider();

		std::string Get(DataType dataType) const;

	private:
		bool InitObjects();
		void FetchOsInfo();
		void FetchLangInfo();

	private:
		std::string os;
		std::string arch;
		std::string region;
		std::string language;
		int ram;
	};
}
#endif