
#ifndef LOGFACILITY_H
#define LOGFACILITY_H

#include <thread>
#include <future>
#include <memory>
#include <fstream>
#include <boost/noncopyable.hpp>
#include "../../lib/format/format.h"

#include "message_queue.h"

namespace Pol {
  namespace Clib {
    namespace Logging {

      namespace Logging_internal {
        // internal worker class which performs the work in a additional thread
        class LogWorker : boost::noncopyable
        {
          typedef std::function<void()> msg;
          typedef message_queue<msg> msg_queue;
        public:
          LogWorker();
          ~LogWorker();
          void exit();
          void send( msg msg_ );
          void move_send( msg &&msg_ );

        private:
          void run();
          bool _done;
          msg_queue _queue;
          std::thread _work_thread;
        };
      }

      // the base class for the differrent sinks
      class LogSink : boost::noncopyable
      {
      public:
        LogSink();
        virtual ~LogSink() {};
        virtual void sink( fmt::Writer* msg ) = 0;
        static void printCurrentTimeStamp( std::ostream &stream );
        static std::string getLoggingTimeStamp();
      };

      // helper struct to define log file behaviour
      struct LogFileBehaviour
      {
        std::string basename;
        bool rollover;
        std::ios_base::openmode openmode;
        bool timestamps;
      };

      // generic sink to log into a file
      class LogSinkGenericFile : public LogSink
      {
      public:
        LogSinkGenericFile( const LogFileBehaviour* behaviour );
        ~LogSinkGenericFile() {};
        void open_log_file();
        void sink( fmt::Writer* msg );
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
      LogSink* getSink();

      // std::cout sink
      class LogSink_cout : public LogSink
      {
      public:
        LogSink_cout();
        ~LogSink_cout() {};
        void sink( fmt::Writer* msg );
      };

      // std::cerr sink
      class LogSink_cerr : public LogSink
      {
      public:
        LogSink_cerr();
        ~LogSink_cerr() {};
        void sink( fmt::Writer* msg );
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
        void sink( fmt::Writer* msg );
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

      // template class to perform a dual log eg. cout + pol.log
      template <typename log1, typename log2>
      class LogSink_dual : public LogSink
      {
      public:
        LogSink_dual();
        ~LogSink_dual() {};
        void sink( fmt::Writer* msg );
      };

      // main class which starts the logging
      class LogFacility : boost::noncopyable
      {
      public:
        LogFacility();
        ~LogFacility();
        template <typename Sink>
        void save( std::unique_ptr<fmt::Writer>&& message );
        void registerSink( LogSink* sink );
      private:
        Logging_internal::LogWorker _worker;
        std::vector<LogSink*> _registered_sinks;
      };

      // construct a message for given sink, on deconstruction sends the msg to the facility
      template <typename Sink>
      class Message
      {
      public:
        Message();
        Message( const std::string &file, const int line, const std::string& function ); // for internal stuff with __FILE__, __LINE__, __FUNCTION__
        ~Message(); // auto flush

        fmt::Writer& message() { return *(_formater.get()); }

      private:
        std::unique_ptr<fmt::Writer> _formater;
      };


      extern LogFacility* global_logger; // pointer to the instance of the main class
      void initLogging( LogFacility* logger ); // initalize the logging
    }
  }


  // several helper defines
//#define DEBUG_LOG_PRINTS
#ifdef DEBUG_LOG_PRINTS
#define LOG_PRINT_CALLER_INFO __FILE__, __LINE__, __FUNCTION__
#else
#define LOG_PRINT_CALLER_INFO
#endif

  // log into pol.log and std::cerr
#define POLLOG_ERROR Clib::Logging::Message<Clib::Logging::LogSink_dual<Clib::Logging::LogSink_cerr, Clib::Logging::LogSink_pollog>>(LOG_PRINT_CALLER_INFO).message()
  // log into pol.log and std::cout
#define POLLOG_INFO Clib::Logging::Message<Clib::Logging::LogSink_dual<Clib::Logging::LogSink_cout, Clib::Logging::LogSink_pollog>>(LOG_PRINT_CALLER_INFO).message()
  // log into pol.log
#define POLLOG Clib::Logging::Message<Clib::Logging::LogSink_pollog>(LOG_PRINT_CALLER_INFO).message()

  // log only into std::cout
#define INFO_PRINT Clib::Logging::Message<Clib::Logging::LogSink_cout>(LOG_PRINT_CALLER_INFO).message()
  // log only into std::cerr
#define ERROR_PRINT Clib::Logging::Message<Clib::Logging::LogSink_cerr>(LOG_PRINT_CALLER_INFO).message()

  // log into script.log
#define SCRIPTLOG Clib::Logging::Message<Clib::Logging::LogSink_scriptlog>(LOG_PRINT_CALLER_INFO).message()
  // log into debug.log (if enabled)
#define DEBUGLOG if (!Clib::Logging::LogSink_debuglog::Disabled) Clib::Logging::Message<Clib::Logging::LogSink_debuglog>(LOG_PRINT_CALLER_INFO).message()
  // log into leak.log
#define LEAKLOG Clib::Logging::Message<Clib::Logging::LogSink_leaklog>(LOG_PRINT_CALLER_INFO).message()

  // performs the switch between start.log and pol.log
#define DEINIT_STARTLOG static_cast<Clib::Logging::LogSink_pollog*>(Clib::Logging::getSink<Clib::Logging::LogSink_pollog>())->deinitialize_startlog

  // disables the debug.log
#define DISABLE_DEBUGLOG static_cast<Clib::Logging::LogSink_debuglog*>(Clib::Logging::getSink<Clib::Logging::LogSink_debuglog>())->disable
  // helper bool if the debug.log is disabled (for more complex logging) DEBUGLOG checks also
#define IS_DEBUGLOG_DISABLED Clib::Logging::LogSink_debuglog::Disabled

}

#endif
