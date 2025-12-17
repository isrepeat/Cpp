#pragma once
#include <Helpers/common.h>
#include <optional>
#include <fstream>
#include <format>
#include <deque>

namespace HELPERS_NS {
	namespace Stream {
		class StreamLineReader {
		public:
			explicit StreamLineReader(
				std::istream& inStream,
				std::size_t historyCapacity = 8)
				: inStream(inStream)
				, historyDeque()
				, peekBuffer(std::nullopt)
				, historyCapacityLimit(historyCapacity > 0 ? historyCapacity : 1) {
			}

			// ¬озвращает следующую строку без сдвига позиции чтени€.
			std::optional<std::string> PeekLine() {
				if (!this->peekBuffer.has_value()) {
					std::string line;
					if (!this->LoadNextLine(line)) {
						return std::nullopt;
					}
					this->peekBuffer = std::move(line);
				}
				return this->peekBuffer;
			}

			// „итает строку, сдвига€ позицию.
			std::optional<std::string> ReadLine() {
				int dummyIndex = 0;
				return this->ReadLine(dummyIndex);
			}

			// „итает строку, сдвига€ позицию. ¬озвращает еЄ индекс через outIndex.
			std::optional<std::string> ReadLine(int& outIndex) {
				std::string line;

				if (this->peekBuffer.has_value()) {
					line = std::move(*this->peekBuffer);
					this->peekBuffer.reset();
				}
				else {
					if (!this->LoadNextLine(line)) {
						return std::nullopt;
					}
				}

				// »ндекс этой строки = количество строк, уже в истории, до добавлени€
				outIndex = this->historyDeque.size();

				this->historyDeque.push_back(line);
				this->EnforceCapacity();
				return line;
			}

			// ѕолучить строку из истории: offset = 0 Ч последн€€ прочитанна€, 1 Ч предпоследн€€ и т.д.
			std::optional<std::string> LookAhead(int offset) const {
				if (offset >= this->historyDeque.size()) {
					return std::nullopt;
				}
				int indexFromStart = this->historyDeque.size() - 1 - offset;
				return this->historyDeque[indexFromStart];
			}

		private:
			bool LoadNextLine(std::string& outLine) {
				if (!std::getline(this->inStream, outLine)) {
					return false;
				}
				this->TrimCarriageReturn(outLine);
				return true;
			}

			void TrimCarriageReturn(std::string& lineString) {
				if (!lineString.empty() && lineString.back() == '\r') {
					lineString.pop_back();
				}
			}

			void EnforceCapacity() {
				while (this->historyDeque.size() > this->historyCapacityLimit) {
					this->historyDeque.pop_front();
				}
			}

		private:
			std::istream& inStream;
			std::deque<std::string> historyDeque;  // истори€ прочитанных строк
			std::optional<std::string> peekBuffer; // строка, загруженна€ PeekLine()
			std::size_t historyCapacityLimit;
		};
	}
}