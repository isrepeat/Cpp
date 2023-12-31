//#define  _CRT_SECURE_NO_WARNINGS
#include <Helpers/StdRedirection.h>
#include <Helpers/Helpers.h>
#include <Helpers/Logger.h>
#include <Helpers/Time.h>

#include <iostream>
#include <fstream>
#include <sstream> 
#include <future>
#include <string>

#include <windows.h>
#include <fcntl.h>
#include <io.h>

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


void TestStdRedirectionClass() {

    printf("[printf] Message A \n");
    printf("[cout] Message B \n");

    HANDLE stdHandle_orig = ::GetStdHandle(STD_OUTPUT_HANDLE);

    std::vector<char> redirectedBuffer;

    H::StdRedirection stdRedirection;
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