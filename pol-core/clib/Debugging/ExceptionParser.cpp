#include "ExceptionParser.h"
#include "LogSink.h"
#include "../threadhelp.h"
#include "../logfacility.h"

#include "../../plib/systemstate.h"
#include "../../plib/polver.h"

#include <cstring>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>

#ifndef _WIN32
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <unistd.h>
#include <sys/syscall.h>
#define SOCKET int
#else
#include<winsock2.h>
#include<Ws2tcpip.h>
#define snprintf _snprintf_s
#define ssize_t SSIZE_T
#pragma warning(disable: 4127) // conditional expression is constant
#pragma warning(disable: 4996) // unsafe strerror
#endif

#define MAX_STACK_TRACE_DEPTH          200
#define MAX_STACK_TRACE_STEP_LENGTH    256

namespace Pol{ namespace Clib{
using namespace std;

///////////////////////////////////////////////////////////////////////////////

void getSignalDescription(int signal, string &signalName, string &signalDescription)
{
    switch(signal)
    {
        case 1:
            signalName = "SIGHUP";
            signalDescription = "hangup detected on controlling terminal or death of controlling process";
            break;
        case 2:
            signalName = "SIGINT";
            signalDescription = "interrupt from keyboard";
            break;
        case 3:
            signalName = "SIGQUIT";
            signalDescription = "quit from keyboard";
            break;
        case 4:
            signalName = "SIGILL";
            signalDescription = "illegal Instruction";
            break;
        case 6:
            signalName = "SIGABRT";
            signalDescription = "abort signal from abort()";
            break;
        case 8:
            signalName = "SIGFPE";
            signalDescription = "floating point exception";
            break;
        case 9:
            signalName = "SIGKILL";
            signalDescription = "kill signal";
            break;
        case 10:
            signalName = "SIGBUS";
            signalDescription = "bus error";
            break;
        case 11:
            signalName = "SIGSEGV";
            signalDescription = "invalid memory reference";
            break;
        case 12:
            signalName = "SIGSYS";
            signalDescription = "bad argument to system call";
            break;
        case 13:
            signalName = "SIGPIPE";
            signalDescription = "broken pipe: write to pipe with no readers";
            break;
        case 14:
            signalName = "SIGALRM";
            signalDescription = "timer signal from alarm()";
            break;
        case 15:
            signalName = "SIGTERM";
            signalDescription = "termination signal";
            break;
        case 18:
            signalName = "SIGCONT";
            signalDescription = "continue signal from tty";
            break;
        case 19:
            signalName = "SIGSTOP";
            signalDescription = "stop signal from tty";
            break;
        case 20:
            signalName = "SIGTSTP";
            signalDescription = "stop signal from user (keyboard)";
            break;
        case 16:
        case 30:
            signalName = "SIGUSR1";
            signalDescription = "user-defined signal 1";
            break;
        case 17:
        case 31:
            signalName = "SIGUSR2";
            signalDescription = "user-defined signal 2";
            break;
        default:
            signalName = "unsupported signal";
            signalDescription = "unsupported signal occurred";
            break;
    }
}

void logExceptionSignal(int pSignal)
{
    string signalName;
    string signalDescription;

    getSignalDescription(pSignal, signalName, signalDescription);
    printf("Signal \"%s\"(%d: %s) detected.\n", signalName.c_str(), pSignal, signalDescription.c_str());
}

string getCompilerVersion()
{
    #ifndef _WIN32
        char result[256];
        sprintf(result, "gcc %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    #else
        string result;
        switch(_MSC_VER)
        {
        case 1800:
            result = "MSVC++ 12.0 (Visual Studio 2013)";
            break;
        case 1700:
            result = "MSVC++ 11.0 (Visual Studio 2012)";
            break;
        case 1600:
            result = "MSVC++ 10.0 (Visual Studio 2010)";
            break;
        case 1500:
            result = "MSVC++ 9.0 (Visual Studio 2008)";
            break;
        case 1400:
            result = "MSVC++ 8.0 (Visual Studio 2005)";
            break;
        case 1310:
            result = "MSVC++ 7.1 (Visual Studio 2003)";
            break;
        case 1300:
            result = "MSVC++ 7.0";
            break;
        case 1200:
            result = "MSVC++ 6.0";
            break;
        case 1100:
            result = "MSVC++ 5.0";
            break;
        default:
            if (_MSC_VER > 1800)
                result = "MSVC++ newer than version 12.0";
            else if (_MSC_VER < 1100)
                result = "MSVC++ older than version 5.0";
            else
                result = "MSVC++ (some unsupported version)";
            break;
        }
    #endif

    return result;
}

void doHttpPOST(string host, string url, string content)
{
    #define MAXLINE 4096
    char request[MAXLINE + 1];
    SOCKET socketFD;
    char targetIP[INET6_ADDRSTRLEN];

    /**
     * prepare the request
     */
    snprintf(request, MAXLINE,
             "POST %s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "Content-Type: application/x-www-form-urlencoded\r\n"
             "User-Agent: POL in-app abort reporting system, %s\r\n"
             "Content-length: %d\r\n\r\n"
             "%s", url.c_str(), host.c_str(), Pol::polverstr, (int)content.size(), content.c_str());

    /**
     * DNS lookup if needed
     */
    struct addrinfo *serverAddr;
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // IPv4 or IPv6
    hints.ai_flags = AI_ADDRCONFIG;
    hints.ai_socktype = SOCK_STREAM;
    int res = getaddrinfo(host.c_str(), "http", &hints, &serverAddr);
    if(res == 0)
    {
        switch(serverAddr->ai_addr->sa_family) {
            case AF_INET:
                if(inet_ntop(AF_INET, &((struct sockaddr_in *)serverAddr->ai_addr)->sin_addr, targetIP, INET_ADDRSTRLEN) == NULL)
                    exit(1);
                break;

            case AF_INET6:
                if(inet_ntop(AF_INET6, &((struct sockaddr_in *)serverAddr->ai_addr)->sin_addr, targetIP, INET6_ADDRSTRLEN) == NULL)
                    exit(1);
                break;

            default:
                fprintf(stderr, "Unknown address family found for %s\n", host.c_str());
                exit(1);
        }

        // create the socket
        socketFD = socket(serverAddr->ai_family, serverAddr->ai_socktype, serverAddr->ai_protocol);

        /**
         * connect to the bug tracking server
         */
        if((res = connect(socketFD, serverAddr->ai_addr, (int)serverAddr->ai_addrlen)) != 0)
        {
            fprintf(stderr, "connect() failed for server \"%s\"(IP: %s) due \"%s\"(%d)\n", host.c_str(), targetIP, strerror(errno), errno);
            exit(1);
        }

        /**
         * send the request
         */
        #ifndef _WIN32
            send(socketFD, request, strlen(request), MSG_NOSIGNAL);
        #else
            send(socketFD, request, (int)strlen(request), 0);
        #endif
        printf("Abort report was sent to %s%s (IP: %s)\n", host.c_str(), url.c_str(), targetIP);

        /**
         * wait for some answers and print them on the screen
         */
        ssize_t readBytes;
        char answer[MAXLINE + 1];

        #ifndef _WIN32
            while ((readBytes = recv(socketFD, answer, MAXLINE, MSG_NOSIGNAL)) > 0) {
        #else
            while ((readBytes = recv(socketFD, answer, MAXLINE, 0)) > 0) {
        #endif
            answer[readBytes] = '\0';
            printf("Answer from bug tracking server: %s\n", answer);
            // skip the received answer and proceed
        }

        // close the socket to the bug tracking server
        #ifndef _WIN32
            close(socketFD);
        #else
            closesocket(socketFD);
        #endif
    }else{
        fprintf(stderr, "getaddrinfo() failed for \"%s\" due to \"%s\"(code: %d)\n", host.c_str(), gai_strerror(res), res);
    }
}

void ExceptionParser::reportProgramAbort(string stackTrace, string reason)
{
    /**
     * set some default values if the abort occurs too early and pol.cfg wasn't parsed yet
     */
    string host = "polserver.com";
    string url = "/pol/report_program_abort.php";
    if((Plib::systemstate.config.report_server.c_str() != NULL) && (Plib::systemstate.config.report_server != ""))
    {
        host = Plib::systemstate.config.report_server;
        if(Plib::systemstate.config.report_url.c_str() != NULL)
            url = Plib::systemstate.config.report_url;
    }

    // create the abort description for the subsequent POST request
    string content = "email=" + Pol::Plib::systemstate.config.report_admin_email + "&"
                     "bin=" + Pol::Plib::systemstate.executable + "&"
                     "start_time=" + Pol::Plib::systemstate.getStartTime() + "&"
                     "abort_time=" + Pol::Clib::Logging::LogSink::getTimeStamp() + "&"
                     "reason=" + reason + "&"
                     "trace=" + stackTrace + "&"
                     "comp=" + getCompilerVersion() + "&"
                     "comp_time=" + Pol::compiledate + "(" + Pol::compiletime + ")&"
                     "build_target=" + Pol::polbuildtag + "&"
                     "build_revision=" + Pol::polverstr + "&"
                     "misc=";

    // execute the POST request
    doHttpPOST(host, url, content);
}

void handleExceptionSignal(int pSignal)
{
    switch(pSignal)
    {
        case SIGILL:
        case SIGFPE:
        case SIGSEGV:
        case SIGTERM:
        case SIGABRT:
            {
                /**
                 * inform the user about the program abort
                 */
                printf("########################################################################################\n");
                if(Plib::systemstate.config.report_program_aborts)
                    printf("POL will exit now. The following will be sent to the POL developers:\n");
                else
                    printf("POL will exit now. Please, post the following to the forum: http://forums.polserver.com/.\n");
                string tStackTrace = ExceptionParser::getTrace();
                printf("Admin contact: %s\n", Pol::Plib::systemstate.config.report_admin_email.c_str());
                printf("Executable: %s\n", Pol::Plib::systemstate.executable.c_str());
                printf("Start time: %s\n", Pol::Plib::systemstate.getStartTime().c_str());
                printf("Current time: %s\n", Pol::Clib::Logging::LogSink::getTimeStamp().c_str());
                printf("\n");
                printf("Stack trace:\n%s", tStackTrace.c_str());
                printf("\n");
                printf("Compiler: %s", getCompilerVersion().c_str());
                printf("Compile time: %s\n", Pol::compiletime);
                printf("Build target: %s\n", Pol::polbuildtag);
                printf("Build revision: %s\n", Pol::polverstr);
                #ifndef _WIN32
                    printf("GNU C library (compile time): %d.%d\n", __GLIBC__, __GLIBC_MINOR__);
                #endif
                printf("\n");
                printf("########################################################################################\n");

                /**
                 * use the program abort reporting system
                 */
                if(Plib::systemstate.config.report_program_aborts)
                {
                    string signalName;
                    string signalDescription;

                    getSignalDescription(pSignal, signalName, signalDescription);
                    ExceptionParser::reportProgramAbort(tStackTrace, "CRASH caused by signal " + signalName + " (" + signalDescription + ")");
                }

                // finally, go to hell
                exit(1);
            }
            break;
        default:
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////

ExceptionParser::ExceptionParser()
{
}

ExceptionParser::~ExceptionParser()
{
}

///////////////////////////////////////////////////////////////////////////////

#ifndef _WIN32
string ExceptionParser::getTrace()
{
    string result;

    void *stackTrace[MAX_STACK_TRACE_DEPTH];
    int stackTraceSize;
    char **stackTraceList;
    int stackTraceStep = 0;
    char *stringBuf = (char*)malloc(MAX_STACK_TRACE_STEP_LENGTH);

    stackTraceSize = backtrace(stackTrace, MAX_STACK_TRACE_DEPTH);
    stackTraceList = backtrace_symbols(stackTrace, stackTraceSize);

    size_t funcNameSize = 256;
    char* funcnName = (char*)malloc(funcNameSize);

    // iterate over all entries and do the demangling
    for ( int i = 0; i < stackTraceSize; i++ )
    {
        // get the pointers to the name, offset and end of offset
        char *beginFuncName = 0;
        char *beginFuncOffset = 0;
        char *endFuncOffset = 0;
        char *beginBinaryName = stackTraceList[i];
        char *beginBinaryOffset = 0;
        char *endBinaryOffset = 0;
        for (char *entryPointer = stackTraceList[i]; *entryPointer; ++entryPointer)
        {
            if (*entryPointer == '(')
            {
                beginFuncName = entryPointer;
            }else if (*entryPointer == '+')
            {
                beginFuncOffset = entryPointer;
            }else if (*entryPointer == ')' && beginFuncOffset)
            {
                endFuncOffset = entryPointer;
            }else if (*entryPointer == '[')
            {
                beginBinaryOffset = entryPointer;
            }else if (*entryPointer == ']' && beginBinaryOffset)
            {
                endBinaryOffset = entryPointer;
                break;
            }
        }

        // set the default value for the output line
        sprintf(stringBuf, "\n");

        // get the detailed values for the output line
        if (beginFuncName && beginFuncOffset && endFuncOffset && beginFuncName < beginFuncOffset)
        {
            // terminate the C strings
            *beginFuncName++ = '\0';
            *beginFuncOffset++ = '\0';
            *endFuncOffset = '\0';
            *beginBinaryOffset++ = '\0';
            *endBinaryOffset = '\0';

            int res;
            funcnName = abi::__cxa_demangle(beginFuncName, funcnName, &funcNameSize, &res);
            unsigned int binaryOffset = strtoul(beginBinaryOffset, NULL, 16);
            if (res == 0)
            {
                if(beginBinaryName && strlen(beginBinaryName))
                    sprintf(stringBuf, "#%02d 0x%016x in %s:[%s] from %s\n", stackTraceStep, binaryOffset, funcnName, beginFuncOffset, beginBinaryName);
                else
                    sprintf(stringBuf, "#%02d 0x%016x in %s from %s\n", stackTraceStep, binaryOffset, funcnName, beginFuncOffset);
                stackTraceStep++;
            }else{
                if(beginBinaryName && strlen(beginBinaryName))
                    sprintf(stringBuf, "#%02d 0x%016x in %s:[%s] from %s\n", stackTraceStep, binaryOffset, beginFuncName, beginFuncOffset, beginBinaryName);
                else
                    sprintf(stringBuf, "#%02d 0x%016x in %s:[%s]\n", stackTraceStep, binaryOffset, beginFuncName, beginFuncOffset);
                stackTraceStep++;
            }
        }else{
            sprintf(stringBuf, "#%02d %s\n", stackTraceStep, stackTraceList[i]);
            stackTraceStep++;
        }

        // append the line to the result
        result += string(stringBuf);
    }

    // memory cleanup
    free(funcnName);
    free(stackTraceList);

    return result;
}

static void handleSignalLinux(int pSignal, siginfo_t *pSignalInfo, void *pArg)
{
    logExceptionSignal(pSignal);
    if (pSignalInfo != NULL)
    {
        if(pSignal == SIGSEGV)
        {
            if(pSignalInfo->si_addr != NULL)
                printf("Segmentation fault detected - faulty memory reference at location: %p\n", pSignalInfo->si_addr);
            else
                printf("Segmentation fault detected - null pointer reference\n");
        }
        if (pSignalInfo->si_errno != 0)
            printf("This signal occurred because \"%s\"(%d)\n", strerror(pSignalInfo->si_errno), pSignalInfo->si_errno);
        if (pSignalInfo->si_code != 0)
            printf("Signal code is %d\n", pSignalInfo->si_code);
    }
    handleExceptionSignal(pSignal);
}

static void handleStackTraceRequestLinux(int signal, siginfo_t *signalInfo, void *arg)
{
    threadhelp::ThreadMap::Contents threadDesc;
    threadhelp::threadmap.CopyContents(threadDesc);

    fmt::Writer output;
    output << "STACK TRACE for thread \"" << threadDesc[pthread_self()] << "\"(" << pthread_self() << "):\n";
    output << ExceptionParser::getTrace() << "\n";

    // print to stdout
    printf("%s", output.c_str());

    // print to error output
    POLLOG_ERROR << output.c_str();

    // wait here for logging facility to make sure everything was processed
    if(Clib::Logging::global_logger)
      Clib::Logging::global_logger->wait_for_empty_queue();
}

void ExceptionParser::logAllStackTraces()
{
    threadhelp::ThreadMap::Contents threadsDesc;
    threadhelp::threadmap.CopyContents(threadsDesc);
    for (const auto& threadDesc : threadsDesc)
    {
        pthread_t threadID = (pthread_t)threadDesc.first;

        if(pthread_kill(threadID, SIGUSR1) != 0)
        {
            fmt::Writer output;
            output << "pthread_kill() failed to send SIGURS1 to thread " << threadsDesc[threadID] << "(" << threadID << ")\n";
            fprintf(stderr, "%s", output.c_str());
        }
    }
}

void ExceptionParser::initGlobalExceptionCatching()
{
    struct sigaction sigAction;

    memset(&sigAction, 0, sizeof(sigAction));
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_sigaction = handleSignalLinux;
    sigAction.sa_flags   = SA_SIGINFO;
    sigaction(SIGINT, &sigAction, NULL);
    sigaction(SIGTERM, &sigAction, NULL);
    sigaction(SIGSEGV, &sigAction, NULL);
    sigaction(SIGABRT, &sigAction, NULL);
    sigAction.sa_sigaction = handleStackTraceRequestLinux;
    sigaction(SIGUSR1, &sigAction, NULL);

    // set handler stack
    stack_t tStack;
    tStack.ss_sp = malloc(SIGSTKSZ);
    if (tStack.ss_sp == NULL)
    {
        printf("Could not allocate signal handler stack\n");
        exit(1);
    }
    tStack.ss_size = SIGSTKSZ;
    tStack.ss_flags = 0;
    if (sigaltstack(&tStack, NULL) == -1)
    {
        printf("Could not set signal handler stack\n");
        exit(1);
    }
}
#else // _WIN32
string ExceptionParser::getTrace()
{
    string result;

    return result;
}

void ExceptionParser::logAllStackTraces()
{

}

void ExceptionParser::initGlobalExceptionCatching()
{

}
#endif // _WIN32

///////////////////////////////////////////////////////////////////////////////

}} // namespaces
