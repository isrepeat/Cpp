#define  _CRT_SECURE_NO_WARNINGS
#include <Helpers/Helpers.h>
#include <Helpers/Logger.h>
#include <Helpers/Time.h>

#include <iostream>
#include <fstream>
#include <sstream> 
#include <future>
#include <string>


#include <windows.h>
#include <io.h>
#include <fcntl.h>

// Related Links:
// https://truong.io/posts/capturing_stdout_for_c++_unit_testing.html
// https://stackoverflow.com/questions/39110882/redirecting-cout-to-the-new-buffer-that-created-with-winapi
// https://stackoverflow.com/questions/311955/redirecting-cout-to-a-console-in-windows
// https://stackoverflow.com/questions/34245235/qt-how-to-redirect-qprocess-stdout-to-textedit
// https://stackoverflow.com/questions/54094127/redirecting-stdout-in-win32-does-not-redirect-stdout
// https://stackoverflow.com/questions/14604366/console-contents
// https://stackoverflow.com/questions/955962/how-to-buffer-stdout-in-memory-and-write-it-from-a-dedicated-thread
// https://stackoverflow.com/questions/5419356/redirect-stdout-stderr-to-a-string
// https://stackoverflow.com/questions/4810516/c-redirecting-stdout/4814197#4814197
// https://stackoverflow.com/questions/49818540/how-to-redirect-stdout-stderr-for-current-process
// https://stackoverflow.com/questions/573724/how-can-i-redirect-stdout-to-some-visible-display-in-a-windows-application


#define MAX_LEN 40
void TestRedirectStdoutToPipe() {
    int res = 0;

    enum PipeStream {
        READ,
        WRITE
    };
    int out_pipe[2]; // 0 - READ, 1 - WRITE stream descriptors
    char buffer[MAX_LEN + 1] = { 0 };
    
    ////Save position of current standard output
    //fpos_t pos;
    //fgetpos(stdout, &pos);

    setvbuf(stdout, NULL, _IONBF, 0); // Stdout will no have buffer
    
    int saved_stdout = _dup(_fileno(stdout));  /* save stdout for display later */
    //H::Timer::Once(2000ms, [&out_pipe, &buffer] {
    //    //fflush(stdout);
    //    int res = _read(out_pipe[READ], buffer, MAX_LEN); /* read from pipe into buffer */
    //    if (res == -1) {
    //        Dbreak;
    //    }
    //    });

    printf("[stdout] Message A \n");
    
    if (_pipe(out_pipe, 65536, O_BINARY) == -1) // make pipe's descriptors for IN / OUT streams
        Dbreak;

    res =_dup2(out_pipe[WRITE], _fileno(stdout));   /* redirect stdout to the pipe */
    if (res == -1) {
        Dbreak;
    }
    _close(out_pipe[WRITE]);

    printf("[pipe] Redirected A \n");

    res = _read(out_pipe[READ], buffer, MAX_LEN); /* read from pipe into buffer */
    if (res == -1) {
        Dbreak;
    }

    printf("[pipe] Redirected B \n");

    res = _read(out_pipe[READ], buffer, MAX_LEN); /* read from pipe into buffer */
    if (res == -1) {
        Dbreak;
    }

    res = _dup2(saved_stdout, _fileno(stdout));  /* reconnect stdout for testing */
    if (res == -1) {
        Dbreak;
    }
    _close(saved_stdout);
    //clearerr(stdout);
    //fsetpos(stdout, &pos);
    //setvbuf(stdout, NULL, _IONBF, 0);

    fprintf(stdout, "[stdout] Message B \n");
    printf("[stdout] Message C \n");

    printf("\n\npipe buffer:\n%s", buffer);
    return;
}


// Use this class before first use ::GetStdHandle(...) to avoid side effects, for example as singleton. 
// All std HANDLEs that was saved with ::GetStdHandle(...) before redirection will not be redirected.
// All std HANDLEs that was saved with ::GetStdHandle(...) during redirection will not be restored.
class StdRedirection {
public:
    StdRedirection();
    ~StdRedirection();

    void BeginRedirect(std::function<void(std::vector<char>)> readCallback);
    void EndRedirect();

private:
    enum PipeStream {
        READ,
        WRITE
    };

    const int PIPE_BUFFER_SIZE = 65536;
    const int OUTPUT_BUFFER_SIZE = MAX_PATH;

    std::mutex mx;

    int ioPipes[2];
    int oldStdOut;
    std::vector<char> outputBuffer;
    
    std::atomic<bool> redirectInProcess;
    std::future<void> listeningRoutine;
};


StdRedirection::StdRedirection()
    : ioPipes{ -1, -1 } // initialize with default values to suppress compiler warning
    , oldStdOut{ -1 }
    , redirectInProcess{ false }
{
    LOG_FUNCTION_ENTER("StdRedirection()");
    setvbuf(stdout, NULL, _IONBF, 0); // set "no buffer" for stdout (no need fflush manually)
}

StdRedirection::~StdRedirection() {
    LOG_FUNCTION_SCOPE("StdRedirection()");
    EndRedirect();
    // If durring redirection std HANDLEs was saved with ::GetStdHandle(...) it not restored
}

void StdRedirection::BeginRedirect(std::function<void(std::vector<char>)> readCallback) {
    LOG_FUNCTION_ENTER("BeginRedirect()");
    LOG_ASSERT(readCallback, "readCallback is empty");
    std::unique_lock lk{ mx };

    if (redirectInProcess.exchange(true)) {
        LOG_WARNING_D("Redirect already started, ignore");
        return;
    }

    oldStdOut = _dup(_fileno(stdout)); // save original stdout descriptor

    if (_pipe(ioPipes, PIPE_BUFFER_SIZE, O_BINARY) == -1) { // make pipe's descriptors for READ / WRITE streams
        Dbreak;
        throw std::exception("_pipe(...) Failed to init ioPipes");
    }
    if (_dup2(ioPipes[WRITE], _fileno(stdout)) == -1) { // redirect stdout to the pipe
        Dbreak;
        throw std::exception("_dup2(...) Failed redirect stdout to the pipe");
    }
    _close(ioPipes[WRITE]); // we no need work with WRITE stream so close it now


    listeningRoutine = std::async(std::launch::async, [this, readCallback] {
        while (redirectInProcess) {
            outputBuffer.resize(OUTPUT_BUFFER_SIZE);

            // NOTE: _read(...) block current thread until any data arrives in the ioPipes[READ]
            int readBytes = _read(ioPipes[READ], outputBuffer.data(), outputBuffer.size());
            if (readBytes == -1) {
                Dbreak;
                throw std::exception("_read(...) Failed when read from ipPipe[READ]");
            }
            else if (readBytes == 0) { // read end of file
                return;
            }

            outputBuffer.resize(readBytes); // truncate
            if (readCallback) {
                readCallback(std::move(outputBuffer));
            }
        }
        });
}

// May throw exception
void StdRedirection::EndRedirect() {
    LOG_FUNCTION_ENTER("EndRedirect()");
    std::unique_lock lk{ mx };

    if (!redirectInProcess.exchange(false)) {
        LOG_WARNING_D("Redirect already finished, ignore");
        return;
    }

    if (_dup2(oldStdOut, _fileno(stdout)) == -1) { // restore stdout to original (saved) descriptor
        Dbreak;
        throw std::exception("_dup2(...) Failed restore stdout");
    }
    // after stdout restored the _read(...) return 0  in  listeningRoutine
    _close(oldStdOut);

    if (listeningRoutine.valid())
        listeningRoutine.get();

    LOG_DEBUG("Redirection finished");
}


HANDLE myStdOut = nullptr;
HANDLE origStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);

void TestStdRedirectionClass() {

    printf("[printf] Message A \n");
    printf("[cout] Message B \n");

    HANDLE stdHandle_orig = ::GetStdHandle(STD_OUTPUT_HANDLE);

    std::vector<char> redirectedBuffer;

    StdRedirection stdRedirection;
    stdRedirection.BeginRedirect([&redirectedBuffer](std::vector<char> outputBuffer) {
        //std::string msg{ outputBuffer.begin(), outputBuffer.end() };
        redirectedBuffer.insert(redirectedBuffer.end(), outputBuffer.begin(), outputBuffer.end());
        //redirectedMessages.push_back(msg);
        });

    lg::DefaultLoggers::Init("D:\\main_53_redirect_IO.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);

    HANDLE stdHandle_curr = ::GetStdHandle(STD_OUTPUT_HANDLE);

    std::string text = "[WriteFile] Redirected A \n";
    DWORD dwBytesWritten = 0;
    bool res = false;
    // TODO: Investigate how duplicate write to STD_OUTPUT_HANDLE
    res = ::WriteFile(stdHandle_orig, text.data(), text.size(), &dwBytesWritten, nullptr); // Fail
    res = ::WriteFile(stdHandle_curr, text.data(), text.size(), &dwBytesWritten, nullptr); // Ok
    LOG_DEBUG_D("[Log] Redirected B"); // Ok

    printf("[printf] Redirected C \n");
    std::cout << "[cout] Redirected D \n";

    stdRedirection.EndRedirect();
    redirectedBuffer.push_back('\0');

    printf("[cout] Message C \n");
    printf("[printf] Message D \n");


    printf("\n\nRedirected messages:");
    //for (auto& msg : redirectedMessages) {
        printf("\n%s", redirectedBuffer.data());
    //}
    return;
}




int main() {
    //TestRedirectStdoutToPipe();
    TestStdRedirectionClass();

    //std::stringstream bufferCout;
    //std::stringstream bufferCerr;

    //std::streambuf* prevBufferCout = std::cout.rdbuf(bufferCout.rdbuf());
    //std::streambuf* prevBufferCerr = std::cerr.rdbuf(bufferCerr.rdbuf());
    

    //LOG_DEBUG_D("Logger redirected message");
    //std::cout << "[cout] Redirected message";
    //std::cerr << "[cerr] Redirected error" << std::endl;
    //printf("[printf] Hello world \n");

    //// Restore original buffers
    //std::cout.rdbuf(prevBufferCout);
    //std::cerr.rdbuf(prevBufferCerr);

    //LOG_DEBUG_D("Logger msg to console");
    //std::cout << "Print message to console" << std::endl;
    //std::cerr << "Print error to console" << std::endl;

    //system("pause");
    Sleep(20000);
	return 0;
}