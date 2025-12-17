#pragma once
#include <Helpers/common.h>
#include "StreamHelpers.h"

#include <filesystem>
#include <optional>
#include <fstream>
#include <format>
#include <deque>
#include <span>

namespace HELPERS_NS {
	namespace Stream {
		class StreamLineViewer {
		public:
			StreamLineViewer(std::istream& inStream) {
				this->buffer = this->ReadAll(inStream);
				this->lines = this->SplitToLines(this->buffer);
			}

			StreamLineViewer(const std::filesystem::path& filePath) {
				std::ifstream inStream(filePath, std::ios::binary);
				HELPERS_NS::Stream::ThrowIfStreamNotOpened(inStream);

				this->buffer = this->ReadAll(inStream);
				this->lines = this->SplitToLines(this->buffer);
			}

			StreamLineViewer(const StreamLineViewer&) = delete;
			StreamLineViewer& operator=(const StreamLineViewer&) = delete;

			StreamLineViewer(StreamLineViewer&&) noexcept = default;
			StreamLineViewer& operator=(StreamLineViewer&&) noexcept = default;

			std::span<const std::string_view> GetAllLines() const {
				return std::span<const std::string_view>(
					this->lines.data(),
					this->lines.size()
				);
			}

			std::string_view GetLine(std::size_t index) const {
				if (index >= this->lines.size()) {
					return std::string_view{};
				}

				return this->lines[index];
			}

			std::span<const std::string_view> GetLinesSpan(
				std::size_t start,
				std::size_t end
			) const {
				if (start > end) {
					end = start;
				}

				if (start > this->lines.size()) {
					start = this->lines.size();
				}

				if (end > this->lines.size()) {
					end = this->lines.size();
				}

				const std::size_t count = end - start;

				return std::span<const std::string_view>(
					this->lines.data() + start,
					count
				);
			}

		private:
			std::string ReadAll(std::istream& inStream) {
				std::string outBuffer;
				inStream.seekg(0, std::ios::end);

				std::streampos endPos = inStream.tellg();
				if (endPos < 0) {
					return {};
				}

				const std::size_t size = static_cast<std::size_t>(endPos);
				outBuffer.resize(size);

				inStream.seekg(0, std::ios::beg);

				if (size > 0) {
					inStream.read(outBuffer.data(), static_cast<std::streamsize>(size));
				}

				return outBuffer;
			}

			std::vector<std::string_view> SplitToLines(const std::string& buffer) {
				std::vector<std::string_view> outLines;

				const char* const begin = buffer.data();
				const char* const end = begin + buffer.size();

				const char* lineStart = begin;

				for (const char* p = begin; p != end; ++p) {
					if (*p == '\n') {
						std::string_view sv(
							lineStart,
							static_cast<std::size_t>(p - lineStart)
						);

						this->TrimCarriageReturn(sv);
						outLines.emplace_back(sv);

						lineStart = p + 1;
					}
				}

				if (lineStart < end) {
					std::string_view sv(
						lineStart,
						static_cast<std::size_t>(end - lineStart)
					);

					this->TrimCarriageReturn(sv);
					outLines.emplace_back(sv);
				}

				return outLines;
			}

			void TrimCarriageReturn(std::string_view& sv) {
				if (!sv.empty()) {
					if (sv.back() == '\r') {
						sv.remove_suffix(1);
					}
				}
			}

		private:
			std::string buffer;
			std::vector<std::string_view> lines;
		};
	}
}