#include "StacktraceRestorer.h"
#include "PrintSymbol.h"
#include "../../../../Shared/Helpers/Channel.h"
#include "../../../../Shared/Helpers/Helpers.h"
#include <diacreate.h> // include after Windows.h
#include <filesystem>
#include <functional>
#include <algorithm>
#include <iostream>
#include <fstream>

#include <dbghelp.h>
#pragma comment (lib, "dbghelp.lib")

std::wstring miniDumpDirectory = L""; // set in RegisterVectoredExceptionHandler()

//#define PRINT_INFO

namespace StacktraceRestorer {
	bool LoadDataFromPdb(const std::wstring& pdbFile, CComPtr<IDiaDataSource>& ppSource, CComPtr<IDiaSession>& ppSession, CComPtr<IDiaSymbol>& ppGlobal) {

		HRESULT hr = CoInitialize(NULL);

		// Obtain access to the provider
		//hr = CoCreateInstance(__uuidof(DiaSource), NULL, CLSCTX_INPROC_SERVER, __uuidof(IDiaDataSource), (void**)(&ppSource));
#ifdef _WIN64
		auto msDia = H::ExePathW() + L"\\msdia140_x64.dll";
#else
		auto msDia = H::ExePathW() + L"\\msdia140_x86.dll";
#endif
		// Use dll manually without registration COM
		hr = NoRegCoCreate(msDia.c_str(), _uuidof(DiaSourceAlt), _uuidof(IDiaDataSource), (void**)&ppSource);
		if (FAILED(hr)) {
#ifdef PRINT_INFO
			wprintf(L"CoCreateInstance failed - HRESULT = %08X\n", hr);
#endif
			return false;
		}


		hr = ppSource->loadDataFromPdb(pdbFile.c_str());
		if (FAILED(hr)) {
#ifdef PRINT_INFO
			wprintf(L"loadDataFromPdb failed - HRESULT = %08X\n", hr);
#endif
			return false;
		}


		// Open a session for querying symbols
		hr = ppSource->openSession(&ppSession);
		if (FAILED(hr)) {
#ifdef PRINT_INFO
			wprintf(L"openSession failed - HRESULT = %08X\n", hr);
#endif
			return false;
		}

		// Retrieve a reference to the global scope
		hr = ppSession->get_globalScope(&ppGlobal);
		if (hr != S_OK) {
			wprintf(L"get_globalScope failed\n");
			return false;
		}

		return true;
	}


	std::vector<SymInfo> GetPublicSymbols(CComPtr<IDiaSession> pSession, CComPtr<IDiaSymbol> pGlobal) {

		// Retrieve all the public symbols
		CComPtr<IDiaEnumSymbols> pEnumSymbols;
		if (FAILED(pGlobal->findChildren(SymTagPublicSymbol, NULL, nsNone, &pEnumSymbols))) {
			return {};
		}

		std::vector<SymInfo> symbolsInfo;

		ULONG celt = 0;
		IDiaSymbol* pSymbol;
		while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol, &celt)) && (celt == 1)) {
			if (auto sym = GetPublicSymbolInfo(pSession, pSymbol)) {
				symbolsInfo.push_back(*sym);
			}
			pSymbol->Release();
		}

		return symbolsInfo;
	}


	std::unique_ptr<LineInfo> FindLineByRVA(const SymInfo& targetSymbol, std::uint32_t RVA) {

		for (int j = 0; j < (int)targetSymbol.lines.size() - 1; j++) {
			auto& leftLine = targetSymbol.lines[j];
			auto& rightLine = targetSymbol.lines[j + 1];

			if (leftLine.RVA <= RVA && RVA <= rightLine.RVA) {
				auto& targetLine = RVA == rightLine.RVA ? rightLine : leftLine;
				return std::make_unique<LineInfo>(targetLine);
			}
		}

		return nullptr;
	}





	API Backtrace BackTrace(int SkipFrames) {
		constexpr int TRACE_MAX_STACK_FRAMES = 99;
		void* stack[TRACE_MAX_STACK_FRAMES];
		ULONG hash;

		////int numFrames = CaptureStackBackTrace(SkipFrames + 1, TRACE_MAX_STACK_FRAMES, stack, &hash);

		int attempts = 3;
		int numFrames = 0;
		while ((numFrames = CaptureStackBackTrace(SkipFrames + 1, TRACE_MAX_STACK_FRAMES, stack, &hash)) == 0) {
			if (--attempts == 0)
				break;
#ifdef PRINT_INFO
			printf("numFrames == 0, try again [attempts left %d]", attempts);
#endif
		}

		if (numFrames == 0) {
			//printf("BackTrace(int) numFrames = 0\n");
			return {};
		}

		wchar_t buff[1024];
		swprintf_s(buff, L"Stack hash: 0x%08lx\n", hash);

		Backtrace backtrace;
		std::wstring prevModuleFilename = L"";

		for (int i = 0; i < numFrames; ++i) {
			void* moduleBaseVoid = nullptr;
			RtlPcToFileHeader(stack[i], &moduleBaseVoid);
			auto moduleBase = (const unsigned char*)moduleBaseVoid;
			constexpr auto MODULE_BUF_SIZE = 4096U;
			wchar_t modulePath[MODULE_BUF_SIZE];
			//const wchar_t* moduleFilename = modulePath;
			if (moduleBase != nullptr) {
				GetModuleFileNameW((HMODULE)moduleBase, modulePath, MODULE_BUF_SIZE);
				std::wstring moduleFilepath{ modulePath };

				int n = moduleFilepath.rfind(L"\\");
				if (n != std::wstring::npos) {
					auto moduleFilename = moduleFilepath.substr(n + 1);
					if (moduleFilename != prevModuleFilename) {
						prevModuleFilename = moduleFilename;
						backtrace.push_back({ moduleFilename, {} });
					}
					//auto RVA = std::uint32_t((unsigned char*)stack[i] - moduleBase);
					backtrace.back().second.push_back({ moduleFilename, std::uint32_t((unsigned char*)stack[i] - moduleBase) });
				}
				//else {
				//    result += WStrPrintFormat(L"%02d:%s+0x%016llx\n", i, moduleFilename.c_str(),
				//        (std::uint64_t)stack[i]);
				//}
			}
		}
		return backtrace;
	}


	std::vector<StackFrame> RestoreStackTraceViaPDB(const std::wstring pdbFile, const std::vector<BacktraceFrame>& backtrace) {

		CComPtr<IDiaDataSource> diaDataSource;
		CComPtr<IDiaSession> diaSession;
		CComPtr<IDiaSymbol> globalSymbol;

		if (!LoadDataFromPdb(pdbFile, diaDataSource, diaSession, globalSymbol)) {
#ifdef PRINT_INFO
			wprintf(L"can't load pdb = %s\n", pdbFile.c_str());
#endif
			//throw std::exception("can't load pdb");
			return {};
		}

		auto symbols = GetPublicSymbols(diaSession, globalSymbol);
		std::sort(symbols.begin(), symbols.end(), [](const SymInfo& left, const SymInfo& right) {
			return left.RVA < right.RVA;
			});

		if (symbols.size() == 0) {
#ifdef PRINT_INFO
			wprintf(L"unnable to get symbols \n");
#endif
			//throw std::exception("unnable to get symbols");
			return {};
		}

#ifdef PRINT_INFO
		for (auto& frame : backtrace) {
			wprintf(L"%s + 0x%08x \n", frame.moduleName.c_str(), frame.RVA);
		}
		wprintf(L"\n\n");
#endif

		std::vector<StackFrame> stackTrace;

		for (auto& btFrame : backtrace) {
			bool found = false;

			for (int i = 0; i < (int)symbols.size() - 1; i++) {
				auto& leftSym = symbols[i];
				auto& rightSym = symbols[i + 1];

				if (leftSym.RVA <= btFrame.RVA && btFrame.RVA < rightSym.RVA) {
					if (leftSym.lines.size() > 0) {
						LineInfo foundLine;

						if (auto line = FindLineByRVA(leftSym, btFrame.RVA)) {
							foundLine = *line;
						}
						else {
							foundLine = leftSym.lines.back();
						}

						stackTrace.push_back({ leftSym.name, H::FS::GetFilenameFromPathW(foundLine.filename), foundLine.lineNumber, foundLine.RVA });
					}
					else {
						stackTrace.push_back({ leftSym.name, L"<file ...>", 0, 0 });  // if have not lines info push only symbol info
					}
					found = true;
					break;
				}
			}

			if (!found) { // check last symbol
				auto& lastSymbol = symbols.back();
				if (lastSymbol.lines.size() > 0) {
					LineInfo foundLine;

					if (auto line = FindLineByRVA(lastSymbol, btFrame.RVA)) {
						foundLine = *line;
					}
					else {
						foundLine = lastSymbol.lines.back();
					}

					stackTrace.push_back({ lastSymbol.name, H::FS::GetFilenameFromPathW(foundLine.filename), foundLine.lineNumber, foundLine.RVA });
				}
				else {
					stackTrace.push_back({lastSymbol.name, L"???", 0, 0 }); // use lastSymbol name as assumption
				}
			}
		}

		return stackTrace;
	}

	API std::vector<StackFrame> BuildStacktrace() {
		std::vector<StacktraceRestorer::StackFrame> stacktrace;
		std::wstring pdbFolder = H::ExePathW() + L"\\";

		auto backtrace = StacktraceRestorer::BackTrace(0);
		for (auto& [moduleName, moduleBacktrace] : backtrace) {
			auto pdbName = H::FS::RemoveExtFromFilenameW(moduleName) + L".pdb";
			auto stackFrames = RestoreStackTraceViaPDB(pdbFolder + pdbName, moduleBacktrace);
			stacktrace.insert(stacktrace.end(), stackFrames.begin(), stackFrames.end());
		}

		return stacktrace;
	}

	API std::vector<StackFrame> BuildStacktrace(const Backtrace& backtrace) {
		std::vector<StacktraceRestorer::StackFrame> stacktrace;
		std::wstring pdbFolder = H::ExePathW() + L"\\";

		for (auto& [moduleName, moduleBacktrace] : backtrace) {
			auto pdbName = H::FS::RemoveExtFromFilenameW(moduleName) + L".pdb";
			auto stackFrames = RestoreStackTraceViaPDB(pdbFolder + pdbName, moduleBacktrace);
			stacktrace.insert(stacktrace.end(), stackFrames.begin(), stackFrames.end());
		}

		return stacktrace;
	}
}