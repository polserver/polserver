#include "ProgramConfig.h"
#include <string>

namespace Pol{ namespace Clib{
using namespace std;

///////////////////////////////////////////////////////////////////////////////

std::string ProgramConfig::m_programName = "";
std::string ProgramConfig::m_programDir = "";

///////////////////////////////////////////////////////////////////////////////

ProgramConfig::ProgramConfig()
{
}
ProgramConfig::~ProgramConfig()
{
}

///////////////////////////////////////////////////////////////////////////////

void ProgramConfig::configureProgramEnvironment(std::string programName)
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

std::string ProgramConfig::programName()
{
    return m_programName;
}

std::string ProgramConfig::programDir()
{
    return m_programDir;
}

///////////////////////////////////////////////////////////////////////////////

}} // namespaces
