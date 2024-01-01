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
// https://stackoverflow.com/questions/71612488/capture-printf-stdout-output-in-a-win32-application-or-reassign-stdout-handle
// https://stackoverflow.com/questions/12255543/using-stdout-in-a-win32-gui-application-crashes-if-i-dont-have-a-redirect-to-f
// https://stackoverflow.com/questions/54094127/redirecting-stdout-in-win32-does-not-redirect-stdout
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
// https://truong.io/posts/capturing_stdout_for_c++_unit_testing.html


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
    HANDLE stdHandle_def = ::GetStdHandle(STD_OUTPUT_HANDLE);
    bool consoleReallocated = H::StdRedirection::ReAllocConsole();
    //BOOL bResult = FreeConsole();
    //bResult = AllocConsole();
    //FILE* fDummy;
    //freopen_s(&fDummy, "CONOUT$", "w", stdout);

    HANDLE stdHandle_new = ::GetStdHandle(STD_OUTPUT_HANDLE);

    printf("[printf] Message A \n");
    printf("[cout] Message B \n");

    // Investigate how redirect to buffer with CreateFile
    bool res = false;
    char pipeBuffer[1024]{ 0 };
    DWORD dwBytesWritten = 0;


    H::StdRedirection stdRedirection;
    std::vector<char> redirectedBuffer;
    stdRedirection.BeginRedirect([&redirectedBuffer](std::vector<char> outputBuffer) {
        redirectedBuffer.insert(redirectedBuffer.end(), outputBuffer.begin(), outputBuffer.end());
        });

    lg::DefaultLoggers::Init("D:\\main_53_redirect_IO.log", lg::InitFlags::DefaultFlags | lg::InitFlags::EnableLogToStdout);

    HANDLE stdHandle_cur = ::GetStdHandle(STD_OUTPUT_HANDLE);
    printf("[printf] Redirected \n"); // Ok

    // TODO: Investigate how duplicate write to STD_OUTPUT_HANDLE
    std::string textA = "[WriteFile] Redirected A \n";
    std::string textB = "[WriteFile] Redirected B \n";
    std::string textC = "[WriteFile] Redirected C \n";
    bool resA = ::WriteFile(stdHandle_def, textA.data(), textA.size(), &dwBytesWritten, nullptr); // Fail
    bool resC = ::WriteFile(stdHandle_new, textB.data(), textB.size(), &dwBytesWritten, nullptr); // Fail
    bool resD = ::WriteFile(stdHandle_cur, textC.data(), textC.size(), &dwBytesWritten, nullptr); // Ok
    
    LOG_DEBUG_D("[Log] Redirected"); // Ok
    std::cout << "[cout] Redirected \n"; // Ok

    stdRedirection.EndRedirect();
    redirectedBuffer.push_back('\0');

    printf("[cout] Message C \n");
    printf("[printf] Message D \n");
    
    printf("\n\nRedirected messages:");
    printf("\n%s", redirectedBuffer.data());
    return;
}




int main() {
    //TestRedirectStdoutToPipe();
    TestStdRedirectionClass();

    //system("pause");
    Sleep(20000);
	return 0;
}