#include "CrashInfo.h"
#include <cassert>

void FillCrashInfoWithExceptionPointers(std::shared_ptr<CrashInfo> crashInfo, EXCEPTION_POINTERS* pExceptionPtrs)
{
    // De-referencing creates a copy
    crashInfo->exceptionPointers = *pExceptionPtrs;
    crashInfo->contextRecord = *(pExceptionPtrs->ContextRecord);
    //crashInfo.threadId = threadId;

    int indexOfExceptionRecord = 0;
    crashInfo->numberOfExceptionRecords = 0;
    EXCEPTION_RECORD* exceptionRecord = pExceptionPtrs->ExceptionRecord;

    while (exceptionRecord != 0)
    {
        if (indexOfExceptionRecord >= MaximumNumberOfNestedExceptions)
        {
            // Yikes, maximum number of nested exceptions reached
            break;
        }

        // De-referencing creates a copy
        crashInfo->exceptionRecords[indexOfExceptionRecord] = *exceptionRecord;

        ++indexOfExceptionRecord;
        ++crashInfo->numberOfExceptionRecords;
        exceptionRecord = exceptionRecord->ExceptionRecord;
    }
}

std::vector<uint8_t> SerializeCrashInfo(std::shared_ptr<CrashInfo> crashInfo) {
    auto ptr = reinterpret_cast<uint8_t*>(crashInfo.get());
    return std::vector<uint8_t>(ptr, ptr + sizeof(*crashInfo.get()));
}

std::shared_ptr<CrashInfo> DeserializeCrashInfo(std::vector<uint8_t>& bytes) {
    auto crashInfo = std::make_shared<CrashInfo>();
    assert(sizeof(*crashInfo.get()) == bytes.size());

    auto ptr = reinterpret_cast<uint8_t*>(crashInfo.get());
    std::copy(bytes.begin(), bytes.end(), ptr);

    return crashInfo;
}

void FixExceptionPointersInCrashInfo(std::shared_ptr<CrashInfo> crashInfo)
{
    crashInfo->exceptionPointers.ContextRecord = &(crashInfo->contextRecord);

    for (int indexOfExceptionRecord = 0; indexOfExceptionRecord < crashInfo->numberOfExceptionRecords; ++indexOfExceptionRecord)
    {
        if (indexOfExceptionRecord == 0)
            crashInfo->exceptionPointers.ExceptionRecord = &(crashInfo->exceptionRecords[indexOfExceptionRecord]);
        else
            crashInfo->exceptionRecords[indexOfExceptionRecord - 1].ExceptionRecord = &(crashInfo->exceptionRecords[indexOfExceptionRecord]);
    }
}