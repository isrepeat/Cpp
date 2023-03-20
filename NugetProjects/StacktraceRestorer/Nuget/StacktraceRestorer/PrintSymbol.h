#pragma once
#include <memory>
#include <dia2.h>
#include <atlbase.h>

namespace StacktraceRestorer {
	struct LineInfo {
		std::uint32_t RVA = 0;
		std::uint32_t lineNumber = 0;
		std::wstring filename;
	};

	struct SymInfo {
		std::uint32_t RVA = 0;
		std::wstring name;
		std::vector<LineInfo> lines;
	};

	void PrintPublicSymbolWithLines(CComPtr<IDiaSession> pSession, CComPtr<IDiaSymbol> pSymbol);
	std::unique_ptr<SymInfo> GetPublicSymbolInfo(CComPtr<IDiaSession> pSession, CComPtr<IDiaSymbol> pSymbol);
}