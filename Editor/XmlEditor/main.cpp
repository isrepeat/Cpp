#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <list>
#include <queue>
#include <utility>


int main() {
	std::wstring inputFilename = L"Test.xml.txt";
	std::wstring outpitFilename = L"TestOut.xml.txt";


	std::list<std::pair<std::wstring, std::wstring>> listReplacedStrings{
		{L"Personal data", L"Hello world"},
		{L"Authorization", L"Parkurist1488"},
	};


	std::vector<std::wstring> lines;
	{
		std::wifstream inFile(inputFilename);
		
		std::queue<std::pair<std::wstring, std::wstring>> replaceNextLinesRules;

		std::wstring line;
		while (std::getline(inFile, line)) {
			if (replaceNextLinesRules.empty()) {
				for (auto& pair : listReplacedStrings) {
					auto& replace = pair.first;
					auto& replaceWith = pair.second;
					if (line.find(pair.first) != std::wstring::npos) {
						replaceNextLinesRules.push({ L"(<translation).+(translation>)", L"$1>" + pair.second+ L"</$2" }); // save 1 and 2 captured group but replace all between
						listReplacedStrings.remove(pair);
						break;
					}
				};
			}
			else {
				auto& pair = replaceNextLinesRules.front();
				line = std::regex_replace(line, std::wregex(pair.first), pair.second);
				replaceNextLinesRules.pop();
			}

			lines.push_back(line);
		}
	}

	{
		std::wofstream outFile(outpitFilename);
		for (const auto& i : lines) {
			outFile << i << std::endl;
		}
	}
	return 0;
}