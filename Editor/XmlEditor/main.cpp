#include <filesystem>
#include <functional>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <list>
#include <queue>
#include <utility>


void ReadFileByLine(const std::wstring& inputFilename, std::function<void(std::wstring)> callback) {
	std::wifstream inFile(inputFilename);

	std::wstring line;
	while (std::getline(inFile, line)) {
		callback(line);
	}
}

std::vector<std::wstring> ReadFileWithReplacedLines(const std::wstring& inputFilename, std::list<std::pair<std::wstring, std::wstring>> listReplacedStrings = {}) {
	std::vector<std::wstring> lines;
	std::queue<std::pair<std::wstring, std::wstring>> replaceNextLinesWithRules;

	ReadFileByLine(inputFilename, [&](std::wstring line) {
		if (replaceNextLinesWithRules.empty()) {
			for (auto& pair : listReplacedStrings) {
				auto& replace = pair.first;
				auto& replaceWith = pair.second;
				if (line.find(pair.first) != std::wstring::npos) {
					replaceNextLinesWithRules.push({ L"(<translation).+(translation>)", L"$1>" + pair.second + L"</$2" }); // save 1 and 2 captured group but replace all between
					listReplacedStrings.remove(pair);
					break;
				}
			};
		}
		else {
			auto& pair = replaceNextLinesWithRules.front();
			line = std::regex_replace(line, std::wregex(pair.first), pair.second);
			replaceNextLinesWithRules.pop();
		}

		lines.push_back(line);
		});

	return lines;
}

void WriteFile(const std::wstring& outputFilename, const std::vector<std::wstring>& lines) {
	std::wofstream outFile(outputFilename);
	for (const auto& i : lines) {
		outFile << i << std::endl;
	}
}


int main() {
	std::list<std::pair<std::wstring, std::wstring>> listReplacedStrings;

	ReadFileByLine(L"Original.txt", [&listReplacedStrings] (std::wstring originalString) {
		listReplacedStrings.push_back({ originalString, L"" });
		});


	auto it = listReplacedStrings.begin();
	ReadFileByLine(L"Replaced.txt", [&it, &listReplacedStrings](std::wstring replacedString) {
		if (it != listReplacedStrings.end()) {
			it->second = replacedString;
			it++;
		}
		});


	auto lines = ReadFileWithReplacedLines(L"Test.xml.txt", listReplacedStrings);
	WriteFile(L"TestOut.xml.txt", lines);

	return 0;
}