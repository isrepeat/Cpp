#pragma once
#include "Helpers/common.h"
#include "Helpers/Logger.h"
#include "Helpers/Stream.h"
#include "Helpers/Regex.h"

#include <optional>
#include <fstream>
#include <string>
#include <vector>

namespace STD_EXT_NS {
	enum class UTFByteOrderMark {
		UTF8,
		UTF16_BE,
		UTF16_LE,
		UTF32_BE,
		UTF32_LE
	};


	struct ifstream : public ::std::ifstream {
		using ::std::ifstream::basic_ifstream;

		static const inline ::std::unordered_map<UTFByteOrderMark, ::std::vector<uint8_t>> KnownUTFByteOrderMarks = {
			{ UTFByteOrderMark::UTF8, { 0xEF, 0xBB, 0xBF } },
			{ UTFByteOrderMark::UTF16_BE, { 0xFE, 0xFF } },
			{ UTFByteOrderMark::UTF16_LE, { 0xFF, 0xFE } },
			{ UTFByteOrderMark::UTF32_BE, { 0x00, 0x00, 0xFE, 0xFF } },
			{ UTFByteOrderMark::UTF32_LE, { 0xFF, 0xFE, 0x00, 0x00 } },
		};

		struct Data {
			::std::vector<char> byteArray;
			::std::optional<int> codePage;
		};


		Data ReadData() {
			HELPERS_NS::Stream::ThrowIfStreamNotOpened(*this, "ReadData");
			HELPERS_NS::Stream::ThrowIfStreamFailed(*this, "ReadData");

			auto& ifstreamRef = static_cast<::std::ifstream&>(*this);

			this->SkipUTFByteOrderMark();

			Data data;
			data.codePage = this->DetectCodePageInFirstLineOfStream();

			data.byteArray = ::std::vector<char>{
				::std::istreambuf_iterator<char>(ifstreamRef),
				::std::istreambuf_iterator<char>()
			};
			return data;
		}


		::std::optional<UTFByteOrderMark> DetectUTFByteOrderMark() {
			::std::istream::pos_type originalPos = this->tellg();
			if (originalPos != 0) {
				throw std::runtime_error("DetectUTFByteOrderMark must be called at the beginning of the stream");
			}

			// Читаем максимум N байт (по самой длинной сигнатуре BOM)
			size_t maxLength = 0;
			for (const auto& [_, bom] : KnownUTFByteOrderMarks) {
				maxLength = std::max(maxLength, bom.size());
			}

			::std::vector<uint8_t> buffer(maxLength);
			this->read(reinterpret_cast<char*>(buffer.data()), static_cast<::std::streamsize>(maxLength));
			HELPERS_NS::Stream::ThrowIfStreamFailed(*this, "DetectUTFByteOrderMark");

			auto readBytes = this->gcount();

			this->seekg(originalPos);

			// Проверяем наличие любого из BOM
			for (const auto& [bomType, bom] : KnownUTFByteOrderMarks) {
				if (static_cast<size_t>(readBytes) < bom.size()) {
					continue; // недостаточно байт для этого BOM
				}

				if (::std::equal(bom.begin(), bom.end(), buffer.begin())) {
					return bomType;
				}
			}

			return ::std::nullopt;
		}

	private:
		void SkipUTFByteOrderMark() {
			::std::istream::pos_type originalPos = this->tellg();
			if (originalPos != 0) {
				throw std::runtime_error("SkipUTFByteOrderMarkInStream must be called at the beginning of the stream (tellg() == 0)");
			}

			auto bomType = this->DetectUTFByteOrderMark();
			if (!bomType) {
				return;
			}

			const auto& bom = KnownUTFByteOrderMarks.at(*bomType);
			this->seekg(static_cast<std::streamoff>(bom.size()), std::ios::beg); // Пропускаем UTF BOM
			HELPERS_NS::Stream::ThrowIfStreamFailed(*this, "SkipUTFByteOrderMark");
		}


		::std::optional<int> DetectCodePageInFirstLineOfStream() {
			auto prevFileStreamPointerPos = this->tellg();
			this->seekg(0, ::std::ios::beg);

			::std::string firstLine;
			if (::std::getline(*this, firstLine)) {
				auto matches = HELPERS_NS::Regex::GetRegexMatches<char, std::string>(firstLine, ::std::regex("CodePage\\s*=\\s*(\\d+)"));
				if (!matches.empty()) {
					if (matches[0].capturedGroups.size() > 1) {
						// Do not recover file stream pointer pos.
						return ::std::stoi(matches[0].capturedGroups[1]);
					}
				}
			}

			this->seekg(prevFileStreamPointerPos, ::std::ios::beg);
			return ::std::nullopt;
		}
	};



	struct ofstream : public ::std::ofstream {
		using ::std::ofstream::ofstream;

		void WriteUTFByteOrderMark(UTFByteOrderMark bomType = UTFByteOrderMark::UTF8) {
			const auto& bom = ifstream::KnownUTFByteOrderMarks.at(bomType);
			this->write(reinterpret_cast<const char*>(bom.data()), static_cast<std::streamsize>(bom.size()));
			HELPERS_NS::Stream::ThrowIfStreamFailed(*this, "WriteUTF8ByteOrderMark");
		}
	};
}