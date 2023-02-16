#include <string>
#include <vector>
#include "PrintSymbol.h"

// Basic types
const wchar_t* const rgBaseType[] =
{
  L"<NoType>",                         // btNoType = 0,
  L"void",                             // btVoid = 1,
  L"char",                             // btChar = 2,
  L"wchar_t",                          // btWChar = 3,
  L"signed char",
  L"unsigned char",
  L"int",                              // btInt = 6,
  L"unsigned int",                     // btUInt = 7,
  L"float",                            // btFloat = 8,
  L"<BCD>",                            // btBCD = 9,
  L"bool",                             // btBool = 10,
  L"short",
  L"unsigned short",
  L"long",                             // btLong = 13,
  L"unsigned long",                    // btULong = 14,
  L"__int8",
  L"__int16",
  L"__int32",
  L"__int64",
  L"__int128",
  L"unsigned __int8",
  L"unsigned __int16",
  L"unsigned __int32",
  L"unsigned __int64",
  L"unsigned __int128",
  L"<currency>",                       // btCurrency = 25,
  L"<date>",                           // btDate = 26,
  L"VARIANT",                          // btVariant = 27,
  L"<complex>",                        // btComplex = 28,
  L"<bit>",                            // btBit = 29,
  L"BSTR",                             // btBSTR = 30,
  L"HRESULT",                          // btHresult = 31
  L"char16_t",                         // btChar16 = 32
  L"char32_t"                          // btChar32 = 33
  L"char8_t",                          // btChar8  = 34
};

// Tags returned by Dia
const wchar_t* const rgTags[] =
{
  L"(SymTagNull)",                     // SymTagNull
  L"Executable (Global)",              // SymTagExe
  L"Compiland",                        // SymTagCompiland
  L"CompilandDetails",                 // SymTagCompilandDetails
  L"CompilandEnv",                     // SymTagCompilandEnv
  L"Function",                         // SymTagFunction
  L"Block",                            // SymTagBlock
  L"Data",                             // SymTagData
  L"Annotation",                       // SymTagAnnotation
  L"Label",                            // SymTagLabel
  L"PublicSymbol",                     // SymTagPublicSymbol
  L"UserDefinedType",                  // SymTagUDT
  L"Enum",                             // SymTagEnum
  L"FunctionType",                     // SymTagFunctionType
  L"PointerType",                      // SymTagPointerType
  L"ArrayType",                        // SymTagArrayType
  L"BaseType",                         // SymTagBaseType
  L"Typedef",                          // SymTagTypedef
  L"BaseClass",                        // SymTagBaseClass
  L"Friend",                           // SymTagFriend
  L"FunctionArgType",                  // SymTagFunctionArgType
  L"FuncDebugStart",                   // SymTagFuncDebugStart
  L"FuncDebugEnd",                     // SymTagFuncDebugEnd
  L"UsingNamespace",                   // SymTagUsingNamespace
  L"VTableShape",                      // SymTagVTableShape
  L"VTable",                           // SymTagVTable
  L"Custom",                           // SymTagCustom
  L"Thunk",                            // SymTagThunk
  L"CustomType",                       // SymTagCustomType
  L"ManagedType",                      // SymTagManagedType
  L"Dimension",                        // SymTagDimension
  L"CallSite",                         // SymTagCallSite
  L"InlineSite",                       // SymTagInlineSite
  L"BaseInterface",                    // SymTagBaseInterface
  L"VectorType",                       // SymTagVectorType
  L"MatrixType",                       // SymTagMatrixType
  L"HLSLType",                         // SymTagHLSLType
  L"Caller",                           // SymTagCaller,
  L"Callee",                           // SymTagCallee,
  L"Export",                           // SymTagExport,
  L"HeapAllocationSite",               // SymTagHeapAllocationSite
  L"CoffGroup",                        // SymTagCoffGroup
  L"Inlinee",                          // SymTagInlinee
};


// Processors
const wchar_t* const rgFloatPackageStrings[] =
{
  L"hardware processor (80x87 for Intel processors)",    // CV_CFL_NDP
  L"emulator",                                           // CV_CFL_EMU
  L"altmath",                                            // CV_CFL_ALT
  L"???"
};

namespace StacktraceRestorer {

    class DiaString
    {
    public:
        DiaString() = default;
        DiaString(const DiaString&) = delete;
        DiaString(DiaString&&) = delete;

        DiaString& operator=(const DiaString&) = delete;
        DiaString& operator=(DiaString&&) = delete;

        ~DiaString()
        {
            ReleaseMemory();
        }

        BSTR* operator&()
        {
            ReleaseMemory();
            return &str_;
        }

        operator std::wstring() const
        {
            if (!str_)
                return L"";

            return str_;
        }

    private:
        void ReleaseMemory()
        {
            if (str_ != nullptr)
            {
                // Cannot call SysFreeString as dia generated invalid BSTR
                // for x64.
                LocalFree(str_ - 2);
                str_ = nullptr;
            }
        }

        BSTR str_ = nullptr;
    };


    std::vector<LineInfo> GetLinesInfo(CComPtr<IDiaEnumLineNumbers> pLines)
    {
        IDiaLineNumber* pLine;
        DWORD celt;
        DWORD dwRVA;
        DWORD dwSeg;
        DWORD dwOffset;
        DWORD dwLinenum;
        DWORD dwSrcId;
        DWORD dwLength;

        std::vector<LineInfo> linesInfo;
        while (SUCCEEDED(pLines->Next(1, &pLine, &celt)) && (celt == 1)) {
            if ((pLine->get_relativeVirtualAddress(&dwRVA) == S_OK) &&
                (pLine->get_addressSection(&dwSeg) == S_OK) &&
                (pLine->get_addressOffset(&dwOffset) == S_OK) &&
                (pLine->get_lineNumber(&dwLinenum) == S_OK) &&
                (pLine->get_sourceFileId(&dwSrcId) == S_OK) &&
                (pLine->get_length(&dwLength) == S_OK)) {
                //wprintf(L"\tline %u at [%08X][%04X:%08X], len = 0x%X", dwLinenum, dwRVA, dwSeg, dwOffset, dwLength);

                //_bstr_t sourceFile = "...";
                //CComPtr<IDiaSourceFile> pSource;
                //if (pLine->get_sourceFile(&pSource) == S_OK) {
                //    if (pSource->get_fileName(sourceFile.GetAddress()) == S_OK) {
                //        //sourceFile = sourceFileBSTR.GetBSTR();
                //    }
                //}
                //linesInfo.push_back({ dwRVA, dwLinenum, sourceFile.GetBSTR() });

                std::wstring sourceFile = L"...";
                CComPtr<IDiaSourceFile> pSource;
                if (pLine->get_sourceFile(&pSource) == S_OK) {
                    DiaString bstrSourceFile;
                    if (pSource->get_fileName(&bstrSourceFile) == S_OK) {
                        sourceFile = bstrSourceFile;
                    }
                }
                linesInfo.push_back({ dwRVA, dwLinenum, sourceFile });


                pLine->Release();
            }
        }

        return linesInfo;
    }


    CComPtr<IDiaEnumLineNumbers> GetLineNumbersByRVA(CComPtr<IDiaSymbol> pFunc, CComPtr<IDiaSession> pSession)
    {
        DWORD rva;
        ULONGLONG length;
        CComPtr<IDiaEnumLineNumbers> pEnum;

        if (pFunc->get_relativeVirtualAddress(&rva) == S_OK) {
            pFunc->get_length(&length);
            if (FAILED(pSession->findLinesByRVA(rva, static_cast<DWORD>(length), &pEnum)))
                return nullptr;
        }
        return pEnum;
    }

    std::unique_ptr<SymInfo> GetPublicSymbolInfo(CComPtr<IDiaSession> pSession, CComPtr<IDiaSymbol> pSymbol)
    {
        DWORD dwSymTag;
        DWORD dwRVA;
        DWORD dwSeg;
        DWORD dwOff;
        DWORD dwLength;

        DiaString symName;
        DiaString symUndecName;
        //_bstr_t symName;
        //_bstr_t symUndecName;

        if (pSymbol->get_symTag(&dwSymTag) != S_OK) {
            return nullptr;
        }

        // dwSymTag == SymTagPublicSymbol (most cases)
        if (dwSymTag != SymTagThunk) {
            if (pSymbol->get_name(&symName) == S_OK) {
                if (pSymbol->get_undecoratedName(&symUndecName) == S_OK) {
            //if (pSymbol->get_name(symName.GetAddress()) == S_OK) {
            //    if (pSymbol->get_undecoratedName(symUndecName.GetAddress()) == S_OK) {
                    //std::wstring symUndecoratedName = symUndecName.GetBSTR();
                    std::wstring symUndecoratedName = symUndecName;


                    if (pSymbol->get_relativeVirtualAddress(&dwRVA) != S_OK) {
                        dwRVA = 0xFFFFFFFF;
                    }
                    pSymbol->get_addressSection(&dwSeg);
                    pSymbol->get_addressOffset(&dwOff);

                    auto symInfo = std::make_unique<SymInfo>();
                    symInfo->RVA = dwRVA;
                    symInfo->name = symUndecoratedName;


                    CComPtr<IDiaEnumLineNumbers> pLines = GetLineNumbersByRVA(pSymbol, pSession);
                    if (pLines) {
                        symInfo->lines = GetLinesInfo(pLines);
                    }

                    return symInfo;
                }
            }
        }
    }

    //void PrintSourceFile(CComPtr<IDiaSourceFile> pSource)
    //{
    //    BSTR bstrSourceName;

    //    if (pSource->get_fileName(&bstrSourceName) == S_OK) {
    //        wprintf(L"\t%s", bstrSourceName);

    //        SysFreeString(bstrSourceName);
    //    }

    //    else {
    //        wprintf(L"ERROR - PrintSourceFile() get_fileName");
    //        return;
    //    }

    //    BYTE checksum[256];
    //    DWORD cbChecksum = sizeof(checksum);

    //    if (pSource->get_checksum(cbChecksum, &cbChecksum, checksum) == S_OK) {
    //        wprintf(L" (");

    //        DWORD checksumType;

    //        if (pSource->get_checksumType(&checksumType) == S_OK) {
    //            switch (checksumType) {
    //            case CHKSUM_TYPE_NONE:
    //                wprintf(L"None");
    //                break;

    //            case CHKSUM_TYPE_MD5:
    //                wprintf(L"MD5");
    //                break;

    //            case CHKSUM_TYPE_SHA1:
    //                wprintf(L"SHA1");
    //                break;

    //            default:
    //                wprintf(L"0x%X", checksumType);
    //                break;
    //            }

    //            if (cbChecksum != 0) {
    //                wprintf(L": ");
    //            }
    //        }

    //        for (DWORD ib = 0; ib < cbChecksum; ib++) {
    //            wprintf(L"%02X", checksum[ib]);
    //        }

    //        wprintf(L")");
    //    }
    //}


    //void PrintLines(CComPtr<IDiaEnumLineNumbers> pLines)
    //{
    //    IDiaLineNumber* pLine;
    //    DWORD celt;
    //    DWORD dwRVA;
    //    DWORD dwSeg;
    //    DWORD dwOffset;
    //    DWORD dwLinenum;
    //    DWORD dwSrcId;
    //    DWORD dwLength;

    //    DWORD dwSrcIdLast = (DWORD)(-1);

    //    while (SUCCEEDED(pLines->Next(1, &pLine, &celt)) && (celt == 1)) {
    //        if ((pLine->get_relativeVirtualAddress(&dwRVA) == S_OK) &&
    //            (pLine->get_addressSection(&dwSeg) == S_OK) &&
    //            (pLine->get_addressOffset(&dwOffset) == S_OK) &&
    //            (pLine->get_lineNumber(&dwLinenum) == S_OK) &&
    //            (pLine->get_sourceFileId(&dwSrcId) == S_OK) &&
    //            (pLine->get_length(&dwLength) == S_OK)) {
    //            wprintf(L"\tline %u at [%08X][%04X:%08X], len = 0x%X", dwLinenum, dwRVA, dwSeg, dwOffset, dwLength);

    //            if (dwSrcId != dwSrcIdLast) {
    //                CComPtr<IDiaSourceFile> pSource;
    //                if (pLine->get_sourceFile(&pSource) == S_OK) {
    //                    PrintSourceFile(pSource);

    //                    dwSrcIdLast = dwSrcId;
    //                }
    //            }

    //            pLine->Release();

    //            putwchar(L'\n');
    //        }
    //    }
    //}




    //void PrintPublicSymbolWithLines(CComPtr<IDiaSession> pSession, CComPtr<IDiaSymbol> pSymbol) {
    //
    //    DWORD dwSymTag;
    //    DWORD dwRVA;
    //    DWORD dwSeg;
    //    DWORD dwOff;
    //    DWORD dwLength;
    //    BSTR bstrName;
    //
    //    if (pSymbol->get_symTag(&dwSymTag) != S_OK) {
    //        return;
    //    }
    //
    //    if (pSymbol->get_relativeVirtualAddress(&dwRVA) != S_OK) {
    //        dwRVA = 0xFFFFFFFF;
    //    }
    //
    //    pSymbol->get_addressSection(&dwSeg);
    //    pSymbol->get_addressOffset(&dwOff);
    //
    //    wprintf(L"%s: [%08X][%04X:%08X] ", rgTags[dwSymTag], dwRVA, dwSeg, dwOff);
    //
    //
    //    if (dwSymTag == SymTagThunk) {
    //        if (pSymbol->get_name(&bstrName) == S_OK) {
    //            wprintf(L"%s\n", bstrName);
    //
    //            SysFreeString(bstrName);
    //        }
    //
    //        else {
    //            if (pSymbol->get_targetRelativeVirtualAddress(&dwRVA) != S_OK) {
    //                dwRVA = 0xFFFFFFFF;
    //            }
    //
    //            pSymbol->get_targetSection(&dwSeg);
    //            pSymbol->get_targetOffset(&dwOff);
    //
    //            wprintf(L"target -> [%08X][%04X:%08X]\n", dwRVA, dwSeg, dwOff);
    //        }
    //    }
    //
    //    else {
    //        // must be a function or a data symbol
    //
    //        BSTR bstrUndname;
    //
    //        if (pSymbol->get_name(&bstrName) == S_OK) {
    //            if (pSymbol->get_undecoratedName(&bstrUndname) == S_OK) {
    //
    //                std::wstring decoratedSymName = bstrName;
    //
    //                bool targetFound = false;
    //                if (decoratedSymName.find(std::wstring{ L"foo" }) != std::wstring::npos ||
    //                    decoratedSymName.find(std::wstring{ L"bar" }) != std::wstring::npos ||
    //                    decoratedSymName.find(std::wstring{ L"privateFunc" }) != std::wstring::npos ||
    //                    decoratedSymName.find(std::wstring{ L"baz" }) != std::wstring::npos) {
    //                    wprintf(L"\n- - - - - - - - - - - >>>>   ");
    //                    targetFound = true;
    //                }
    //
    //
    //                wprintf(L"%s(%s)\n", L"***", bstrUndname);
    //
    //                if (targetFound) {
    //                    wprintf(L"\n ------ lines: \n");
    //                    CComPtr<IDiaEnumLineNumbers> pLines = GetLineNumbersByRVA(pSymbol, pSession);
    //                    if (pLines) {
    //                        PrintLines(pLines);
    //                    }
    //                    wprintf(L"\n\n\n");
    //                }
    //
    //                SysFreeString(bstrUndname);
    //            }
    //
    //            else {
    //                wprintf(L"%s\n", bstrName);
    //            }
    //
    //            SysFreeString(bstrName);
    //        }
    //    }
    //}

    //void PrintPublicSymbolWithLines(CComPtr<IDiaSession> pSession, CComPtr<IDiaSymbol> pSymbol)
    //{
    //    DWORD dwSymTag;
    //    DWORD dwRVA;
    //    DWORD dwSeg;
    //    DWORD dwOff;
    //    DWORD dwLength;
    //    BSTR bstrName;
    //    BSTR bstrUndname;

    //    if (pSymbol->get_symTag(&dwSymTag) != S_OK) {
    //        return;
    //    }

    //    // dwSymTag == SymTagPublicSymbol
    //    if (dwSymTag != SymTagThunk) {

    //        if (pSymbol->get_name(&bstrName) == S_OK) {
    //            if (pSymbol->get_undecoratedName(&bstrUndname) == S_OK) {

    //                std::wstring undecoratedSymName = bstrUndname;

    //                bool targetFound = false;
    //                if (undecoratedSymName.find(std::wstring{ L"foo" }) != std::wstring::npos ||
    //                    undecoratedSymName.find(std::wstring{ L"bar" }) != std::wstring::npos ||
    //                    undecoratedSymName.find(std::wstring{ L"privateFunc" }) != std::wstring::npos ||
    //                    undecoratedSymName.find(std::wstring{ L"baz" }) != std::wstring::npos) {
    //                    targetFound = true;
    //                }

    //                if (!targetFound) {
    //                    SysFreeString(bstrName);
    //                    SysFreeString(bstrUndname);
    //                    return;
    //                }


    //                if (pSymbol->get_relativeVirtualAddress(&dwRVA) != S_OK) {
    //                    dwRVA = 0xFFFFFFFF;
    //                }
    //                pSymbol->get_addressSection(&dwSeg);
    //                pSymbol->get_addressOffset(&dwOff);

    //                wprintf(L"%s: [%08X][%04X:%08X] ", rgTags[dwSymTag], dwRVA, dwSeg, dwOff);
    //                //wprintf(L"%s(%s)\n", bstrName, bstrUndname);
    //                wprintf(L"%s(%s)\n", L"***", bstrUndname);

    //                wprintf(L"--- lines: \n");
    //                CComPtr<IDiaEnumLineNumbers> pLines = GetLineNumbersByRVA(pSymbol, pSession);
    //                if (pLines) {
    //                    PrintLines(pLines);
    //                }
    //                wprintf(L"\n\n\n");


    //                SysFreeString(bstrUndname);
    //            }
    //            else {
    //                wprintf(L"%s\n", bstrName);
    //            }

    //            SysFreeString(bstrName);
    //        }
    //    }
    //}
}