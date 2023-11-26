//#include <Windows.h>
//#include <filesystem>
//#include <functional>
//#include <iostream>
//#include <fstream>
//#include <vector>
//#include <string>
//#include <regex>
//#include <list>
//#include <map>
//#include <queue>
//#include <utility>
//#include "../../../Shared/Helpers/Regex.h"
//
//#define TEST_WITHOUT_ARGS 1
//
//#ifndef _DEBUG
//#define TEST_WITHOUT_ARGS 0
//#endif
//
//
//std::wstring EncodeSpecialSymbols(const std::wstring str) {
//	std::wstring res = std::regex_replace(str, std::wregex(L"[<]|[>]|[&]|[']|[\"]"), L"[__$&__]");
//	res = std::regex_replace(res, std::wregex(L"\\[__<__\\]"), L"&lt;");
//	res = std::regex_replace(res, std::wregex(L"\\[__>__\\]"), L"&gt;");
//	res = std::regex_replace(res, std::wregex(L"\\[__&__\\]"), L"&amp;");
//	res = std::regex_replace(res, std::wregex(L"\\[__'__\\]"), L"&apos;");
//	res = std::regex_replace(res, std::wregex(L"\\[__\"__\\]"), L"&quot;");
//
//	return res;
//}
//
//void ReadFileByLine(const std::wstring& inputFilename, std::function<void(std::wstring)> callback) {
//	std::wifstream inFile(inputFilename);
//
//	std::wstring line;
//	while (std::getline(inFile, line)) {
//		callback(line);
//	}
//}
//
//std::vector<std::wstring> ReadFileWithReplacedLines(const std::wstring& inputFilename, std::list<std::pair<std::wstring, std::wstring>> listReplacedStrings = {}) {
//	std::vector<std::wstring> lines;
//	std::queue<std::pair<std::wstring, std::wstring>> replaceNextLinesWithRules;
//
//	ReadFileByLine(inputFilename, [&](std::wstring line) {
//		if (replaceNextLinesWithRules.empty()) {
//			for (auto& pair : listReplacedStrings) {
//				auto& replace = pair.first;
//				auto& replaceWith = pair.second;
//
//				if (line.find(pair.first) != std::wstring::npos) { // fast check
//					if (H::FindInsideAnyTagWithRegex(line, std::wregex(L"^" + pair.first + L"$"))) { // deep validation
//						replaceNextLinesWithRules.push({ L"(<translation).+(translation>)", L"$1>" + pair.second + L"</$2" }); // save 1 and 2 captured group but replace all between
//						listReplacedStrings.remove(pair);
//						break;
//					}
//				}
//			};
//		}
//		else {
//			auto& pair = replaceNextLinesWithRules.front();
//			line = std::regex_replace(line, std::wregex(pair.first), pair.second);
//			replaceNextLinesWithRules.pop();
//		}
//
//		lines.push_back(line);
//		});
//
//	return lines;
//}
//
//void WriteFile(const std::wstring& outputFilename, const std::vector<std::wstring>& lines) {
//	std::wofstream outFile(outputFilename);
//	for (const auto& i : lines) {
//		outFile << i << std::endl;
//	}
//}
//
//
//
//int wmain(int argc, wchar_t* argv[]) {
//#if TEST_WITHOUT_ARGS == 1
//	std::wstring originalFilename = L"rd_manager_ru.ts";
//	std::wstring outputFilename = L"rd_manager_ru.txt";
//	std::wstring originalStringsFilename = L"OriginalStrings.txt";
//	std::wstring replacedStringsFilename = L"ReplacedStrings_ru.txt";
//#else
//	for (int i = 0; i < argc; i++) {
//		wprintf(L"arg [%d] = %s \n", i, argv[i]);
//	}
//	wprintf(L"\n\n");
//
//	if (argc != 3 && argc != 5) {
//		wprintf(L"Usage \"XmlEditor.exe <Original.xml> <Output.xml> \"\n");
//		wprintf(L"or \n");
//		wprintf(L"Usage \"XmlEditor.exe <Original.xml> <Output.xml> <OriginalStrings.txt> <ReplacedStrings.txt> \"\n");
//		Sleep(1000);
//		return 1;
//	}
//
//	std::wstring originalFilename = argv[1];
//	std::wstring outputFilename = argv[2];
//	std::wstring originalStringsFilename = argc > 3 ? argv[3] : L"OriginalStrings.txt";
//	std::wstring replacedStringsFilename = argc > 3 ? argv[4] : L"ReplacedStrings.txt";
//#endif
//
//	try {
//		std::list<std::pair<std::wstring, std::wstring>> listReplacedStrings;
//
//		ReadFileByLine(originalStringsFilename, [&listReplacedStrings](std::wstring originalString) {
//			listReplacedStrings.push_back({ EncodeSpecialSymbols(originalString), L"" });
//			});
//
//
//		auto it = listReplacedStrings.begin();
//		ReadFileByLine(replacedStringsFilename, [&it, &listReplacedStrings](std::wstring replacedString) {
//			if (it != listReplacedStrings.end()) {
//				it->second = EncodeSpecialSymbols(replacedString);
//				it++;
//			}
//			});
//
//
//		auto lines = ReadFileWithReplacedLines(originalFilename, listReplacedStrings);
//		WriteFile(outputFilename, lines);
//	}
//	catch (...) {
//		wprintf(L"\n\n");
//		wprintf(L"was exception ... \n");
//		Sleep(1000);
//		return 1;
//	}
//	return 0;
//}