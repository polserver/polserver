#include "ProgramMain.h"

#include <stdlib.h>

#include "../Debugging/ExceptionParser.h"
#include "../logfacility.h"
#include "ProgramConfig.h"
#include "pol_global_config.h"

#ifdef ENABLE_BENCHMARK
#include <benchmark/benchmark.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "../Header_Windows.h"
#include <crtdbg.h>
#include <psapi.h>
#include <windows.h>  // for GetModuleFileName

#pragma comment( lib, "psapi.lib" )  // 32bit is a bit dumb..
#else
#endif
#include <clocale>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace Pol
{
namespace Clib
{
using namespace std;

///////////////////////////////////////////////////////////////////////////////

ProgramMain::ProgramMain() {}
ProgramMain::~ProgramMain() {}
///////////////////////////////////////////////////////////////////////////////

void ProgramMain::start( int argc, char* argv[] )
{
  using namespace Pol;
  Clib::Logging::LogFacility logger;
  Clib::Logging::initLogging( &logger );
  Clib::ExceptionParser::initGlobalExceptionCatching();

  std::setlocale( LC_TIME, "" );
  std::setlocale( LC_CTYPE, "en_US.UTF-8" );  // unicode aware strings, unsure if english is enough,
                                              // or if it needs to be a user defined value

  int exitcode = 0;

  try
  {
// FIXME: 2008 Upgrades needed here? Make sure this still valid on 2008
#if defined( _WIN32 ) && defined( _DEBUG ) && _MSC_VER >= 1300
    // on VS.NET, disable obnoxious heap checking
    int flags = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
    flags &= 0x0000FFFF;  // set heap check frequency to 0
    _CrtSetDbgFlag( flags );
#endif

    /**********************************************
     * store program arguments
     **********************************************/
    m_programArguments.clear();
    for ( int i = 0; i < argc; i++ )
    {
      m_programArguments.push_back( std::string( argv[i] ) );
    }

    /**********************************************
     * determine and store the executable name
     **********************************************/
    std::string binaryPath = argv[0];
#ifdef WINDOWS
    char modulePath[_MAX_PATH];
    if ( GetModuleFileName( nullptr, modulePath, sizeof modulePath ) )
      binaryPath = modulePath;
#endif
    PROG_CONFIG::configureProgramEnvironment( binaryPath );

#ifdef ENABLE_BENCHMARK
    benchmark::Initialize( &argc, argv );
#endif

    /**********************************************
     * MAIN
     **********************************************/
    exitcode = main();
  }
  catch ( const char* msg )
  {
    ERROR_PRINT << "Execution aborted due to: " << msg << "\n";
    exitcode = 1;
  }
  catch ( std::string& str )
  {
    ERROR_PRINT << "Execution aborted due to: " << str << "\n";
    exitcode = 1;
  }                                 // egcs has some trouble realizing 'exception' should catch
  catch ( std::runtime_error& re )  // runtime_errors, so...
  {
    ERROR_PRINT << "Execution aborted due to: " << re.what() << "\n";
    exitcode = 1;
  }
  catch ( std::exception& ex )
  {
    ERROR_PRINT << "Execution aborted due to: " << ex.what() << "\n";
    exitcode = 1;
  }
  catch ( int xn )
  {
    // Something that throws an integer is responsible for printing
    // its own error message.
    // "throw 3" is meant as an alternative to exit(3).
    exitcode = xn;
  }
#ifndef _WIN32
  catch ( ... )
  {
    ERROR_PRINT << "Execution aborted due to generic exception."
                << "\n";
    exitcode = 2;
  }
#endif
  Clib::Logging::global_logger->wait_for_empty_queue();

  exit( exitcode );
}

const std::vector<std::string>& ProgramMain::programArgs() const
{
  return m_programArguments;
}

bool ProgramMain::programArgsFind( const std::string& filter, std::string* rest ) const
{
  const std::vector<std::string>& binArgs = programArgs();
  for ( size_t i = 1; i < binArgs.size(); i++ )
  {
    const std::string& param = binArgs[i];
    switch ( param[0] )
    {
    case '/':
    case '-':
      if ( param.substr( 1, filter.size() ) == filter )
      {
        if ( rest != nullptr )
          *rest = param.substr( 1 + filter.size(), param.size() - ( 1 + filter.size() ) );
        return true;
      }
      break;
    default:
      break;
    }
  }
  return false;
}

std::string ProgramMain::programArgsFindEquals( const std::string& filter,
                                                std::string defaultVal ) const
{
  const std::vector<std::string>& binArgs = programArgs();
  for ( size_t i = 1; i < binArgs.size(); i++ )
  {
    const std::string& param = binArgs[i];
    if ( param.substr( 0, filter.size() ) == filter )
      return param.substr( filter.size(), param.size() - ( filter.size() ) );
  }
  return defaultVal;
}

int ProgramMain::programArgsFindEquals( const std::string& filter, int defaultVal,
                                        bool hexVal ) const
{
  std::string val = programArgsFindEquals( filter, "" );
  if ( val.empty() )
    return defaultVal;
  return strtoul( val.c_str(), nullptr, hexVal ? 16 : 10 );
}
}
}  // namespaces
