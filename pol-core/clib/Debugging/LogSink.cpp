#include "LogSink.h"

#include <chrono>
#include <time.h>

#include "../clib.h"

namespace Pol
{
namespace Clib
{
namespace Logging
{
using std::chrono::system_clock;

void LogSink::addTimeStamp( std::ostream& stream )
{
  stream << getTimeStamp();
}

std::string LogSink::getTimeStamp()
{
  fmt::Writer writer;
  time_t tClockTime = system_clock::to_time_t( system_clock::now() );
  struct tm tTime = Clib::localtime( tClockTime );

  // write "[%m/%d %H:%M:%S] "
  writer << '[' << fmt::pad( tTime.tm_mon + 1, 2, '0' ) << '/' << fmt::pad( tTime.tm_mday, 2, '0' )
         << ' ' << fmt::pad( tTime.tm_hour, 2, '0' ) << ':' << fmt::pad( tTime.tm_min, 2, '0' )
         << ':' << fmt::pad( tTime.tm_sec, 2, '0' ) << "] ";

  return writer.str();
}

}  // namespace Logging
}  // namespace Clib
}  // namespace Pol
