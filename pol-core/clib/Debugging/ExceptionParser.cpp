#include "ExceptionParser.h"
#include "LogSink.h"

#include "plib/systemstate.h"
#include "plib/polver.h"

#include <cstring>
#include <signal.h>

#ifndef _WIN32
#include <execinfo.h>
#include <cxxabi.h>
#include <unistd.h>
#include <sys/syscall.h>
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

std::string getCompilerVersion()
{
	#ifndef _WIN32
		char result[256];
		sprintf(result, "gcc %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
	#else
		std::string result;
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
			if(_MSC_VER > 1800)
				result = "MSVC++ newer than version 12.0"
			else if (_MSC_VER < 1100)
				result = "MSVC++ older than version 5.0"
			else
				result = "MSVC++ (some unsupported version)";
			break;
		}
	#endif

	return result;
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
                printf("########################################################################################\n");
                printf("POL will exit now. Please post the following on http://forums.polserver.com/tracker.php.\n");
                string tStackTrace = ExceptionParser::getTrace();
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

void ExceptionParser::initGlobalExceptionCatching()
{

}
#endif // _WIN32

///////////////////////////////////////////////////////////////////////////////

}} // namespaces
