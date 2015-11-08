#include "ConfigEnvironment.h"
#include <string>

namespace Pol{ namespace Clib{
using namespace std;

///////////////////////////////////////////////////////////////////////////////

std::string ConfigEnvironment::m_programName = "";
std::string ConfigEnvironment::m_programDir = "";

///////////////////////////////////////////////////////////////////////////////

void ConfigEnvironment::configureProgramEnvironment(std::string programName)
{
    m_programName = programName;

	std::string exeDir = programName;
	std::string::size_type bslashpos;
	while (std::string::npos != (bslashpos = exeDir.find('\\')))
	{
		exeDir.replace(bslashpos, 1, 1, '/');
	}

	std::string::size_type pos = exeDir.find_last_of("/");
	if (pos != std::string::npos)
	{
		exeDir.erase(pos);
		exeDir += "/";
	}
	m_programDir = exeDir;
}

std::string ConfigEnvironment::programName()
{
    return m_programName;
}

std::string ConfigEnvironment::programDir()
{
    return m_programDir;
}

///////////////////////////////////////////////////////////////////////////////

}} // namespaces
