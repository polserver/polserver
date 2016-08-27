/*
ATTENTION:
This header is part of the PCH
Remove the include in all StdAfx.h files or live with the consequences :)
*/

#ifndef CLIB_LOGFACILITY_H
#define CLIB_LOGFACILITY_H

#include <vector>
#include <map>
#include <thread>
#include <future>
#include <memory>
#include <fstream>
#include <boost/noncopyable.hpp>
#include "../../lib/format/format.h"

#include "compilerspecifics.h"
#include "Debugging/LogSink.h"
#include "message_queue.h"

namespace Pol
{
namespace Core
{
struct PolConfig;
}
namespace Clib
{
extern bool LogfileTimestampEveryLine;

namespace Logging
{
struct LogFileBehaviour;

// generic sink to log into a file
class LogSinkGenericFile : public LogSink
{
public:
  LogSinkGenericFile( const LogFileBehaviour* behaviour );
  LogSinkGenericFile();
  virtual ~LogSinkGenericFile();
  void open_log_file( bool open_timestamp );
  void setBehaviour( const LogFileBehaviour* behaviour, std::string filename );
  virtual void addMessage( fmt::Writer* msg ) POL_OVERRIDE;
  virtual void addMessage( fmt::Writer* msg, std::string id ) POL_OVERRIDE;

protected:
  bool test_for_rollover( std::chrono::time_point<std::chrono::system_clock>& now );
  const LogFileBehaviour* _behaviour;
  std::ofstream _filestream;
  std::string _log_filename;
  struct tm _opened;
  std::chrono::time_point<std::chrono::system_clock> _lasttimestamp;
  bool _active_line;
};

// template function to get the instance of given sink
template <typename Sink>
Sink* getSink();

// std::cout sink
class LogSink_cout : public LogSink
{
public:
  LogSink_cout();
  virtual ~LogSink_cout(){};
  virtual void addMessage( fmt::Writer* msg ) POL_OVERRIDE;
  virtual void addMessage( fmt::Writer* msg, std::string id ) POL_OVERRIDE;
};

// std::cerr sink
class LogSink_cerr : public LogSink
{
public:
  LogSink_cerr();
  virtual ~LogSink_cerr(){};
  virtual void addMessage( fmt::Writer* msg ) POL_OVERRIDE;
  virtual void addMessage( fmt::Writer* msg, std::string id ) POL_OVERRIDE;
};

// pol.log (and start.log) file sink
class LogSink_pollog : public LogSinkGenericFile
{
public:
  LogSink_pollog();
  virtual ~LogSink_pollog(){};
  void deinitialize_startlog();
};

// script.log file sink
class LogSink_scriptlog : public LogSinkGenericFile
{
public:
  LogSink_scriptlog();
  virtual ~LogSink_scriptlog(){};
};

// debug.log file sink
class LogSink_debuglog : public LogSinkGenericFile
{
public:
  LogSink_debuglog();
  virtual ~LogSink_debuglog(){};
  virtual void addMessage( fmt::Writer* msg ) POL_OVERRIDE;
  virtual void addMessage( fmt::Writer* msg, std::string id ) POL_OVERRIDE;
  void disable();
  static bool Disabled;
};

// leak.log file sink
class LogSink_leaklog : public LogSinkGenericFile
{
public:
  LogSink_leaklog();
  virtual ~LogSink_leaklog(){};
};

class LogSink_flexlog : public LogSink
{
public:
  LogSink_flexlog();
  virtual ~LogSink_flexlog();
  std::string create( std::string logfilename, bool open_timestamp );
  virtual void addMessage( fmt::Writer* msg ) POL_OVERRIDE;
  virtual void addMessage( fmt::Writer* msg, std::string id ) POL_OVERRIDE;
  void close( std::string id );

private:
  std::map<std::string, std::shared_ptr<LogSinkGenericFile>> _logfiles;
};
// template class to perform a dual log eg. cout + pol.log
template <typename log1, typename log2>
class LogSink_dual : public LogSink
{
public:
  LogSink_dual();
  virtual ~LogSink_dual(){};
  virtual void addMessage( fmt::Writer* msg ) POL_OVERRIDE;
  virtual void addMessage( fmt::Writer* msg, std::string id ) POL_OVERRIDE;
};

// main class which starts the logging
class LogFacility : boost::noncopyable
{
public:
  LogFacility();
  ~LogFacility();
  template <typename Sink>
  void save( fmt::Writer* message, std::string id );
  void registerSink( LogSink* sink );
  void disableDebugLog();
  void deinitializeStartLog();
  void closeFlexLog( std::string id );
  std::string registerFlexLogger( std::string logfilename, bool open_timestamp );
  void wait_for_empty_queue();

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
}
}


// several helper defines
//#define DEBUG_LOG_PRINTS
#ifdef DEBUG_LOG_PRINTS
#define LOG_PRINT_CALLER_INFO __FILE__, __LINE__, __FUNCTION__
#define LOG_PRINT_CALLER_INFO2 , __FILE__, __LINE__, __FUNCTION__
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

#define GET_LOG_FILESTAMP Clib::Logging::LogSink::getTimeStamp()
}

#endif  // CLIB_LOGFACILITY_H
