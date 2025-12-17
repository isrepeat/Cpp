#pragma once
#include "StdRedirection.h"
#include "Logger.h"
#include "Macros.h"
#include <fcntl.h>
#include <io.h>

namespace HELPERS_NS {
    bool StdRedirection::ReAllocConsole() {
        BOOL bResult;
        if (!FreeConsole()) {
            LogLastError;
            return false;
        }
        if (!AllocConsole()) {
            LogLastError;
            return false;
        }

        FILE* fDummy;
        if (freopen_s(&fDummy, "CONOUT$", "w", stdout) != 0) {
            return false;
        }
        return true;
    }


    StdRedirection::StdRedirection() {
        LOG_FUNCTION_ENTER("StdRedirection()");
    }

    StdRedirection::~StdRedirection() {
        LOG_FUNCTION_SCOPE("StdRedirection()");
        EndRedirect();
        // If durring redirection std HANDLEs was saved with ::GetStdHandle(...) it not restored
    }

    void StdRedirection::BeginRedirect(std::function<void(std::vector<char>)> readCallback) {
        LOG_FUNCTION_ENTER("BeginRedirect(readCallback)");
        std::unique_lock lk{ mx };

        if (redirectInProcess.exchange(true)) {
            LOG_WARNING_D("Redirect already started, ignore");
            return;
        }

        fdPrevStdOut = _dup(_fileno(stdout)); // save original stdout descriptor

        CreatePipe(&hReadPipe, &hWritePipe, nullptr, PIPE_BUFFER_SIZE);
        SetStdHandle(STD_OUTPUT_HANDLE, hWritePipe);

        int fdWritePipe = _open_osfhandle((intptr_t)hWritePipe, O_WRONLY); // associate a file descriptor to the file handle
        if (_dup2(fdWritePipe, _fileno(stdout)) == -1) {
            Dbreak;
            throw std::exception("_dup2(...) Failed redirect stdout to hWritePipe");
        }
        setvbuf(stdout, NULL, _IONBF, 0); // set "no buffer" for stdout (no need fflush manually)

        this->readCallback = readCallback;
        listeningRoutine = std::async(std::launch::async, [this] {
            while (redirectInProcess) {
                try {
                    //outputBuffer.clear(); // ensure that buffer cleared to avoid inserting new data at end
                    // If callback is empty all data will append at the end of outputBuffer
                    ReadFileAsync<char>(hReadPipe, redirectInProcess, outputBuffer, OUTPUT_BUFFER_SIZE);
                    std::unique_lock lk{ mx };
                    if (this->readCallback) {
                        this->readCallback(std::move(outputBuffer));
                    }
                }
                catch (const std::exception& ex) {
                    LOG_DEBUG_D("Catch ReadFileAsync exception = {}", ex.what());
                    LogLastError;
                    return;
                }
            }
            });
    }

    void StdRedirection::SetCallback(std::function<void(std::vector<char>)> readCallback) {
        LOG_FUNCTION_ENTER("SetCallback(readCallback)");
        std::unique_lock lk{ mx };
        this->readCallback = readCallback;
    }

    void StdRedirection::EndRedirect() {
        LOG_FUNCTION_ENTER("EndRedirect()");
        {
            std::unique_lock lk{ mx };

            if (!redirectInProcess.exchange(false)) {
                LOG_WARNING_D("Redirect already finished, ignore");
                return;
            }

            if (_dup2(fdPrevStdOut, _fileno(stdout)) == -1) { // restore stdout to original (saved) descriptor
                Dbreak;
                throw std::exception("_dup2(...) Failed restore stdout");
            }
            _close(fdPrevStdOut);

            if (!CloseHandleSafe(hWritePipe)) {
                LOG_ERROR_D("Failed close hWritePipe");
                LogLastError;
            }
            if (!CloseHandleSafe(hReadPipe)) {
                LOG_ERROR_D("Failed close hReadPipe");
                LogLastError;
            }
        }

        if (listeningRoutine.valid())
            listeningRoutine.get();

        LOG_DEBUG("Redirection finished");
    }
}