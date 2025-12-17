#pragma once
#include "common.h"
#include "File.h"
#include <functional>
#include <future>
#include <mutex>

namespace HELPERS_NS {
    // Use this class before first use ::GetStdHandle(...) to avoid side effects, for example as singleton. 
    // All std HANDLEs that was saved with ::GetStdHandle(...) before redirection will not be redirected.
    // All std HANDLEs that was saved with ::GetStdHandle(...) during redirection will not be restored.
    class StdRedirection {
    public:
        StdRedirection();
        ~StdRedirection();

        static bool ReAllocConsole();

        void BeginRedirect(std::function<void(std::vector<char>)> readCallback);
        void SetCallback(std::function<void(std::vector<char>)> readCallback);
        void EndRedirect();

    private:
        const int PIPE_BUFFER_SIZE = 65536;
        const int OUTPUT_BUFFER_SIZE = MAX_PATH;

        std::mutex mx;
        int fdPrevStdOut = -1;
        HANDLE hReadPipe = nullptr;
        HANDLE hWritePipe = nullptr;

        std::vector<char> outputBuffer;
        std::function<void(std::vector<char>)> readCallback;

        std::atomic<bool> redirectInProcess = false;
        std::future<void> listeningRoutine;
    };
}