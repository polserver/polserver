#include "LogSink.h"
#include "../clib.h"

#include <chrono>

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

void LogSink::addTimeStamp(ostream &stream)
{
	stream << getTimeStamp();
}

string LogSink::getTimeStamp()
{
    fmt::Writer writer;
    time_t tClockTime = system_clock::to_time_t(system_clock::now());
    struct tm tTime = Clib::localtime(tClockTime);

    // write "[%m/%d %H:%M:%S] "
    writer << '['
        << fmt::pad(tTime.tm_mon+1, 2, '0') << '/'
        << fmt::pad(tTime.tm_mday, 2, '0') << ' '
        << fmt::pad(tTime.tm_hour, 2, '0') << ':'
        << fmt::pad(tTime.tm_min, 2, '0') << ':'
        << fmt::pad(tTime.tm_sec, 2, '0') << "] ";

    return writer.str();
}

///////////////////////////////////////////////////////////////////////////////

}}} // namespaces
