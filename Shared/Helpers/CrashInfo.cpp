#include "CrashInfo.h"
#include <cassert>

void FillCrashInfoWithExceptionPointers(CrashInfo& crashInfo, EXCEPTION_POINTERS* exceptionPointers)
{
    // De-referencing creates a copy
    crashInfo.exceptionPointers = *exceptionPointers;
    crashInfo.contextRecord = *(exceptionPointers->ContextRecord);
    //crashInfo.threadId = threadId;

    int indexOfExceptionRecord = 0;
    crashInfo.numberOfExceptionRecords = 0;
    EXCEPTION_RECORD* exceptionRecord = exceptionPointers->ExceptionRecord;

    while (exceptionRecord != 0)
    {
        if (indexOfExceptionRecord >= MaximumNumberOfNestedExceptions)
        {
            // Yikes, maximum number of nested exceptions reached
            break;
        }

        // De-referencing creates a copy
        crashInfo.exceptionRecords[indexOfExceptionRecord] = *exceptionRecord;

        ++indexOfExceptionRecord;
        ++crashInfo.numberOfExceptionRecords;
        exceptionRecord = exceptionRecord->ExceptionRecord;
    }
}

std::vector<uint8_t> SerializeCrashInfo(CrashInfo& crashInfo) {
    auto ptr = reinterpret_cast<uint8_t*>(&crashInfo);
    return std::vector<uint8_t>(ptr, ptr + sizeof(crashInfo));
}

CrashInfo DeserializeCrashInfo(std::vector<uint8_t>& bytes) {
    CrashInfo crashInfo;
    assert(sizeof(crashInfo) == bytes.size());


    auto ptr = reinterpret_cast<uint8_t*>(&crashInfo);
    std::copy(bytes.begin(), bytes.end(), ptr);

    return crashInfo;
    //auto ptr = reinterpret_cast<uint8_t*>(&bytes[0]);
    //std::memcpy(&crashInfo, ptr, bytes.size());
    //std::copy(
}

void FixExceptionPointersInCrashInfo(CrashInfo& crashInfo)
{
    crashInfo.exceptionPointers.ContextRecord = &(crashInfo.contextRecord);

    for (int indexOfExceptionRecord = 0; indexOfExceptionRecord < crashInfo.numberOfExceptionRecords; ++indexOfExceptionRecord)
    {
        if (indexOfExceptionRecord == 0)
            crashInfo.exceptionPointers.ExceptionRecord = &(crashInfo.exceptionRecords[indexOfExceptionRecord]);
        else
            crashInfo.exceptionRecords[indexOfExceptionRecord - 1].ExceptionRecord = &(crashInfo.exceptionRecords[indexOfExceptionRecord]);
    }
}