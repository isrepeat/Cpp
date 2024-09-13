#include <JsonParser/JsonParser.h>
#include <MagicEnum/MagicEnum.h>
#include <Helpers/JSONConfigLoader.hpp>
#include <Helpers/SystemInfo.h>
#include <Helpers/Config.h>
#include <Helpers/Logger.h>
#include <Helpers/String.h>
#include <Helpers/Flags.h>
#include <Helpers/Regex.h>
#include <utility>
#include <fstream>
#include <format>
#include <ranges>
#include <regex>


#define _Enum__ShellExtensionType \
	Unknown, \
	Archivator

namespace TestConfigParsing::ShellExt::enums {
	enum ShellExtensionType {
		_Enum__ShellExtensionType
	};
}

JSONPARSER_ENUM_DECLARE_STRING_PARSER(TestConfigParsing::ShellExt::enums, ShellExtensionType,
	_Enum__ShellExtensionType
);


namespace TestConfigParsing {
	namespace ShellExt {
		namespace json {
			struct ShellBaseParams {
				enums::ShellExtensionType shellExtensionType = enums::ShellExtensionType::Unknown;
				std::wstring activationProtocol = L"test";
			};

			struct ShellBase : ShellBaseParams {
				JS_OBJECT(
					JS_MEMBER_ALIASES(shellExtensionType, "ShellExtensionType", "shell_extension_type"),
					JS_MEMBER_ALIASES(activationProtocol, "ActivationProtoco", "activation_protoco")
				);

				ShellBase() = default;
				ShellBase(ShellBaseParams shellBaseParams)
					: ShellBaseParams(shellBaseParams)
				{}
			};


			/* ------------------------------------ */
			/*                Settings              */
			/* ------------------------------------ */
			struct ShellMenu {
				std::wstring title;
				std::wstring command;
				std::vector<ShellMenu> subMenus;

				JS_OBJECT(
					JS_MEMBER_ALIASES(title, "Title"),
					JS_MEMBER_ALIASES(command, "Command"),
					JS_MEMBER_ALIASES(subMenus, "SubMenus", "sub_menus")
				);
			};

			struct ShellExtensionSettings : ShellBase {
				std::vector<ShellMenu> menus;

				JS_OBJECT_WITH_SUPER(
					JS_SUPER(ShellBase),
					JS_MEMBER_ALIASES(menus, "Menus")
				);

				ShellExtensionSettings(ShellBaseParams shellBaseParams, std::vector<ShellMenu> menus)
					: ShellBase(shellBaseParams)
					, menus{ menus }
				{}
			};

			/* ------------------------------------ */
			/*             Localization             */
			/* ------------------------------------ */
			struct ShellExtensionTranslation {
				std::map<std::string, std::wstring> translationMap;

				JS_OBJECT(
					JS_MEMBER_ALIASES(translationMap, "TranslationMap", "translation_map")
				);
			};

			struct ShellExtensionLocalization {
				std::map<std::string, ShellExtensionTranslation> localizationMap;

				JS_OBJECT(
					JS_MEMBER_ALIASES(localizationMap, "LocalizationMap", "localization_map")
				);
			};

			struct SupportedLocales {
				std::set<std::string> locales;

				JS_OBJECT(
					JS_MEMBER_ALIASES(locales, "Locales")
				);
			};
		} // namespace json


#define __NAMESPACE__ TestConfigParsing, ShellExt
		PP_INLINE_TEMPLATE_SPECIALIZATION_BEGIN();
		namespace PP_LAST_NAMESPACE {
			class ShellExtensionConfig;
		};

		template <>
		struct H::ConfigData<PP_LAST_NAMESPACE::ShellExtensionConfig> {
			struct Data {
				PP_LAST_NAMESPACE::json::ShellExtensionSettings shellExtensionSettings;
				PP_LAST_NAMESPACE::json::ShellExtensionLocalization shellExtensionLocalization;
				PP_LAST_NAMESPACE::json::SupportedLocales supportedLocales;

				JS_OBJECT(
					JS_MEMBER_ALIASES(shellExtensionSettings, "ShellExtensionSettings", "shell_extension_settings"),
					JS_MEMBER_ALIASES(shellExtensionLocalization, "ShellExtensionLocalization", "shell_extension_localization"),
					JS_MEMBER_ALIASES(supportedLocales, "SupportedLocales", "supported_locales")
				);
			};
		};
		PP_INLINE_TEMPLATE_SPECIALIZATION_END();


		class ShellExtensionConfig : public H::ConfigBase<class ShellExtensionConfig> {
		public:
			ShellExtensionConfig()
				: _Base(this->CreateDefaultData())
			{}

		private:
			_Data CreateDefaultData() {
				json::ShellBaseParams shellBaseParams;
				shellBaseParams.shellExtensionType = enums::ShellExtensionType::Archivator;
				shellBaseParams.activationProtocol = L"dct-archivator";

				json::ShellMenu menuA = { L"<translate:MenuA>", L"MenuA_command" };
				json::ShellMenu menuB = {
					L"MenuB",
					L"MenuB_command",
					{
						json::ShellMenu{L"subMenuB1", L"subMenuB1_command"},
						json::ShellMenu{L"subMenuB2", L"subMenuB2_command"},
					}
				};

				return _Data{
					json::ShellExtensionSettings{
						shellBaseParams,
						std::vector<json::ShellMenu>{
							menuA,
							menuB,
						}
					},
					json::ShellExtensionLocalization{
						std::map<std::string, json::ShellExtensionTranslation>{
							{
								"en",
								json::ShellExtensionTranslation{
									std::map<std::string, std::wstring> {
										{"MenuA", L"en_MenuA"}
									}
								}
							},
							{
								"ru",
								json::ShellExtensionTranslation{
									std::map<std::string, std::wstring> {
										{"MenuA", L"ru_MenuA"}
									}
								}
							},
						}
					},
					
					json::SupportedLocales{
						std::set<std::string>{
							{"en"},
							{"ru"},
						}
					},
				};
			}
		};


		class AppFeatures {
		private:
			AppFeatures();
		public:
			~AppFeatures() = default;
			static AppFeatures& GetInstance();

			struct JSONObject {
				static constexpr const char* Filename = "ShellExtensionConfig.txt";

				ShellExt::ShellExtensionConfig::_Data shellExtensionConfig = ShellExt::ShellExtensionConfig::GetDataCopy();

				JS_OBJ(
					shellExtensionConfig
				);

				static void AfterLoadHandler(const JSONObject& jsonObject) {
					const_cast<ShellExt::ShellExtensionConfig::_Data&>(ShellExt::ShellExtensionConfig::GetDataLocked().Get()) = jsonObject.shellExtensionConfig;
				}
			};

		private:
			const std::filesystem::path configName;
			const std::filesystem::path configFolder;
		};

		AppFeatures& AppFeatures::GetInstance() {
			static AppFeatures instance;
			return instance;
		}

		AppFeatures::AppFeatures()
			: configName{ JSONObject::Filename }
			, configFolder{ "D:\\TEST_CONFIGS\\" }
			//, configFolder{ H::ExePath() }
		{
			lg::DefaultLoggers::InitSingleton();
			bool wasSomethingTranslated = false;

			if (H::JSONConfigLoader<AppFeatures::JSONObject, AppFeatures>::Load(this->configFolder / this->configName)) {
				auto shellExtensionConfig = ShellExt::ShellExtensionConfig::GetDataLocked();
				auto shellExtensionConfigMutable = const_cast<ShellExt::ShellExtensionConfig::_Data*>(&shellExtensionConfig.Get());

				for (auto& shellMenu : shellExtensionConfigMutable->shellExtensionSettings.menus) {
					// For example if string = "<translate: My 'title'>" regex must capture "My 'title'"
					auto matches = H::Regex::GetRegexMatches<wchar_t>(shellMenu.title, std::wregex{ L"<translate:[ ]*(.*)>" });
					if (!matches.empty()) {
						auto sourceWString = matches[0].capturedGroups.at(1);
						auto sourceString = H::WStrToStr(sourceWString);

						std::string capturedLocale = "en";
						auto prefferedLanguages = H::GetUserPreferredUILanguages();
						if (!prefferedLanguages.empty()) {
							//auto firstPrefferedLanguage = H::WStrToStr(prefferedLanguages.front());
							auto firstPrefferedLanguage = prefferedLanguages.front();
							auto matches = H::Regex::GetRegexMatches<wchar_t>(firstPrefferedLanguage, std::wregex{ L"(\\w\\w)([-_]\\w\\w)?" });
							if (!matches.empty()) {
								auto mainLocale = matches[0].capturedGroups.at(1);
								//capturedLocale = H::to_lower(mainLocale);
								capturedLocale = H::to_lower(H::WStrToStr(mainLocale));
							}
						}

						auto localeIt = shellExtensionConfig->shellExtensionLocalization.localizationMap.find(capturedLocale);
						if (localeIt != shellExtensionConfig->shellExtensionLocalization.localizationMap.end()) {
							auto& [locale, shellExtensionTranslation] = *localeIt;

							auto sourceStringIt = shellExtensionTranslation.translationMap.find(sourceString);
							if (sourceStringIt != shellExtensionTranslation.translationMap.end()) {
								auto& [_, translatedString] = *sourceStringIt;
								shellMenu.title = translatedString;
								wasSomethingTranslated = true;
							}
						}
					}
				}
			}
			else {
				LOG_ERROR_D("Config not loaded");
			}

			//if (wasSomethingTranslated) {
			//	LOG_DEBUG_D("{} [translated]:\n"
			//		"{}"
			//		, JSONObject::Filename
			//		, ::JS::serializeStruct(ShellExt::ShellExtensionConfig::GetDataLocked().Get())
			//	);
			//}
		}
	} // namespace ShellExt


	void TestLoadShellExtensionConfig() {
		LOG_FUNCTION_ENTER("TestLoadShellExtensionConfig()");

		ShellExt::AppFeatures::GetInstance(); // Entry point to load config
		NOOP;
	}
} // namespace TestConfigParsing





int main() {
	H::Flags<lg::InitFlags> loggerInitFlags =
		lg::InitFlags::DefaultFlags |
		lg::InitFlags::EnableLogToStdout;

	lg::DefaultLoggers::Init("D:\\main_60_Json.log", loggerInitFlags);

	LOG_DEBUG_D("Упаковать [Папка]");

	JS::LoggerCallback::Register([](std::string msg) {
		LOG_DEBUG_D("[JsonParser] {}", msg);
		});

	TestConfigParsing::TestLoadShellExtensionConfig();

	Sleep(15'000);
	return 0;
}