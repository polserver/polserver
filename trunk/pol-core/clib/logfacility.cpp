
/*
logging stuff
basic idea is to have an extra thread which performs all the file operations
messages get send on deconstruction the fmt::Writer is used
the sinks are fixed defined which removes a bit the flexibility, but makes the code cleaner
and new sinks can be easily added.

Usage:
POLLOG << "my text\n"
POLLOG.Format("hello {}") << "world";
*/
#include <fstream> 
#include <chrono>
#include <iomanip>

#include "stl_inc.h"
#include "logfacility.h"

#include "logfile.h" // TODO needed for LogfileTimestampEveryLine move it later here

namespace Pol {
  namespace Clib {
    namespace Logging {

      // definitions of the logfile behaviours
      static LogFileBehaviour startlogBehaviour = {
        "log/startdraft",
        false,
        std::ios_base::out | std::ios_base::trunc,
        false
      };
      static LogFileBehaviour pollogBehaviour = {
        "log/poldraft",
        true,
        std::ios_base::out | std::ios_base::app,
        true
      };
      static LogFileBehaviour debuglogBehaviour = {
        "log/debugdraft",
        false,
        std::ios_base::out | std::ios_base::app,
        false
      };
      static LogFileBehaviour scriptlogBehaviour = {
        "log/scriptdraft",
        false,
        std::ios_base::out | std::ios_base::app,
        false
      };
      static LogFileBehaviour leaklogBehaviour = {
        "log/leakdraft",
        false,
        std::ios_base::out | std::ios_base::app,
        false
      };

      // due to a bug in VS a global cannot thread join in the deconstructor
      // thats why this is only a pointer and owned somewhere in main
      LogFacility* global_logger = nullptr;
      void initLogging( LogFacility* logger )
      {
        global_logger = logger;
      }

      LogFacility::LogFacility() : _worker() {}

      // note this blocks till the worker is finished
      LogFacility::~LogFacility()
      {
        _worker.exit();
        for ( auto &sink : _registered_sinks )
          delete sink;
      }

      // evil madness
      // since c++11 cannot capture by move inside a lambda and I need to transfer ownership of the fmt::Writer
      // this performs a move on copy operations wrapper for given type
      // use with care, eg if the whole lambda gets copied madness can happen
      template<typename T>
      struct MoveCopy
      {
        mutable T _movable;

        explicit MoveCopy( T&& value ) : _movable( std::move( value ) ) {}
        MoveCopy( const MoveCopy& other ) : _movable( std::move( other._movable ) ) {}
        MoveCopy( MoveCopy&& other ) : _movable( std::move( other._movable ) ) {}

        MoveCopy& operator=( MoveCopy const& other )
        {
          _movable = std::move( other._movable );
          return *this;
        }
        MoveCopy& operator=( MoveCopy&& other )
        {
          _movable = std::move( other._movable );
          return *this;
        }
      };

      // small helper function to create the wrapper
      template<typename T>
      MoveCopy<T> makeMoveCopy( T&& aValue )
      {
        return MoveCopy<T>( std::move( aValue ) );
      }

      // send logsink as a lambda to the worker
      template <typename Sink>
      void LogFacility::save( std::unique_ptr<fmt::Writer>&& message )
      {
        auto moved = makeMoveCopy( std::move( message ) ); // see note above we need to transfer ownership into the lambda
        _worker.move_send( std::move( [moved]()
        {
          try
          {
            getSink<Sink>()->sink( moved._movable.get() );
          }
          catch ( std::exception& msg )
          {
            cout << msg.what() << endl;
          }
        } ) );
      }

      // register sink for later deconstruction
      void LogFacility::registerSink( LogSink *sink )
      {
        _registered_sinks.push_back( sink );
      }

      // Message default construct
      template <typename Sink>
      Message<Sink>::Message() :_formater( new fmt::Writer() )
      {}
      // directly fill on construction the formater with file, line and function
      template <typename Sink>
      Message<Sink>::Message( const std::string &file, const int line, const std::string& function ) : _formater( new fmt::Writer() )
      {
        *_formater << file << "[" << line << "] " << function << " : ";
      }
      // on deconstruction transfer the formater to the facility
      template <typename Sink>
      Message<Sink>::~Message()
      {
        if ( _formater->size() > 0 )
          global_logger->save<Sink>( std::move( _formater ) );
      }

      namespace Logging_internal {
        // run thread on construction
        LogWorker::LogWorker() : _done( false ), _queue(), _work_thread()
        {
          run();
        }

        // on deconstruction send exit
        LogWorker::~LogWorker()
        {
          if ( !_done )
          {
            exit();
          }
        }
        // blocks till the queue is empty
        void LogWorker::exit()
        {
          send( [&]() { _done = true; } );
          _work_thread.join(); // wait for it
        }
        // send and move msg into queue
        void LogWorker::move_send( msg &&msg_ )
        {
          _queue.push_move( std::move( msg_ ) );
        }
        // send msg into queue
        void LogWorker::send( msg msg_ )
        {
          _queue.push( msg_ );
        }

        // endless loop in thread
        void LogWorker::run()
        {
          _work_thread = std::thread( [&]()
          {
            while ( !_done )
            {
              msg func;
              _queue.pop_wait( &func );
              func(); // execute
            }
          } );
        }
      }

      // create and get a sink
      template <typename Sink>
      LogSink* getSink()
      {
        // does not need to be threadsafe since its only executed inside the worker thread
        // with later vc its automatically threadsafe (magic statics)
        static Sink* sink = new Sink();
        return sink;
      }

      // base sink construction register self for later deconstruction
      LogSink::LogSink()
      {
        global_logger->registerSink( this );
      }

      // helper function to print timestamp into stream
      void LogSink::printCurrentTimeStamp( std::ostream &stream )
      {
        stream << getLoggingTimeStamp();
      }

      // formats and returns current time as std::string
      // put_time is not yet implemented in gcc, and i dont like strftime :)
      std::string LogSink::getLoggingTimeStamp()
      {
        using std::chrono::system_clock;
        time_t now = system_clock::to_time_t( system_clock::now() );
        struct tm* tm_now = localtime( &now );
        fmt::Writer tmp; // write '[%m/%d %H:%M:%S] '
        tmp << '[' <<
          fmt::pad( tm_now->tm_mon, 2, '0' ) << '/' << fmt::pad( tm_now->tm_mday, 2, '0' ) << ' '
          << fmt::pad( tm_now->tm_hour, 2, '0' ) << ':'
          << fmt::pad( tm_now->tm_min, 2, '0' ) << ':'
          << fmt::pad( tm_now->tm_sec, 2, '0' ) << "] ";
        return tmp.str();
      }

      // first construction also opens the file
      LogSinkGenericFile::LogSinkGenericFile( const LogFileBehaviour* behaviour ) : LogSink(),
        _behaviour( behaviour ), _log_filename( behaviour->basename + ".log" ), _active_line( false )
      {
        open_log_file();
      }

      // open file
      void LogSinkGenericFile::open_log_file()
      {
        _filestream.open( _log_filename, _behaviour->openmode );
        if ( !_filestream.is_open() )
        {
          fmt::Writer tmp;
          tmp << "failed to open logfile " << _log_filename << "\n";
          getSink<LogSink_cerr>()->sink( &tmp );
          return;
        }
        printCurrentTimeStamp( _filestream );
        _filestream << "Logfile opened." << endl;

        time_t t_now = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
        _opened = *localtime( &t_now ); // mark current time for later possible rollover
        _active_line = false;
      }

      // print given msg into filestream
      void LogSinkGenericFile::sink( fmt::Writer* msg )
      {
        if ( !_filestream.is_open() )
          return;
        if ( !msg->size() )
          return;

        if ( !_active_line ) // only rollover or add timestamp if there is currently no open line
        {
          using std::chrono::system_clock;
          std::chrono::time_point<system_clock> now = system_clock::now();
          if ( now != _lasttimestamp )
          {
            if ( !test_for_rollover( now ) )
              return;
            if ( _behaviour->timestamps )
              printCurrentTimeStamp( _filestream );
          }
          else if ( _behaviour->timestamps && Clib::LogfileTimestampEveryLine )
            printCurrentTimeStamp( _filestream );
        }
        _active_line = ( msg->data()[msg->size() - 1] != '\n' ); // is the last character a newline?
        _filestream << msg->c_str();
        _filestream.flush();
      }

      // check if a rollover is needed (new day)
      bool LogSinkGenericFile::test_for_rollover( std::chrono::time_point<std::chrono::system_clock>& now )
      {
        time_t t_now = std::chrono::system_clock::to_time_t( now );
        auto tm_now = localtime( &t_now );
        if ( _behaviour->rollover && ( tm_now->tm_mday != _opened.tm_mday ||
          tm_now->tm_mon != _opened.tm_mon ) )
        {
          // roll the log file over
          char buffer[30];
          strftime( buffer, sizeof buffer, "%Y-%m-%d", &_opened );
          string archive_name = _behaviour->basename + "-" + buffer + ".log";
          _filestream.close();

          // whether the rename succeeds or fails, the action is the same.
          rename( _log_filename.c_str(), archive_name.c_str() );

          // moved.  open the new file
          _filestream.open( _log_filename, _behaviour->openmode );
          if ( !_filestream.is_open() )
            return false;
          _opened = *tm_now;
        }
        return true;
      }

      LogSink_cout::LogSink_cout() : LogSink()
      {}
      // print given msg into std::cout
      void LogSink_cout::sink( fmt::Writer* msg )
      {
        cout << msg->c_str();
        cout.flush();
      }
      LogSink_cerr::LogSink_cerr() : LogSink()
      {}
      // print given msg into std::cerr
      void LogSink_cerr::sink( fmt::Writer* msg )
      {
        cerr << msg->c_str();
        cerr.flush();
      }

      // on construction this opens not pol.log instead start.log
      LogSink_pollog::LogSink_pollog() : LogSinkGenericFile( &startlogBehaviour )
      {}

      // performs the switch between start.log and pol.log
      void LogSink_pollog::deinitialize_startlog()
      {
        _filestream.close();
        _behaviour = &pollogBehaviour;
        _log_filename = _behaviour->basename + ".log";
        open_log_file();
      }

      // on construction opens script.log
      LogSink_scriptlog::LogSink_scriptlog() : LogSinkGenericFile( &scriptlogBehaviour )
      {}

      // on construction opens debug.log
      LogSink_debuglog::LogSink_debuglog() : LogSinkGenericFile( &debuglogBehaviour )
      {}

      // debug.log can be disabled
      void LogSink_debuglog::disable()
      {
        if ( _filestream.is_open() )
          _filestream.close();
        Disabled = true;
      }
      // only print the msg if not Disabled
      void LogSink_debuglog::sink( fmt::Writer* msg )
      {
        if ( !Disabled )
          LogSinkGenericFile::sink( msg );
      }

      // on construction opens leak.log
      LogSink_leaklog::LogSink_leaklog() : LogSinkGenericFile( &leaklogBehaviour )
      {}

      template <typename log1, typename log2>
      LogSink_dual<log1, log2>::LogSink_dual() : LogSink()
      {}
      // performs the sink with given msg for both sinks
      template <typename log1, typename log2>
      void LogSink_dual<log1, log2>::sink( fmt::Writer* msg )
      {
        getSink<log1>()->sink( msg );
        getSink<log2>()->sink( msg );
      }

    }
  }
  bool Clib::Logging::LogSink_debuglog::Disabled = false;
}

// forward define the templates
// dont want to add all the templates into the header
// could reduce the compilation time a bit

#define SINK_TEMPLATE_DEFINES(sink) \
  template class Pol::Clib::Logging::Message<Pol::Clib::Logging::sink>; \
  template Pol::Clib::Logging::LogSink* Pol::Clib::Logging::getSink<Pol::Clib::Logging::sink>( ); \
  template void Pol::Clib::Logging::LogFacility::save<Pol::Clib::Logging::sink>( std::unique_ptr<fmt::Writer>&& message );

#define SINK_TEMPLATE_DEFINES_DUAL(sink1, sink2) \
  template class Pol::Clib::Logging::Message<Pol::Clib::Logging::LogSink_dual<Pol::Clib::Logging::sink1, Pol::Clib::Logging::sink2>; \
  template Pol::Clib::Logging::LogSink* Pol::Clib::Logging::getSink<Pol::Clib::Logging::LogSink_dual<Pol::Clib::Logging::sink1, Pol::Clib::Logging::sink2>( ); \
  template void Pol::Clib::Logging::LogFacility::save<Pol::Clib::Logging::LogSink_dual<Pol::Clib::Logging::sink1, Pol::Clib::Logging::sink2>( std::unique_ptr<fmt::Writer>&& message );


SINK_TEMPLATE_DEFINES( LogSink_cout )
SINK_TEMPLATE_DEFINES( LogSink_cerr )
SINK_TEMPLATE_DEFINES( LogSink_pollog )
SINK_TEMPLATE_DEFINES( LogSink_scriptlog )
SINK_TEMPLATE_DEFINES( LogSink_debuglog )
SINK_TEMPLATE_DEFINES( LogSink_leaklog )

SINK_TEMPLATE_DEFINES_DUAL( LogSink_cout, LogSink_pollog> )
SINK_TEMPLATE_DEFINES_DUAL( LogSink_cout, LogSink_scriptlog> )
SINK_TEMPLATE_DEFINES_DUAL( LogSink_cout, LogSink_debuglog> )
SINK_TEMPLATE_DEFINES_DUAL( LogSink_cout, LogSink_leaklog> )

SINK_TEMPLATE_DEFINES_DUAL( LogSink_cerr, LogSink_pollog> )
SINK_TEMPLATE_DEFINES_DUAL( LogSink_cerr, LogSink_scriptlog> )
SINK_TEMPLATE_DEFINES_DUAL( LogSink_cerr, LogSink_debuglog> )
SINK_TEMPLATE_DEFINES_DUAL( LogSink_cerr, LogSink_leaklog> )

