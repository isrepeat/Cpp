#pragma once
#include "common.h"
#include <string_view>
#include <type_traits>
#include <optional>
#include <string>
#include <vector>
#include <regex>


namespace HELPERS_NS {
    namespace Regex {
		namespace details {
			template<typename CharT>
			using view_t = std::basic_string_view<CharT>;

			template<typename CharT>
			using string_t = std::basic_string<CharT, std::char_traits<CharT>, std::allocator<CharT>>;

			template<typename CharT>
			using regex_t = std::basic_regex<CharT>;

			template<typename CharT>
			using iter_t = typename view_t<CharT>::const_iterator;

			template<typename CharT>
			using match_t = std::match_results<iter_t<CharT>>;

			template<typename OutString, typename View, typename It>
			inline OutString ToOutString(
				View base, // это тот же самый текст, по которому выполнялся regex_search
				const std::sub_match<It>& subMatch
			) {
				using CharT = typename View::value_type;
				
				// Почему используем base:
				// sub_match хранит пару итераторов [first,last) в ИСХОДНОМ тексте. Если на их основе
				// напрямую создать string_view (OutString = basic_string_view), статический анализатор
				// часто ругается: "view из временного объекта" (gsl.view), потому что subMatch — временный.
				// Мы же явно «привязываем» view к живущему base (тот самый буфер/вью, что передали в regex_search).
				// Так мы:
				//   1) Гарантируем корректное время жизни (view ссылается на base.data()).
				//   2) Избавляемся от ложного предупреждения про «висящий» view.
				//
				// Важно: base ДОЛЖЕН ссылаться на тот же буфер, что и использовался в regex_search.
				// Иначе вычисленный pos будет некорректным.
				if constexpr (std::is_same_v<OutString, std::basic_string_view<CharT>>) {
					// Позиция подстроки относительно начала base
					const std::size_t pos = static_cast<std::size_t>(subMatch.first - base.begin());

					return OutString(
						base.data() + pos,
						static_cast<std::size_t>(subMatch.length())
					);
				}
				else {
					// Для "owning"-строки копируем символы из диапазона подстроки.
					// Почему не subMatch.str()? Так мы избегаем лишнего временного std::basic_string.
					return OutString(
						subMatch.first,
						subMatch.first + subMatch.length()
					);
				}
			}
		} // namespace details


		template<typename CharT, typename OutString = std::basic_string_view<CharT>>
		struct RegexMatchResult {
			std::vector<OutString> capturedGroups;
			OutString prefix;
			OutString suffix;

			RegexMatchResult(
				std::basic_string_view<CharT> base,
				const std::match_results<typename std::basic_string_view<CharT>::const_iterator>& match
			) {
				this->capturedGroups.reserve(match.size());
				
				// Все sub_match (включая prefix/suffix) указывают на диапазоны в исходном тексте.
				// Строим OutString через base, чтобы «привязать» view к живому буферу.
				for (const auto& subMatch : match) {
					this->capturedGroups.push_back(details::ToOutString<OutString>(base, subMatch));
				}
				this->prefix = details::ToOutString<OutString>(base, match.prefix());
				this->suffix = details::ToOutString<OutString>(base, match.suffix());
			}
		};


		template<typename CharT, typename OutString = std::basic_string_view<CharT>>
		inline std::vector<RegexMatchResult<CharT, OutString>> GetRegexMatches(
			std::basic_string_view<CharT> text,
			const std::basic_regex<CharT>& rx
		) {
			using It = typename std::basic_string_view<CharT>::const_iterator;
			std::regex_iterator<It> it(text.begin(), text.end(), rx), last;

			std::vector<RegexMatchResult<CharT, OutString>> results;
			for (; it != last; ++it) {
				// Передаём тот же base (text), внутри которого лежат все подстроки, для безопасной сборки view.
				results.emplace_back(text, *it);
			}
			return results;
		}


		template<typename CharT, typename OutString = std::basic_string_view<CharT>>
		inline std::optional<RegexMatchResult<CharT, OutString>> GetRegexMatch(
			std::basic_string_view<CharT> text,
			const std::basic_regex<CharT>& rx
		) {
			using It = typename std::basic_string_view<CharT>::const_iterator;
			std::match_results<It> match;

			if (std::regex_search(text.begin(), text.end(), match, rx)) {
				// Передаём тот же base (text), внутри которого лежат все подстроки, для безопасной сборки view.
				return RegexMatchResult<CharT, OutString>(text, match);
			}
			return std::nullopt;
		}

		

		inline bool FindInsideTagWithRegex(
			std::wstring_view text,
			std::wstring_view tag,
			const std::wregex& innerRx
		) {
			const std::wregex rx(
				std::wstring(L"([^<]*)<") + std::wstring(tag) + L"[^>]*>(.+?)<[/]" + std::wstring(tag) + L">([^<]*)"
			);

			auto matches = GetRegexMatches<wchar_t, std::wstring_view>(text, rx);
			for (auto& match : matches) {
				details::match_t<wchar_t> inner;

				const auto& body = match.capturedGroups.size() > 2 ? match.capturedGroups[2] : std::wstring_view{};
				if (std::regex_search(body.begin(), body.end(), inner, innerRx)) {
					return true;
				}
			}
			return false;
		}


		inline bool FindInsideAnyTagWithRegex(
			std::wstring_view text,
			const std::wregex& innerRx
		) {
			const std::wstring anyTag = L"[^>]*";
			const std::wregex  rx(L"([^<]*)<" + anyTag + L">(.+?)<[/]" + anyTag + L">([^<]*)");

			auto matches = GetRegexMatches<wchar_t, std::wstring_view>(text, rx);
			for (const auto& match : matches) {
				details::match_t<wchar_t> inner;

				const auto& body = match.capturedGroups.size() > 2 ? match.capturedGroups[2] : std::wstring_view{};
				if (std::regex_search(body.begin(), body.end(), inner, innerRx)) {
					return true;
				}
			}
			return false;
		}
    }
}