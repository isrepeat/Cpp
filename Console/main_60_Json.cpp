#include <JsonParser/JsonParser.h>
#include <MagicEnum/MagicEnum.h>
#include <Helpers/JSONConfigLoader.hpp>
#include <Helpers/Config.h>
#include <Helpers/Logger.h>
#include <Helpers/Flags.h>
#include <fstream>
#include <format>
#include <ranges>


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
					JS_MEMBER_ALIASES(activationProtocol, "ActivationProtocol", "activation_protocol")
				);

				ShellBase() = default;
				ShellBase(ShellBaseParams shellBaseParams)
					: ShellBaseParams(shellBaseParams)
				{}
			};

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

				JS_OBJECT(
					JS_MEMBER_ALIASES(shellExtensionSettings, "ShellExtensionSettings", "shell_extension_settings")
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

				json::ShellMenu menuA = { L"MenuA", L"MenuA_command" };
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
				};
			}
		};


		const char jsonShellExtensionConfig[] = R"json(
			{
				"shellExtensionType": "Archivator",
				"menu": {
					"title": "Archivator [Directory]",
					"command": "Pack"
				}
			}
			)json";



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
			, configFolder{ L"D:\\TEST_CONFIGS\\" }
			//, configFolder{ H::ExePath() }
		{
			lg::DefaultLoggers::InitSingleton();

			if (!H::JSONConfigLoader<AppFeatures::JSONObject, AppFeatures>::Load(this->configFolder / this->configName)) {
				LOG_ERROR("Config not loaded");
			}
		}
	} // namespace ShellExt


	void TestLoadShellExtensionConfig() {
		LOG_FUNCTION_ENTER("TestLoadShellExtensionConfig()");

		//ShellExt::json::ShellExtensionSettings shellExtensionSettings;
		//LOG_DEBUG_D("ShellExtensionSettings [src]:\n"
		//	"{}"
		//	, ::JS::serializeStruct(shellExtensionSettings).c_str()
		//);

		//std::string jsonString = ShellExt::jsonShellExtensionConfig;
		//::JS::ParseTo(jsonString.c_str(), jsonString.size(), shellExtensionSettings);

		//LOG_DEBUG_D("ShellExtensionSettings [parsed]:\n"
		//	"{}"
		//	, ::JS::serializeStruct(shellExtensionSettings).c_str()
		//);


		ShellExt::AppFeatures::GetInstance(); // Entry point to load config
		auto shellExtensionConfig = ShellExt::ShellExtensionConfig::GetDataLocked();
		NOOP;
	}
} // namespace TestConfigParsing





int main() {
	H::Flags<lg::InitFlags> loggerInitFlags =
		lg::InitFlags::DefaultFlags |
		lg::InitFlags::EnableLogToStdout;

	lg::DefaultLoggers::Init(L"D:\\main_60_Json.log", loggerInitFlags);

	JS::LoggerCallback::Register([](std::string msg) {
		LOG_DEBUG_D("[JsonParser] {}", msg);
		});

	TestConfigParsing::TestLoadShellExtensionConfig();

	Sleep(15'000);
	return 0;
}