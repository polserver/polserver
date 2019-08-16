/*
ATTENTION:
This header is part of the PCH
Remove the include in all StdAfx.h files or live with the consequences :)
*/
#ifndef CLIB_LOG_SINK_H
#define CLIB_LOG_SINK_H

#include <iosfwd>
#include <string>

#include <format/format.h>

namespace Pol
{
namespace Clib
{
namespace Logging
{
class LogSink
{
public:
  LogSink() = default;
  virtual ~LogSink() = default;

  LogSink( const LogSink& ) = delete;
  LogSink& operator=( const LogSink& ) = delete;

  virtual void addMessage( fmt::Writer* msg ) = 0;
  virtual void addMessage( fmt::Writer* msg, const std::string& id ) = 0;

  /**
   * Helper function to print timestamp into stream
   */
  static void addTimeStamp( std::ostream& stream );

  /**
   * Formats and returns current time as std::string
   */
  static std::string getTimeStamp();
};
}  // namespace Logging
}  // namespace Clib
}  // namespace Pol

#endif
