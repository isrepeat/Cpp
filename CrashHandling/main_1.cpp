//#include <windows.h>
//#include <dbghelp.h>
//#include <stdio.h>
//#include <fstream>
//#include <string>
//#pragma comment(lib,"dbghelp.lib")
//
//int main(int argc, char* argv[]) {
//    if (argc != 3) {
//        printf("usage %s \"Module/Path\" \"Offset as hex 0x1010\"\n", argv[0]);
//        return FALSE;
//    }
//
//
//    
//    DWORD64 offset = _strtoui64(argv[2], NULL, 16);
//    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
//    HANDLE hProcess = GetCurrentProcess();
//    if (!SymInitialize(hProcess, NULL, FALSE)) {
//        printf("SymInitialize returned error : %u\n", GetLastError());
//        return FALSE;
//    }
//    DWORD64 ModBase = SymLoadModuleEx(hProcess, NULL, argv[1], NULL, 0, 0, NULL, 0);
//    if ((ModBase == 0) && (GetLastError() != ERROR_SUCCESS)) {
//        printf("SymLoadModuleEx returned error : %u\n", GetLastError());
//        return FALSE;
//    }
//    DWORD64 SymAddr = ModBase + offset;
//    printf("Module %s Loaded At 0x%I64x SymAddr = 0x%I64x\n", argv[1], ModBase, SymAddr);
//    IMAGEHLP_LINE64 line = { 0 };
//    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
//    DWORD dwDisplacement = 0;
//    if (!SymGetLineFromAddr64(hProcess, SymAddr, &dwDisplacement, &line)) {
//        printf("SymGetLineFromAddr64 returned error : %u\n", GetLastError());
//        return FALSE;
//    }
//    printf("Source= %s # %u @ %I64x\n", line.FileName, line.LineNumber, line.Address);
//    std::ifstream fs;
//    fs.open(line.FileName);
//    if (fs.is_open()) {
//        std::string buff;
//        unsigned  int lineno = 0;
//        while (lineno != line.LineNumber && getline(fs, buff)) {
//            ++lineno;
//        }
//        printf("%s\n", buff.c_str());
//        fs.close();
//    }
//    else {
//        printf("cannot open %s\n", line.FileName);
//    }
//}