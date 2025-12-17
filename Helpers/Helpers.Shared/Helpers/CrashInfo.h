#pragma once
#include "common.h"
#include <Windows.h>
#include <vector>
#include <memory>

// The maximum number of nested exception that we can handle. The value we
// use for this constant is an arbitrarily chosen number that is, hopefully,
// sufficiently high to support all realistic and surrealistic scenarios.
//
// sizeof(CrashInfo) for a maximum of 1000 = ca. 80 KB
const int MaximumNumberOfNestedExceptions = 1000;

#pragma pack(push, 1)
// Structure with information about the crash that we can pass to the
// watchdog process
struct CrashInfo {
    EXCEPTION_POINTERS exceptionPointers;
    int numberOfExceptionRecords;
    // Contiguous area of memory that can easily be processed by memcpy
    EXCEPTION_RECORD exceptionRecords[MaximumNumberOfNestedExceptions];
    CONTEXT contextRecord;
    //HANDLE hProcess;
    //int processId;
    int threadId;
};
#pragma pack(pop)


// The EXCEPTION_POINTERS parameter is the original exception pointer
// that we are going to deep-copy.
// The CrashInfo parameter receives the copy.
void FillCrashInfoWithExceptionPointers(std::shared_ptr<CrashInfo> crashInfo, EXCEPTION_POINTERS* exceptionPointers);

std::vector<uint8_t> SerializeCrashInfo(std::shared_ptr<CrashInfo> crashInfo);
std::shared_ptr<CrashInfo> DeserializeCrashInfo(std::vector<uint8_t>& crashInfo);

// The CrashInfo parameter is both in/out (must be run from mini dump writer process)
void FixExceptionPointersInCrashInfo(std::shared_ptr<CrashInfo> crashInfo);
