#include <Helpers/ThreadSafeObject.hpp>
#include <Helpers/JSONConfigLoader.hpp>
#include <JsonParser/JsonParser.h>
#include <MagicEnum/MagicEnum.h>
#include <Helpers/Logger.h>
#include <Helpers/Flags.h>
#include <fstream>
#include <format>
#include <ranges>


#define __JSONPARSER_ENUM_DECLARE_STRING_PARSER(namespaceName, enumName, ...) \
namespace namespaceName { \
	struct js_##enumName##_string_struct \
	{ \
	  template <size_t N> \
	  explicit js_##enumName##_string_struct(const char(&data)[N]) \
	  { \
		JS::Internal::populateEnumNames(_strings, data); \
	  } \
	  std::vector<JS::DataRef> _strings; \
	  \
	  static const std::vector<JS::DataRef>& strings() \
	  { \
		static js_##enumName##_string_struct ret(#__VA_ARGS__); \
		return ret._strings; \
	  } \
	}; \
} \
namespace JS \
{ \
	template <> \
	struct TypeHandler<namespaceName::enumName> \
	{ \
		static inline Error to(namespaceName::enumName& to_type, ParseContext& context) \
		{ \
			return Internal::EnumHandler<namespaceName::enumName, namespaceName::js_##enumName##_string_struct>::to(to_type, context); \
		} \
		static inline void from(const namespaceName::enumName& from_type, Token& token, Serializer& serializer) \
		{ \
			return Internal::EnumHandler<namespaceName::enumName, namespaceName::js_##enumName##_string_struct>::from(from_type, token, serializer); \
		} \
	}; \
}

#define JSONPARSER_ENUM_DECLARE_STRING_PARSER(...) PP_EXPAND(__JSONPARSER_ENUM_DECLARE_STRING_PARSER(__VA_ARGS__))

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
			struct ShellBase {
				enums::ShellExtensionType shellExtensionType = enums::ShellExtensionType::Unknown;

				JS_OBJECT(
					JS_MEMBER_ALIASES(shellExtensionType, "ShellExtensionType", "shell_extension_type")
				);
			};

			struct ShellMenu {
				std::wstring title;
				std::wstring command;
				//std::vector<ShellMenu> subMenus;

				JS_OBJECT(
					JS_MEMBER_ALIASES(title, "Title"),
					JS_MEMBER_ALIASES(command, "Command")
					//JS_MEMBER_ALIASES(subMenus, "SubMenus", "sub_menus")
				);

			};

			struct ShellExtensionSettings : ShellBase {
				ShellMenu menu;

				JS_OBJECT_WITH_SUPER(
					JS_SUPER(ShellBase),
					JS_MEMBER_ALIASES(menu, "Menu")
				);

				ShellExtensionSettings(ShellMenu menu)
					: menu{ menu }
				{}
			};
		} // namespace json


		class ShellExtensionConfig {
		private:
			static ShellExtensionConfig& GetInstance();
			ShellExtensionConfig();
		public:
			~ShellExtensionConfig() = default;

			struct Data {
				json::ShellExtensionSettings shellExtensionSettings;

				JS_OBJECT(
					JS_MEMBER_ALIASES(shellExtensionSettings, "ShellExtensionSettings", "shell_extension_settings")
				);
			};

			// TODO: move to base class
			static H::ThreadSafeObject<std::recursive_mutex, const Data>::_Locked GetDataLocked();
			static Data GetDataCopy();

		private:
			H::ThreadSafeObject<std::recursive_mutex, const Data> data;
		};

		ShellExtensionConfig& ShellExtensionConfig::GetInstance() {
			static ShellExtensionConfig instance;
			return instance;
		}

		ShellExtensionConfig::ShellExtensionConfig()
			: data{
				json::ShellExtensionSettings{
					json::ShellMenu{L"defaultTitle", L"defaultCommand"}
				},
			}
		{
		}

		H::ThreadSafeObject<std::recursive_mutex, const ShellExtensionConfig::Data>::_Locked ShellExtensionConfig::GetDataLocked() {
			return GetInstance().data.Lock();
		}

		ShellExtensionConfig::Data ShellExtensionConfig::GetDataCopy() {
			auto locked = GetDataLocked();
			auto copy = locked.Get();
			return copy;
		}

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

				ShellExt::ShellExtensionConfig::Data shellExtensionConfig = ShellExt::ShellExtensionConfig::GetDataCopy();

				JS_OBJ(
					shellExtensionConfig
				);

				static void AfterLoadHandler(const JSONObject& jsonObject) {
					const_cast<ShellExt::ShellExtensionConfig::Data&>(ShellExt::ShellExtensionConfig::GetDataLocked().Get()) = jsonObject.shellExtensionConfig;
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