#include <ProgramMain.h>

#include "Debugging/ExceptionParser.h"
#include "Program/ProgramConfig.h"

#include "clib.h"
#include "logfacility.h"
#include "strexcpt.h"

#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#    include <windows.h> // for GetModuleFileName
#    include <crtdbg.h>
#   include <psapi.h>
#   pragma comment(lib, "psapi.lib") // 32bit is a bit dumb..
#else
#   include <unistd.h>
#   include <sys/resource.h>
#endif
#include <stdexcept>
#include <string>

namespace Pol{ namespace Clib{
using namespace std;

///////////////////////////////////////////////////////////////////////////////

ProgramMain::ProgramMain()
{
}
ProgramMain::~ProgramMain()
{
}
///////////////////////////////////////////////////////////////////////////////

void ProgramMain::start(int argc, char *argv[])
{
    using namespace Pol;
    Clib::Logging::LogFacility logger;
    Clib::Logging::initLogging( &logger );
    Clib::ExceptionParser::initGlobalExceptionCatching();

    setlocale(LC_TIME,"");
    int exitcode = 0;

    try
    {
        // FIXME: 2008 Upgrades needed here? Make sure this still valid on 2008
#if defined(_WIN32) && defined(_DEBUG) && _MSC_VER >= 1300
        // on VS.NET, disable obnoxious heap checking
        int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
        flags &= 0x0000FFFF; // set heap check frequency to 0
        _CrtSetDbgFlag( flags );
#endif

        /**********************************************
         * store program arguments
         **********************************************/
        m_programArguments.clear();
        for(int i = 0; i < argc; i++)
        {
        	m_programArguments.push_back(std::string(argv[i]));
        }

        /**********************************************
         * determine and store the executable name
         **********************************************/
        std::string binaryPath = argv[0];
        #ifdef WINDOWS
            char modulePath[_MAX_PATH];
            if (GetModuleFileName(NULL, modulePath, sizeof modulePath))
            	binaryPath = modulePath;
        #endif
        PROG_CONFIG::configureProgramEnvironment(binaryPath);

        /**********************************************
         * MAIN
         **********************************************/
        exitcode = main();
    }
    catch( const char *msg )
    {
        ERROR_PRINT << "Execution aborted due to: " << msg << "\n";
        exitcode = 1;
    }
    catch( std::string& str )
    {
      ERROR_PRINT << "Execution aborted due to: " << str << "\n";
        exitcode = 1;
    }       // egcs has some trouble realizing 'exception' should catch
    catch (std::runtime_error& re)   // runtime_errors, so...
    {
      ERROR_PRINT << "Execution aborted due to: " << re.what( ) << "\n";
        exitcode = 1;
    }
    catch (std::exception& ex)
    {
      ERROR_PRINT << "Execution aborted due to: " << ex.what( ) << "\n";
        exitcode = 1;
    }
    catch( int xn )
    {
        // Something that throws an integer is responsible for printing
        // its own error message.
        // "throw 3" is meant as an alternative to exit(3).
        exitcode = xn;
    }
#ifndef _WIN32
    catch( ... )
    {
      ERROR_PRINT << "Execution aborted due to generic exception." << "\n";
        exitcode = 2;
    }
#endif
    Clib::Logging::global_logger->wait_for_empty_queue();

    exit(exitcode);
}

const std::vector<std::string>& ProgramMain::programArgs()
{
	return m_programArguments;
}

std::string ProgramMain::programArgsFind(std::string filter)
{
	const std::vector<std::string> binArgs = programArgs();
	for (int i = 1; i < (int)binArgs.size(); i++)
	{
		const std::string& param = binArgs[i];
		switch (param[0])
		{
			case '/':
			case '-':
				if (param.substr(1, filter.size()) == filter)
					return param.substr(1 + filter.size(), param.size() - (1 + filter.size()));
				break;
			default:
				break;
		}
	}
	return "";
}

std::string ProgramMain::programArgsFindEquals(std::string filter, std::string defaultVal)
{
	const std::vector<std::string> binArgs = programArgs();
	for (int i = 1; i < (int)binArgs.size(); i++)
	{
		const std::string& param = binArgs[i];
		if (param.substr(0, filter.size()) == filter)
			return param.substr(filter.size(), param.size() - (filter.size()));
	}
	return defaultVal;
}

}} // namespaces
