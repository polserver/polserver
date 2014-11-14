#include "LogSink.h"

#include <chrono>

#ifdef _MSC_VER
#pragma warning(disable:4996) // localtime() deprecation warning
#endif

namespace Pol{ namespace Clib{ namespace Logging{
using namespace std;
using std::chrono::system_clock;

///////////////////////////////////////////////////////////////////////////////

LogSink::LogSink()
{
}

LogSink::~LogSink()
{
}

///////////////////////////////////////////////////////////////////////////////

void LogSink::printCurrentTimeStamp(ostream &pStream)
{
	pStream << getLoggingTimeStamp();
}

// put_time is not yet implemented in gcc, and i dont like strftime
string LogSink::getLoggingTimeStamp()
{
	fmt::Writer tWrite; // write '[%m/%d %H:%M:%S] '
	time_t tClockTime = system_clock::to_time_t(system_clock::now());
	struct tm* tTime = localtime(&tClockTime);

	tWrite << '['
		<< fmt::pad(tTime->tm_mon+1, 2, '0') << '/'
		<< fmt::pad(tTime->tm_mday, 2, '0') << ' '
		<< fmt::pad(tTime->tm_hour, 2, '0') << ':'
		<< fmt::pad(tTime->tm_min, 2, '0') << ':'
		<< fmt::pad(tTime->tm_sec, 2, '0') << "] ";

	return tWrite.str();
}

///////////////////////////////////////////////////////////////////////////////

}}} // namespaces
