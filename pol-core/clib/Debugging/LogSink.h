/*
ATTENTION:
This header is part of the PCH
Remove the include in all StdAfx.h files or live with the consequences :)
*/
#pragma once

#include <iosfwd>
#include <string>

namespace Pol::Clib::Logging
{
class LogSink
{
public:
  LogSink() = default;
  virtual ~LogSink() = default;

  LogSink( const LogSink& ) = delete;
  LogSink& operator=( const LogSink& ) = delete;

  virtual void addMessage( const std::string& msg ) = 0;
  virtual void addMessage( const std::string& msg, const std::string& id ) = 0;

  /**
   * Helper function to print timestamp into stream
   */
  static void addTimeStamp( std::ostream& stream );

  /**
   * Formats and returns current time as std::string
   */
  static std::string getTimeStamp();
};
}  // namespace Pol::Clib::Logging
