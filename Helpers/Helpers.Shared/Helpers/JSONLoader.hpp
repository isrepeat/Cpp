#pragma once
#include "Helpers/common.h"
#include "JsonParser/JsonParser.h"
#include "Helpers/Meta/FunctionTraits.h"
#include "Helpers/Std/Extensions/fstreamEx.h"
#include "Helpers/Localization.h"
#include "Helpers/Logger.h"

#include <vector>
#include <string>

#if !__cpp_concepts
#pragma message(PP_MSG_ERROR_REQUIRE_CPP_CONCEPTS)
#endif

namespace HELPERS_NS {
	template <typename JSONObjectT>
	__requires_expr(
		meta::concepts::has_static_function_with_signature<decltype(&JSONObjectT::AfterLoadHandler), void(*)(const JSONObjectT&)>
	) class JSONLoader {
		JSONLoader() = delete;
		~JSONLoader() = delete;

	private:
		template <typename AppFeaturesImplT>
		friend class AppFeaturesBase;

		// Return 'true' if the 'AfterLoadHandler' was successfuly called:
		// - json file exist and could be parsed success;
		// - json file not exist and could be saved success;
		static bool Load(const std::filesystem::path& filepath) {
			LOG_FUNCTION_ENTER("Load(filepath = {})", filepath.string());
			std::string jsonFilename = filepath.filename().string();

			STD_EXT_NS::ifstream jsonConfigFile(filepath, std::ios::binary);
			if (jsonConfigFile.is_open()) {
				auto readData = jsonConfigFile.ReadData();
				jsonConfigFile.close();

				LOG_DEBUG_D("\"{}\" data: \n{}", jsonFilename, HELPERS_NS::VecToStr(readData.byteArray));

				JS::ParserParams parserParams;
				if (parserParams.codePage = readData.codePage) {
					LOG_DEBUG_D("\"{}\" code page = {}", jsonFilename, parserParams.codePage.value());
				}

				// To avoid merge results after parsing ensure that all JSON objects is empty.
				JSONObjectT jsonObject;
				if (JS::ParseTo(readData.byteArray, jsonObject, parserParams)) {
					JSONObjectT::AfterLoadHandler(jsonObject);
					return true;
				}
				else {
					LOG_ERROR_D("Cannot parse '{}'", jsonFilename);
					if constexpr (requires { requires
						meta::concepts::has_static_function<decltype(&JSONObjectT::LoadErrorHandler)>
					}) {
						JSONObjectT::LoadErrorHandler();
					}
				}
			}
			else {
				LOG_WARNING_D("{} not found, create with default json", jsonFilename);

				JSONObjectT jsonObject;
				// If JSONObjectT has 'CreateWithDefaultData' static method so create json with 
				// specific default data before save.
				if constexpr (requires { requires
					meta::concepts::has_static_function_with_signature<decltype(&JSONObjectT::CreateWithDefaultData), JSONObjectT(*)()>
				}) {
					jsonObject = JSONObjectT::CreateWithDefaultData();
				}

				if (Save(filepath, jsonObject)) {
					JSONObjectT::AfterLoadHandler(jsonObject);
					return true;
				}
			}
			return false;
		}

		static bool Save(const std::filesystem::path& filepath, const JSONObjectT& jsonObject) {
			try {
				LOG_FUNCTION_ENTER("Save(filepath, jsonObject)");
				// You can use here for example BeforeSaveHandler(...) in future

				auto serializedStruct = JS::serializeStruct(jsonObject);
				LOG_DEBUG_D("Serialized jsonObject data: \n{}", serializedStruct);

				bool result = std::filesystem::create_directory(filepath.parent_path());

				std::ofstream outFile;
				outFile.open(filepath);
				outFile.write(serializedStruct.data(), serializedStruct.size());
				outFile.close();
			}
			catch (...) {
				LOG_ERROR_D("Cannot save config");
				LogLastError;
				return false;
			}
			return true;
		}
	};
}