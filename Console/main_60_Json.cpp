#include <JsonParser/JsonParser.h>
#include <MagicEnum/MagicEnum.h>
#include <Helpers/JSONLoader.hpp>
#include <Helpers/AppFeaturesBase.h>
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

const std::filesystem::path g_TestOutputFolder =
	std::filesystem::path(__TEST_OUTPUT_FOLDER) / std::filesystem::path(__FILE__).stem();

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

				ShellExtensionSettings() = default;
				ShellExtensionSettings(ShellBaseParams shellBaseParams, std::vector<ShellMenu> menus = {})
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

				JS_OBJECT(
					JS_MEMBER_ALIASES(shellExtensionSettings, "ShellExtensionSettings", "shell_extension_settings"),
					JS_MEMBER_ALIASES(shellExtensionLocalization, "ShellExtensionLocalization", "shell_extension_localization")
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
										{"MenuA", L"Привет"}
									}
								}
							},
						}
					},
				};
			}
		};




		struct Constants : H::_Singleton<struct Constants> {
			const H::Locale userLocale;

			Constants()
				: userLocale{ this->GetFirstUserPreferredUILocaleOrDefault() }
			{
			}

		private:
			H::Locale GetFirstUserPreferredUILocaleOrDefault() {
				// NOTE: Log user default locale for information, but use "en-US" as default locale for app
				//       regardless of user default locale (at most cases these locales are the same).
				LOG_DEBUG_D("User default locale:");
				H::GetUserDefaultLocale().Log();

				H::Locale defaultLocale{
					.localName = "en-US",
					.languageCode = "en",
					.scriptCode = "",
					.countryCode = "US"
				};

				LOG_DEBUG_D("User preferred UI locales:");
				auto userPreferredUILocales = H::GetUserPreferredUILocales();
				for (auto& userPreferredUILocale : userPreferredUILocales) {
					userPreferredUILocale.Log();
					LOG_DEBUG_D("");
				}
				return userPreferredUILocales.size() > 0 ? userPreferredUILocales[0] : defaultLocale;
			}
		};



		struct ShellExtensionConfigJSON {
			//static constexpr const char* Filename = "ShellExtensionConfig.txt";
			static constexpr const char* Filename = "ShellExtensionConfig_UTF8_WithBOM.txt";

			ShellExt::ShellExtensionConfig::_Data shellExtensionConfig;

			JS_OBJ(
				shellExtensionConfig
			);

			static ShellExtensionConfigJSON CreateWithDefaultData() {
				return ShellExtensionConfigJSON {
					.shellExtensionConfig = ShellExt::ShellExtensionConfig::GetDataCopy()
				};
			}
			static void AfterLoadHandler(const ShellExtensionConfigJSON& parsedJsonObject) {
				const_cast<ShellExt::ShellExtensionConfig::_Data&>(ShellExt::ShellExtensionConfig::GetDataLocked().Get()) = parsedJsonObject.shellExtensionConfig;
			}
		};

		class AppFeatures : public H::AppFeaturesBase<class AppFeatures> {
			friend _InheritedBase;	

		private:
			AppFeatures();

		public:
			~AppFeatures() = default;

		private:
			bool TranslateConfig();
			bool TranslateShellMenuRecursive(json::ShellMenu& shellMenu);

		private:
			const std::filesystem::path configName;
			const std::filesystem::path configFolder;
		};


		AppFeatures::AppFeatures()
			: configName{ ShellExtensionConfigJSON::Filename }
			, configFolder{ g_TestOutputFolder }
		{
			bool wasSomethingTranslated = false;

			if (this->AppFeaturesBase::LoadConfig<ShellExtensionConfigJSON>(this->configFolder / this->configName)) {
				if (this->TranslateConfig()) {
					LOG_DEBUG_D("\"{}\" translated [{}]:\n"
						"{}"
						, ShellExtensionConfigJSON::Filename, Constants::GetInstance().userLocale.localName
						, ::JS::serializeStruct(ShellExtensionConfig::GetDataLocked().Get())
					);
				}
			}
			else {
				LOG_ERROR_D("Config not loaded");
			}
		}


		bool AppFeatures::TranslateConfig() {
			LOG_FUNCTION_SCOPE("TranslateConfig()");
			LOG_DEBUG_D("userLocale = \"{}\"", Constants::GetInstance().userLocale.localName);

			auto shellExtensionConfig = ShellExtensionConfig::GetDataLocked();
			auto shellExtensionConfigMutable = const_cast<ShellExtensionConfig::_Data*>(&shellExtensionConfig.Get());
			bool wasSomethingTranslated = false;

			for (auto& shellMenu : shellExtensionConfigMutable->shellExtensionSettings.menus) {
				wasSomethingTranslated |= this->TranslateShellMenuRecursive(shellMenu);
			}
			return wasSomethingTranslated;
		}


		bool AppFeatures::TranslateShellMenuRecursive(json::ShellMenu& shellMenu) {
			LOG_FUNCTION_SCOPE(L"TranslateShellMenuRecursive(shellMenu.title = \"{}\")", shellMenu.title);

			auto shellExtensionConfig = ShellExtensionConfig::GetDataLocked();
			auto shellExtensionConfigMutable = const_cast<ShellExtensionConfig::_Data*>(&shellExtensionConfig.Get());
			bool wasSomethingTranslated = false;

			// For example if string = "<translate: My 'title'>" regex must capture "My 'title'"
			auto matches = H::Regex::GetRegexMatches<wchar_t>(shellMenu.title, std::wregex{ L"<translate:[ ]*(.*)>" });
			if (!matches.empty()) {
				auto sourceWString = matches[0].capturedGroups.at(1);
				auto sourceString = H::WStrToStr(sourceWString);
				LOG_DEBUG_D("sourceString = \"{}\"", sourceString);

				auto userLocale = Constants::GetInstance().userLocale;

				// [strict] Try find full locale name.
				auto localeIt = shellExtensionConfig->shellExtensionLocalization.localizationMap.find(userLocale.localName);
				if (localeIt == shellExtensionConfig->shellExtensionLocalization.localizationMap.end()) {
					// [not strict] Try find only main language code.
					localeIt = shellExtensionConfig->shellExtensionLocalization.localizationMap.find(userLocale.languageCode);
				}

				if (localeIt != shellExtensionConfig->shellExtensionLocalization.localizationMap.end()) {
					auto& [__locale, shellExtensionTranslation] = *localeIt;
					LOG_DEBUG_D("suitable locale = \"{}\"", __locale);

					auto sourceStringIt = shellExtensionTranslation.translationMap.find(sourceString);
					if (sourceStringIt != shellExtensionTranslation.translationMap.end()) {
						auto& [__sourceString, translatedString] = *sourceStringIt;
						LOG_DEBUG_D(L"translatedString = \"{}\"", translatedString);
						shellMenu.title = translatedString;
						wasSomethingTranslated = true;
					}
					else {
						LOG_WARNING_D("not found match for sourceString in translationMap");
					}
				}
				else {
					LOG_WARNING_D("not found suitable locale in localizationMap");
				}
			}

			if (!shellMenu.subMenus.empty()) {
				LOG_DEBUG_D("Translate sub menus ...");

				for (auto& subMenu : shellMenu.subMenus) {
					wasSomethingTranslated |= this->TranslateShellMenuRecursive(subMenu);
				}
			}
			return wasSomethingTranslated;
		}

	} // namespace ShellExt


	void TestLoadShellExtensionConfig() {
		LOG_FUNCTION_ENTER("TestLoadShellExtensionConfig()");

		ShellExt::AppFeatures::GetInstance(); // Entry point to load config
		NOOP;
	}
} // namespace TestConfigParsing





int main() {
	_set_error_mode(_OUT_TO_MSGBOX);

	H::Flags<lg::InitFlags> loggerInitFlags =
		lg::InitFlags::DefaultFlags |
		lg::InitFlags::EnableLogToStdout;

	lg::DefaultLoggers::Init(g_TestOutputFolder / (g_TestOutputFolder.stem().string() + ".log"), loggerInitFlags);

	JS::LoggerCallback::Register([](std::string msg) {
		LOG_DEBUG_D("[JsonParser] {}", msg);
		});

	TestConfigParsing::TestLoadShellExtensionConfig();

	//Sleep(15'000);
	system("pause");
	return 0;
}