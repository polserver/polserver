#include "ProgramConfig.h"
#include "pol_global_config.h"
#include "pol_revision.h"
#include <string>

namespace Pol
{
namespace Clib
{
using namespace std;

///////////////////////////////////////////////////////////////////////////////

std::string ProgramConfig::m_programName = "";
std::string ProgramConfig::m_programDir = "";

///////////////////////////////////////////////////////////////////////////////

void ProgramConfig::configureProgramEnvironment( const std::string& programName )
{
  m_programName = programName;

  std::string exeDir = programName;
  std::string::size_type bslashpos;
  while ( std::string::npos != ( bslashpos = exeDir.find( '\\' ) ) )
  {
    exeDir.replace( bslashpos, 1, 1, '/' );
  }

  std::string::size_type pos = exeDir.find_last_of( '/' );
  if ( pos != std::string::npos )
  {
    exeDir.erase( pos );
    exeDir += "/";
  }

  pos = exeDir.rfind( "/./" );
  if ( pos != std::string::npos )
  {
    exeDir.erase( pos - 1, 2 );
  }

  m_programDir = exeDir;
}

std::string ProgramConfig::programName()
{
  return m_programName;
}

std::string ProgramConfig::programDir()
{
  return m_programDir;
}

std::string ProgramConfig::build_target()
{
  return POL_BUILD_TARGET;
}

std::string ProgramConfig::build_datetime()
{
  return POL_BUILD_DATETIME;
}

///////////////////////////////////////////////////////////////////////////////
}
}  // namespaces
