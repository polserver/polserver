
#ifndef LOGFACILITY_H
#define LOGFACILITY_H

#include <map>
#include <thread>
#include <future>
#include <memory>
#include <fstream>
#include <boost/noncopyable.hpp>
#include "../../lib/format/format.h"

#include "message_queue.h"

namespace Pol {
  namespace Core {
    struct PolConfig;
  }
  namespace Clib {
    extern bool LogfileTimestampEveryLine;

    namespace Logging {
      struct LogFileBehaviour;

      // the base class for the differrent sinks
      class LogSink : boost::noncopyable
      {
      public:
        LogSink();
        virtual ~LogSink() {};
        virtual void sink( fmt::Writer* msg, unsigned int id = 0 ) = 0;
        virtual void close( unsigned int ) {};
        static void printCurrentTimeStamp( std::ostream &stream );
        static std::string getLoggingTimeStamp();
      };

      

      // generic sink to log into a file
      class LogSinkGenericFile : public LogSink
      {
      public:
        LogSinkGenericFile( const LogFileBehaviour* behaviour );
        LogSinkGenericFile();
        ~LogSinkGenericFile();
        void open_log_file();
        void setBehaviour( const LogFileBehaviour* behaviour, std::string filename );
        void sink( fmt::Writer* msg, unsigned int id = 0 );
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
        ~LogSink_cout() {};
        void sink( fmt::Writer* msg, unsigned int id = 0 );
      };

      // std::cerr sink
      class LogSink_cerr : public LogSink
      {
      public:
        LogSink_cerr();
        ~LogSink_cerr() {};
        void sink( fmt::Writer* msg, unsigned int id = 0 );
      };

      // pol.log (and start.log) file sink
      class LogSink_pollog : public LogSinkGenericFile
      {
      public:
        LogSink_pollog();
        ~LogSink_pollog() {};
        void deinitialize_startlog();
      };

      // script.log file sink
      class LogSink_scriptlog : public LogSinkGenericFile
      {
      public:
        LogSink_scriptlog();
        ~LogSink_scriptlog() {};
      };

      // debug.log file sink
      class LogSink_debuglog : public LogSinkGenericFile
      {
      public:
        LogSink_debuglog();
        ~LogSink_debuglog() {};
        void sink( fmt::Writer* msg, unsigned int id = 0 );
        void disable();
        static bool Disabled;
      };

      // leak.log file sink
      class LogSink_leaklog : public LogSinkGenericFile
      {
      public:
        LogSink_leaklog();
        ~LogSink_leaklog() {};
      };

      class LogSink_flexlog : public LogSink
      {
      public:
        LogSink_flexlog();
        ~LogSink_flexlog();
        unsigned int create( std::string logfilename );
        void sink( fmt::Writer* msg, unsigned int id );
        void close( unsigned int id );
      private:
        std::map<unsigned int, std::shared_ptr<LogSinkGenericFile>> _logfiles;
        unsigned int _idcounter;
      };
      // template class to perform a dual log eg. cout + pol.log
      template <typename log1, typename log2>
      class LogSink_dual : public LogSink
      {
      public:
        LogSink_dual();
        ~LogSink_dual() {};
        void sink( fmt::Writer* msg, unsigned int id = 0 );
      };

      // main class which starts the logging
      class LogFacility : boost::noncopyable
      {
      public:
        LogFacility();
        ~LogFacility();
        template <typename Sink>
        void save( std::unique_ptr<fmt::Writer>&& message, unsigned int id );
        void registerSink( LogSink* sink );
        void disableDebugLog();
        void deinitializeStartLog();
        void closeFlexLog( unsigned int id );
        unsigned int registerFlexLogger( std::string logfilename );
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
        Message( unsigned int id );
        Message( const std::string &file, const int line, const std::string& function ); // for internal stuff with __FILE__, __LINE__, __FUNCTION__
        Message( unsigned int id, const std::string &file, const int line, const std::string& function ); // for internal stuff with __FILE__, __LINE__, __FUNCTION__
        ~Message(); // auto flush

        fmt::Writer& message() { return *( _formater.get() ); }

      private:
        std::unique_ptr<fmt::Writer> _formater;
        unsigned int _id;
      };


      extern LogFacility* global_logger; // pointer to the instance of the main class
      void initLogging( LogFacility* logger ); // initalize the logging
    }
  }

  
  // several helper defines
//#define DEBUG_LOG_PRINTS
#ifdef DEBUG_LOG_PRINTS
#define LOG_PRINT_CALLER_INFO ,__FILE__, __LINE__, __FUNCTION__
#else
#define LOG_PRINT_CALLER_INFO
#endif

  // log into pol.log and std::cerr
#define POLLOG_ERROR Clib::Logging::Message<Clib::Logging::LogSink_dual<Clib::Logging::LogSink_cerr, Clib::Logging::LogSink_pollog>>(0 LOG_PRINT_CALLER_INFO).message()
  // log into pol.log and std::cout
#define POLLOG_INFO Clib::Logging::Message<Clib::Logging::LogSink_dual<Clib::Logging::LogSink_cout, Clib::Logging::LogSink_pollog>>(0 LOG_PRINT_CALLER_INFO).message()
  // log into pol.log
#define POLLOG Clib::Logging::Message<Clib::Logging::LogSink_pollog>(0 LOG_PRINT_CALLER_INFO).message()

  // log only into std::cout
#define INFO_PRINT Clib::Logging::Message<Clib::Logging::LogSink_cout>(0 LOG_PRINT_CALLER_INFO).message()
  // log only into std::cout if level is equal or higher
#define INFO_PRINT_TRACE(n) if ( Core::config.debug_level >= n ) INFO_PRINT
  // log only into std::cerr
#define ERROR_PRINT Clib::Logging::Message<Clib::Logging::LogSink_cerr>(0 LOG_PRINT_CALLER_INFO).message()

  // log into script.log
#define SCRIPTLOG Clib::Logging::Message<Clib::Logging::LogSink_scriptlog>(0 LOG_PRINT_CALLER_INFO).message()
  // log into debug.log (if enabled)
#define DEBUGLOG if (!Clib::Logging::LogSink_debuglog::Disabled) Clib::Logging::Message<Clib::Logging::LogSink_debuglog>(0 LOG_PRINT_CALLER_INFO).message()
  // log into leak.log
#define LEAKLOG Clib::Logging::Message<Clib::Logging::LogSink_leaklog>(0 LOG_PRINT_CALLER_INFO).message()

  // log into sink id need a call of OPEN_LOG before
#define FLEXLOG(id) Clib::Logging::Message<Clib::Logging::LogSink_flexlog>(id LOG_PRINT_CALLER_INFO).message()
  // open logfile of given filename, returns unique unsigned int for usage of logging/closing
#define OPEN_FLEXLOG(filename) Clib::Logging::global_logger->registerFlexLogger(filename)
  // close logfile of given id
#define CLOSE_FLEXLOG(id) Clib::Logging::global_logger->closeFlexLog(id)

  // performs the switch between start.log and pol.log
#define DEINIT_STARTLOG Clib::Logging::global_logger->deinitializeStartLog  

  // disables the debug.log
#define DISABLE_DEBUGLOG Clib::Logging::global_logger->disableDebugLog
  // helper bool if the debug.log is disabled (for more complex logging) DEBUGLOG checks also
#define IS_DEBUGLOG_DISABLED Clib::Logging::LogSink_debuglog::Disabled

#define GET_LOG_FILESTAMP Clib::Logging::LogSink::getLoggingTimeStamp()

}

#endif
