/*
ATTENTION:
This header is part of the PCH
Remove the include in all StdAfx.h files or live with the consequences :)
*/

#pragma once

#include <chrono>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fstream>
#include <future>
#include <map>
#include <memory>
#include <thread>
#include <type_traits>
#include <vector>

#include "Debugging/LogSink.h"

namespace Pol
{
namespace Clib
{
extern bool LogfileTimestampEveryLine;

namespace Logging
{
constexpr char CONSOLE_RESET_COLOR[] = "\x1b[0m";

struct LogFileBehaviour;
class LogFacility;

// generic sink to log into a file
class LogSinkGenericFile : public LogSink
{
public:
  LogSinkGenericFile( const LogFileBehaviour* behaviour );
  LogSinkGenericFile();
  ~LogSinkGenericFile() override;
  void open_log_file( bool open_timestamp );
  void setBehaviour( const LogFileBehaviour* behaviour, std::string filename );
  void addMessage( const std::string& msg ) override;
  void addMessage( const std::string& msg, const std::string& id ) override;

protected:
  friend class LogFacility;

  bool test_for_rollover( std::chrono::time_point<std::chrono::system_clock>& now );
  const LogFileBehaviour* _behaviour;
  std::ofstream _filestream;
  std::string _log_filename;
  struct tm _opened;
  std::chrono::time_point<std::chrono::system_clock> _lasttimestamp;
  bool _active_line;
  static bool _disabled;
};

// template function to get the instance of given sink
template <typename Sink>
Sink* getSink();

// std::cout sink
class LogSink_cout final : public LogSink
{
public:
  LogSink_cout();
  ~LogSink_cout() override = default;
  void addMessage( const std::string& msg ) override;
  void addMessage( const std::string& msg, const std::string& id ) override;
};

// std::cerr sink
class LogSink_cerr final : public LogSink
{
public:
  LogSink_cerr();
  ~LogSink_cerr() override = default;
  void addMessage( const std::string& msg ) override;
  void addMessage( const std::string& msg, const std::string& id ) override;
};

// pol.log (and start.log) file sink
class LogSink_pollog final : public LogSinkGenericFile
{
public:
  LogSink_pollog();
  ~LogSink_pollog() override = default;
  void deinitialize_startlog();
};

// script.log file sink
class LogSink_scriptlog final : public LogSinkGenericFile
{
public:
  LogSink_scriptlog();
  ~LogSink_scriptlog() override = default;
};

// debug.log file sink
class LogSink_debuglog final : public LogSinkGenericFile
{
public:
  LogSink_debuglog();
  ~LogSink_debuglog() override = default;
  void addMessage( const std::string& msg ) override;
  void addMessage( const std::string& msg, const std::string& id ) override;
  void disable();
  static bool Disabled;
};

// leak.log file sink
class LogSink_leaklog final : public LogSinkGenericFile
{
public:
  LogSink_leaklog();
  ~LogSink_leaklog() override = default;
};

class LogSink_flexlog final : public LogSink
{
public:
  LogSink_flexlog();
  ~LogSink_flexlog() override = default;
  std::string create( std::string logfilename, bool open_timestamp );
  void addMessage( const std::string& msg ) override;
  void addMessage( const std::string& msg, const std::string& id ) override;
  void close( const std::string& id );

private:
  std::map<std::string, std::shared_ptr<LogSinkGenericFile>> _logfiles;
};
// template class to perform a dual log eg. cout + pol.log
template <typename log1, typename log2>
class LogSink_dual final : public LogSink
{
public:
  LogSink_dual();
  ~LogSink_dual() override = default;
  void addMessage( const std::string& msg ) override;
  void addMessage( const std::string& msg, const std::string& id ) override;
};

// main class which starts the logging
class LogFacility
{
public:
  LogFacility();
  ~LogFacility();
  LogFacility( const LogFacility& ) = delete;
  LogFacility& operator=( const LogFacility& ) = delete;
  template <typename Sink>
  void save( std::string message, std::string id );
  void registerSink( LogSink* sink );
  void disableDebugLog();
  void disableFileLog();
  void deinitializeStartLog();
  void closeFlexLog( const std::string& id );
  std::string registerFlexLogger( const std::string& logfilename, bool open_timestamp );
  void wait_for_empty_queue();

  static bool _vsDebuggerPresent;

private:
  class LogWorker;
  std::unique_ptr<LogWorker> _worker;
  std::vector<LogSink*> _registered_sinks;
};


// concepts to choose between compiletime formatting and runtime
// fmt::format_string still needs be the parameter type to be able to do compile time checks of the
// formatting, otherwise it would be done at runtime
template <typename T, typename... Args>
concept ConstructableFmtString = requires( T str ) { fmt::format_string<Args...>{ str }; };
template <typename T>
concept RuntimeString = std::same_as<T, std::string> || std::is_pointer_v<T>;
template <typename T, typename... Args>
concept RunTimeFmt = ConstructableFmtString<T, Args...> && RuntimeString<T>;

// macro struct for logging entrypoint
// performs the actual formatting and sending to sink
template <typename Sink>
struct Message
{
  template <bool newline, typename... Args>
  static void logmsg( fmt::format_string<Args...> format_str, Args&&... args )
  {
    try
    {
      if constexpr ( newline )
        send( fmt::format( format_str, std::forward<Args>( args )... ) + '\n' );
      else
        send( fmt::format( format_str, std::forward<Args>( args )... ) );
    }
    catch ( ... )
    {
      const auto& fview = format_str.str;
      send( std::string( "failed to format compile: " ) +
            std::string( fview.begin(), fview.end() ) + "\n" );
    }
  }
  template <bool newline>
  static void logmsg( RunTimeFmt auto const& format, auto&&... args )
  {
    try
    {
      if constexpr ( sizeof...( args ) == 0 )
      {
        if constexpr ( newline )
          send( std::string( format ) + '\n' );
        else
          send( std::string( format ) );
      }
      else
      {
        if constexpr ( newline )
          send( fmt::format( fmt::runtime( format ), std::forward<decltype( args )>( args )... ) +
                '\n' );
        else
          send( fmt::format( fmt::runtime( format ), std::forward<decltype( args )>( args )... ) );
      }
    }
    catch ( ... )
    {
      send( std::string( "failed to format runtime: " ) + format + '\n' );
    }
  }

  template <typename... Args>
  static void logmsglnID( const std::string& id, fmt::format_string<Args...> format_str,
                          Args&&... args )
  {
    send( fmt::format( format_str, std::forward<Args>( args )... ) + '\n', id );
  }

  static void logmsglnID( const std::string& id, RunTimeFmt auto const& format, auto&&... args )
  {
    try
    {
      if constexpr ( sizeof...( args ) == 0 )
        send( std::string( format ) + '\n', id );
      else
        send(
            fmt::format( fmt::runtime( format ), std::forward<decltype( args )>( args )... ) + '\n',
            id );
    }
    catch ( ... )
    {
      send( std::string( "failed to format: " ) + format + '\n', id );
    }
  }

private:
  static void send( std::string msg, std::string id = {} );
};

extern LogFacility* global_logger;        // pointer to the instance of the main class
void initLogging( LogFacility* logger );  // initalize the logging
}  // namespace Logging
}  // namespace Clib


// several helper defines

// log into pol.log and std::cerr with \n addition
#define POLLOG_ERRORLN                                                            \
  Clib::Logging::Message<Clib::Logging::LogSink_dual<Clib::Logging::LogSink_cerr, \
                                                     Clib::Logging::LogSink_pollog>>::logmsg<true>
// log into pol.log and std::cerr without \n addition
#define POLLOG_ERROR                                                              \
  Clib::Logging::Message<Clib::Logging::LogSink_dual<Clib::Logging::LogSink_cerr, \
                                                     Clib::Logging::LogSink_pollog>>::logmsg<true>

// log into pol.log and std::cout with \n addition
#define POLLOG_INFOLN                                                             \
  Clib::Logging::Message<Clib::Logging::LogSink_dual<Clib::Logging::LogSink_cout, \
                                                     Clib::Logging::LogSink_pollog>>::logmsg<true>
// log into pol.log and std::cout without \n addition
#define POLLOG_INFO                                   \
  Clib::Logging::Message<Clib::Logging::LogSink_dual< \
      Clib::Logging::LogSink_cout, Clib::Logging::LogSink_pollog>>::logmsg<false>

// log into pol.log with \n addition
#define POLLOGLN Clib::Logging::Message<Clib::Logging::LogSink_pollog>::logmsg<true>
// log into pol.log without \n addition
#define POLLOG Clib::Logging::Message<Clib::Logging::LogSink_pollog>::logmsg<false>

// log only into std::cout with \n addition
#define INFO_PRINTLN Clib::Logging::Message<Clib::Logging::LogSink_cout>::logmsg<true>
// log only into std::cout without \n addition
#define INFO_PRINT Clib::Logging::Message<Clib::Logging::LogSink_cout>::logmsg<false>
// log only into std::cout if level is equal or higher with \n addition
#define INFO_PRINTLN_TRACE( n )                    \
  if ( Plib::systemstate.config.debug_level >= n ) \
  INFO_PRINTLN

// log only into std::cerr with \n addition
#define ERROR_PRINTLN Clib::Logging::Message<Clib::Logging::LogSink_cerr>::logmsg<true>
// log only into std::cerr without \n addition
#define ERROR_PRINT Clib::Logging::Message<Clib::Logging::LogSink_cerr>::logmsg<false>

// log into script.log with \n addition
#define SCRIPTLOGLN Clib::Logging::Message<Clib::Logging::LogSink_scriptlog>::logmsg<true>
// log into script.log without \n addition
#define SCRIPTLOG Clib::Logging::Message<Clib::Logging::LogSink_scriptlog>::logmsg<false>

// log into debug.log (if enabled) with \n addition
#define DEBUGLOGLN                                  \
  if ( !Clib::Logging::LogSink_debuglog::Disabled ) \
  Clib::Logging::Message<Clib::Logging::LogSink_debuglog>::logmsg<true>

// log into leak.log without \n addition
#define LEAKLOG Clib::Logging::Message<Clib::Logging::LogSink_leaklog>::logmsg<false>
// log into leak.log with \n addition
#define LEAKLOGLN Clib::Logging::Message<Clib::Logging::LogSink_leaklog>::logmsg<true>

// log into sink id with \n addition, need a call of OPEN_LOG before
#define FLEXLOGLN Clib::Logging::Message<Clib::Logging::LogSink_flexlog>::logmsglnID
// open logfile of given filename, returns unique unsigned int for usage of logging/closing
#define OPEN_FLEXLOG( filename, open_timestamp ) \
  Clib::Logging::global_logger->registerFlexLogger( filename, open_timestamp )
// close logfile of given id
#define CLOSE_FLEXLOG( id ) Clib::Logging::global_logger->closeFlexLog( id )

// performs the switch between start.log and pol.log
#define DEINIT_STARTLOG Clib::Logging::global_logger->deinitializeStartLog

// disables the debug.log
#define DISABLE_DEBUGLOG Clib::Logging::global_logger->disableDebugLog
// helper bool if the debug.log is disabled (for more complex logging) DEBUGLOG checks also
#define IS_DEBUGLOG_DISABLED Clib::Logging::LogSink_debuglog::Disabled

#define GET_LOG_FILESTAMP Clib::Logging::LogSink::getTimeStamp()
}  // namespace Pol
