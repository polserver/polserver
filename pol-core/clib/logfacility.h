/*
ATTENTION:
This header is part of the PCH
Remove the include in all StdAfx.h files or live with the consequences :)
*/

#ifndef CLIB_LOGFACILITY_H
#define CLIB_LOGFACILITY_H

#include <format/format.h>
#include <fstream>
#include <future>
#include <map>
#include <memory>
#include <thread>
#include <vector>

#include "Debugging/LogSink.h"

namespace Pol
{
namespace Clib
{
extern bool LogfileTimestampEveryLine;

namespace Logging
{
struct LogFileBehaviour;
class LogFacility;

// generic sink to log into a file
class LogSinkGenericFile : public LogSink
{
public:
  LogSinkGenericFile( const LogFileBehaviour* behaviour );
  LogSinkGenericFile();
  virtual ~LogSinkGenericFile();
  void open_log_file( bool open_timestamp );
  void setBehaviour( const LogFileBehaviour* behaviour, std::string filename );
  virtual void addMessage( fmt::Writer* msg ) override;
  virtual void addMessage( fmt::Writer* msg, const std::string& id ) override;

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
  virtual ~LogSink_cout() = default;
  virtual void addMessage( fmt::Writer* msg ) override;
  virtual void addMessage( fmt::Writer* msg, const std::string& id ) override;
};

// std::cerr sink
class LogSink_cerr final : public LogSink
{
public:
  LogSink_cerr();
  virtual ~LogSink_cerr() = default;
  virtual void addMessage( fmt::Writer* msg ) override;
  virtual void addMessage( fmt::Writer* msg, const std::string& id ) override;
};

// pol.log (and start.log) file sink
class LogSink_pollog final : public LogSinkGenericFile
{
public:
  LogSink_pollog();
  virtual ~LogSink_pollog() = default;
  void deinitialize_startlog();
};

// script.log file sink
class LogSink_scriptlog final : public LogSinkGenericFile
{
public:
  LogSink_scriptlog();
  virtual ~LogSink_scriptlog() = default;
};

#ifdef ENABLE_NODEJS
// debug.log file sink
class LogSink_nodelog final : public LogSinkGenericFile
{
public:
  LogSink_nodelog();
  virtual ~LogSink_nodelog() = default;
  virtual void addMessage( fmt::Writer* msg ) override;
  virtual void addMessage( fmt::Writer* msg, const std::string& id ) override;
  void disable();
  static bool Disabled;
};
#endif


// debug.log file sink
class LogSink_debuglog final : public LogSinkGenericFile
{
public:
  LogSink_debuglog();
  virtual ~LogSink_debuglog() = default;
  virtual void addMessage( fmt::Writer* msg ) override;
  virtual void addMessage( fmt::Writer* msg, const std::string& id ) override;
  void disable();
  static bool Disabled;
};

// leak.log file sink
class LogSink_leaklog final : public LogSinkGenericFile
{
public:
  LogSink_leaklog();
  virtual ~LogSink_leaklog() = default;
};

class LogSink_flexlog final : public LogSink
{
public:
  LogSink_flexlog();
  virtual ~LogSink_flexlog() = default;
  std::string create( std::string logfilename, bool open_timestamp );
  virtual void addMessage( fmt::Writer* msg ) override;
  virtual void addMessage( fmt::Writer* msg, const std::string& id ) override;
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
  virtual ~LogSink_dual() = default;
  virtual void addMessage( fmt::Writer* msg ) override;
  virtual void addMessage( fmt::Writer* msg, const std::string& id ) override;
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
  void save( fmt::Writer* message, const std::string& id );
  void registerSink( LogSink* sink );
  void disableDebugLog();
  void disableNodeLog();
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

// construct a message for given sink, on deconstruction sends the msg to the facility
template <typename Sink>
class Message
{
public:
  Message();
  Message( const std::string& id );
  Message(
      const std::string& file, const int line,
      const std::string& function );  // for internal stuff with __FILE__, __LINE__, __FUNCTION__
  Message(
      const std::string& id, const std::string& file, const int line,
      const std::string& function );  // for internal stuff with __FILE__, __LINE__, __FUNCTION__
  ~Message();                         // auto flush

  fmt::Writer& message() { return *( _formater.get() ); }

private:
  std::unique_ptr<fmt::Writer> _formater;
  std::string _id;
};


extern LogFacility* global_logger;        // pointer to the instance of the main class
void initLogging( LogFacility* logger );  // initalize the logging
}  // namespace Logging
}  // namespace Clib


// several helper defines
//#define DEBUG_LOG_PRINTS
#ifdef DEBUG_LOG_PRINTS
#ifdef WINDOWS
#define __FILENAME__ ( strrchr( __FILE__, '\\' ) ? strrchr( __FILE__, '\\' ) + 1 : __FILE__ )
#else
#define __FILENAME__ ( strrchr( __FILE__, '/' ) ? strrchr( __FILE__, '/' ) + 1 : __FILE__ )
#endif
#define LOG_PRINT_CALLER_INFO __FILENAME__, __LINE__, __FUNCTION__
#define LOG_PRINT_CALLER_INFO2 , __FILENAME__, __LINE__, __FUNCTION__
#else
#define LOG_PRINT_CALLER_INFO
#define LOG_PRINT_CALLER_INFO2
#endif

// log into pol.log and std::cerr
#define POLLOG_ERROR                                                                            \
  Clib::Logging::Message<                                                                       \
      Clib::Logging::LogSink_dual<Clib::Logging::LogSink_cerr, Clib::Logging::LogSink_pollog>>( \
      LOG_PRINT_CALLER_INFO )                                                                   \
      .message()
// log into pol.log and std::cout
#define POLLOG_INFO                                                                             \
  Clib::Logging::Message<                                                                       \
      Clib::Logging::LogSink_dual<Clib::Logging::LogSink_cout, Clib::Logging::LogSink_pollog>>( \
      LOG_PRINT_CALLER_INFO )                                                                   \
      .message()
// log into pol.log
#define POLLOG \
  Clib::Logging::Message<Clib::Logging::LogSink_pollog>( LOG_PRINT_CALLER_INFO ).message()

// log only into std::cout
#define INFO_PRINT \
  Clib::Logging::Message<Clib::Logging::LogSink_cout>( LOG_PRINT_CALLER_INFO ).message()
// log only into std::cout if level is equal or higher
#define INFO_PRINT_TRACE( n )                      \
  if ( Plib::systemstate.config.debug_level >= n ) \
  INFO_PRINT
// log only into std::cerr
#define ERROR_PRINT \
  Clib::Logging::Message<Clib::Logging::LogSink_cerr>( LOG_PRINT_CALLER_INFO ).message()

// log into script.log
#define SCRIPTLOG \
  Clib::Logging::Message<Clib::Logging::LogSink_scriptlog>( LOG_PRINT_CALLER_INFO ).message()
// log into node.log (if enabled)
#define NODELOG                                    \
  if ( !Clib::Logging::LogSink_nodelog::Disabled ) \
  Clib::Logging::Message<Clib::Logging::LogSink_nodelog>( LOG_PRINT_CALLER_INFO ).message()
// log into debug.log (if enabled)
#define DEBUGLOG                                    \
  if ( !Clib::Logging::LogSink_debuglog::Disabled ) \
  Clib::Logging::Message<Clib::Logging::LogSink_debuglog>( LOG_PRINT_CALLER_INFO ).message()
// log into leak.log
#define LEAKLOG \
  Clib::Logging::Message<Clib::Logging::LogSink_leaklog>( LOG_PRINT_CALLER_INFO ).message()

// log into sink id need a call of OPEN_LOG before
#define FLEXLOG( id ) \
  Clib::Logging::Message<Clib::Logging::LogSink_flexlog>( id LOG_PRINT_CALLER_INFO2 ).message()
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

// disables the debug.log
#define DISABLE_NODELOG Clib::Logging::global_logger->disableNodeLog
// helper bool if the debug.log is disabled (for more complex logging) DEBUGLOG checks also
#define IS_NODELOG_DISABLED Clib::Logging::LogSink_nodelog::Disabled

#define GET_LOG_FILESTAMP Clib::Logging::LogSink::getTimeStamp()
}  // namespace Pol

#endif  // CLIB_LOGFACILITY_H
