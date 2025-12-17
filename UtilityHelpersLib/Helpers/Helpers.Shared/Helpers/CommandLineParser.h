#pragma once
#include "common.h"
#include "Meta/FunctionTraits.h"
#include "Logger.h"

#include <unordered_map>
#include <string_view>
#include <stdexcept>
#include <optional>
#include <string>
#include <format>
#include <vector>

namespace HELPERS_NS {
	template <typename TChar>
	class CommandLineParserBase {
	public:
		using string_t = std::basic_string<TChar>;
		using string_view_t = std::basic_string_view<TChar>;

		enum class ExpectedValuesCount {
			None,
			Single,
			Multiple
		};

		struct FlagDesc {
			string_t name;
			ExpectedValuesCount expectedValuesCount = ExpectedValuesCount::None;
			bool required = false;
		};

		struct ParsedFlagData {
			FlagDesc flagDesc;
			std::vector<string_t> values;
		};

	public:
		template<typename TParsed>
		__requires_expr(
			HELPERS_NS::meta::concepts::has_static_function_with_signature<
			decltype(&TParsed::Description), std::vector<FlagDesc>(*)()
			>&&
			HELPERS_NS::meta::concepts::has_static_function_with_signature<
			decltype(&TParsed::MapParsedResults), void (*)(const CommandLineParserBase<TChar>&, TParsed&)
			>
		) static const TParsed ParseTo(
			int argc,
			typename string_t::value_type* argv[]
		) {
			const auto flagDescs = TParsed::Description();

			CommandLineParserBase<TChar> cmdLineParser{ flagDescs };
			cmdLineParser.Parse(argc, argv);

			TParsed out{};
			TParsed::MapParsedResults(cmdLineParser, out);

			if constexpr (meta::concepts::has_method_with_signature<
				decltype(&TParsed::Validate), void (TParsed::*)() const
			>) {
				out.Validate();
			}

			return out;
		}

		CommandLineParserBase(const std::vector<FlagDesc>& flagDescs) {
			for (const auto& flagDesc : flagDescs) {
				this->mapFlagNameToDesc[flagDesc.name] = flagDesc;
			}
		}

		void Parse(int argc, TChar* argv[]) {
			for (int i = 1; i < argc; ++i) {
				string_t arg = argv[i];

				if (!arg.empty() && arg[0] == static_cast<TChar>('-')) {
					auto it = this->mapFlagNameToDesc.find(arg);
					if (it == this->mapFlagNameToDesc.end()) {
						// Без форматирования имени, чтобы не запутываться с char/wchar_t
						throw std::runtime_error{ "Unknown option" };
					}

					auto& parsed = this->mapFlagNameToParsedData[arg];
					parsed.flagDesc = it->second;

					if (parsed.flagDesc.expectedValuesCount == ExpectedValuesCount::None) {
						// Просто флаг
					}
					else {
						if (i + 1 >= argc) {
							throw std::runtime_error{ "Option requires value" };
						}

						// Следующий токен не должен начинаться с '-'
						if (argv[i + 1][0] == static_cast<TChar>('-')) {
							throw std::runtime_error{ "Option requires value" };
						}

						++i;
						parsed.values.push_back(argv[i]);

						// Для Multiple допускаем повтор: -f A -f B -f C
						// (повторное появление флага будет обработано на следующей итерации цикла)
					}
				}
				else {
					this->positionalArgs.push_back(arg);
				}
			}

			// Проверяем обязательные
			for (const auto& [name, flagDesc] : this->mapFlagNameToDesc) {
				if (!flagDesc.required) {
					continue;
				}
				if (this->mapFlagNameToParsedData.find(name) == this->mapFlagNameToParsedData.end()) {
					throw std::runtime_error{ "Missing required option" };
				}
			}
		}

		bool Has(string_view_t name) const {
			auto it = this->mapFlagNameToParsedData.find(string_t{ name });
			if (it == this->mapFlagNameToParsedData.end()) {
				return false;
			}
			return true;

			// Возвращает true только для флагов без зависимых значений (т.е. ExpectedValuesCount::None)
			//return it->second.values.empty();
		}

		std::optional<string_t> Get(string_view_t name) const {
			auto it = this->mapFlagNameToParsedData.find(string_t{ name });
			if (it == this->mapFlagNameToParsedData.end()) {
				return std::nullopt;
			}
			if (it->second.values.empty()) {
				return std::nullopt;
			}
			return it->second.values.front();
		}

		std::vector<string_t> GetAll(string_view_t name) const {
			auto it = this->mapFlagNameToParsedData.find(string_t{ name });
			if (it == this->mapFlagNameToParsedData.end()) {
				return {};
			}
			return it->second.values;
		}

		const std::vector<string_t>& GetPositional() const {
			return this->positionalArgs;
		}

	private:
		std::vector<string_t> positionalArgs;
		std::unordered_map<string_t, FlagDesc> mapFlagNameToDesc;
		std::unordered_map<string_t, ParsedFlagData> mapFlagNameToParsedData;
	};

	using CommandLineParserA = CommandLineParserBase<char>;
	using CommandLineParserW = CommandLineParserBase<wchar_t>;
}