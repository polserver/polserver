
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

#include "logfacility.h"

#include <fstream> 
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory.h>

#ifdef _MSC_VER
    #pragma warning(disable:4996) // disable localtime() unsafe warning. use something else, perhaps? ;)
#endif

namespace Pol {
  namespace Clib {
    bool LogfileTimestampEveryLine = false;
    namespace Logging {

      // helper struct to define log file behaviour
      struct LogFileBehaviour
      {
        std::string basename;
        bool rollover;
        std::ios_base::openmode openmode;
        bool timestamps;
      };

      // definitions of the logfile behaviours
      static LogFileBehaviour startlogBehaviour = {
        "log/start",
        false,
        std::ios_base::out | std::ios_base::trunc,
        false
      };
      static LogFileBehaviour pollogBehaviour = {
        "log/pol",
        true,
        std::ios_base::out | std::ios_base::app,
        true
      };
      static LogFileBehaviour debuglogBehaviour = {
        "log/debug",
        false,
        std::ios_base::out | std::ios_base::app,
        false
      };
      static LogFileBehaviour scriptlogBehaviour = {
        "log/script",
        false,
        std::ios_base::out | std::ios_base::app,
        false
      };
      static LogFileBehaviour leaklogBehaviour = {
        "log/leak",
        false,
        std::ios_base::out | std::ios_base::app,
        false
      };
      static LogFileBehaviour flexlogBehaviour = {
        "", // dummy name
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

      // internal worker class which performs the work in a additional thread
      class LogFacility::LogWorker : boost::noncopyable
      {
        typedef std::function<void()> msg;
        typedef message_queue<msg> msg_queue;
      public:
        // run thread on construction
        LogWorker() : _done( false ), _queue(), _work_thread()
        {
          run();
        }
        // on deconstruction send exit
        ~LogWorker()
        {
          if ( !_done )
          {
            exit();
          }
        }
        // blocks till the queue is empty
        void exit()
        {
          send( [&]() { _done = true; } );
          _work_thread.join(); // wait for it
        }
        // send and move msg into queue
        void move_send( msg &&msg_ )
        {
          _queue.push_move( std::move( msg_ ) );
        }
        // send msg into queue
        void send( msg msg_ )
        {
          _queue.push( msg_ );
        }

      private:
        // endless loop in thread
        void run()
        {
          _work_thread = std::thread( [&]()
          {
            while ( !_done )
            {
              try
              {
                msg func;
                _queue.pop_wait( &func );
                func(); // execute
              }
              catch ( std::exception& msg )
              {
                std::cout << msg.what( ) << std::endl;
              }
            }
          } );
        }
        bool _done;
        msg_queue _queue;
        std::thread _work_thread;
      };


      LogFacility::LogFacility() : _worker( new LogWorker ) {}

      // note this blocks till the worker is finished
      LogFacility::~LogFacility()
      {
        _worker->exit();
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
      void LogFacility::save( std::unique_ptr<fmt::Writer>&& message, std::string id )
      {
        auto moved = makeMoveCopy( std::move( message ) ); // see note above we need to transfer ownership into the lambda
        _worker->move_send( std::move( [moved, id]()
        {
          try
          {
            getSink<Sink>()->sink( moved._movable.get(), id );
          }
          catch ( std::exception& msg )
          {
              std::cout << msg.what() << std::endl;
          }
        } ) );
      }

      // register sink for later deconstruction
      void LogFacility::registerSink( LogSink *sink )
      {
        _registered_sinks.push_back( sink );
      }

      // disables debuglog
      void LogFacility::disableDebugLog()
      {
        _worker->send( []()
        {
          getSink<LogSink_debuglog>()->disable();
        } );
      }

      // disables startlog ( activates pol.log )
      void LogFacility::deinitializeStartLog()
      {
        _worker->send( []()
        {
          getSink<LogSink_pollog>()->deinitialize_startlog();
        } );
      }

      // closes flex sink of given id
      void LogFacility::closeFlexLog( std::string id )
      {
        _worker->send( [id]()
        {
          getSink<LogSink_flexlog>()->close( id );
        } );
      }

      // register new flex sink with given filename
      // blocks to return unique identifier
      std::string LogFacility::registerFlexLogger( std::string logfilename, bool open_timestamp )
      {
        auto promise = std::make_shared<std::promise<std::string>>();
        auto ret = promise->get_future();
        _worker->send( [=, &logfilename]()
        {
          try
          {
            promise->set_value( getSink<LogSink_flexlog>( )->create( logfilename, open_timestamp ) );
          }
          catch ( ... )
          {
            promise->set_exception( std::current_exception() );
          }
        } );
        return ret.get(); // block wait till valid
      }

      // block waits till the queue is empty
      void LogFacility::wait_for_empty_queue()
      {
        auto promise = std::make_shared<std::promise<bool>>( );
        auto ret = promise->get_future( );
        _worker->send( [=]( )
        {
            promise->set_value( true );
        } );
        ret.get( ); // block wait till valid
      }

      // Message default construct
      template <typename Sink>
      Message<Sink>::Message() :_formater( new fmt::Writer() ), _id( "" )
      {}

      template <typename Sink>
      Message<Sink>::Message( const std::string& id ) : _formater( new fmt::Writer( ) ), _id( id )
      {}
      // directly fill on construction the formater with file, line and function
      template <typename Sink>
      Message<Sink>::Message( const std::string &file, const int line, const std::string& function ) : _formater( new fmt::Writer() ), _id( "" )
      {
        *_formater << file << "[" << line << "] " << function << " : ";
      }
      // directly fill on construction the formater with file, line and function
      template <typename Sink>
      Message<Sink>::Message( const std::string& id, const std::string &file, const int line, const std::string& function )
        : _formater( new fmt::Writer() ), _id( id )
      {
        *_formater << file << "[" << line << "] " << function << " : ";
      }
      // on deconstruction transfer the formater to the facility
      template <typename Sink>
      Message<Sink>::~Message()
      {
        if ( _formater->size() > 0 )
          global_logger->save<Sink>( std::move( _formater ), _id );
      }



      // create and get a sink
      template <typename Sink>
      Sink* getSink()
      {
        // does not need to be threadsafe since its only executed inside the worker thread
        // with later vc its automatically threadsafe (magic statics)
        static std::once_flag flag;
        static Sink* sink = new Sink();
        std::call_once( flag, []( Sink* s )
        {
          global_logger->registerSink( s );
        }, sink );
        return sink;
      }

      // base sink construction register self for later deconstruction
      LogSink::LogSink()
      {
        //global_logger->registerSink( this );
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
          fmt::pad( tm_now->tm_mon+1, 2, '0' ) << '/' << fmt::pad( tm_now->tm_mday, 2, '0' ) << ' '
          << fmt::pad( tm_now->tm_hour, 2, '0' ) << ':'
          << fmt::pad( tm_now->tm_min, 2, '0' ) << ':'
          << fmt::pad( tm_now->tm_sec, 2, '0' ) << "] ";
        return tmp.str();
      }

      // first construction also opens the file
      LogSinkGenericFile::LogSinkGenericFile( const LogFileBehaviour* behaviour ) : LogSink(),
		_behaviour( behaviour ), _log_filename( behaviour->basename + ".log" ), _active_line( false )
      {
		memset( &_opened, 0, sizeof( _opened ) );
        open_log_file(true);
      }
      // default constructor does not open directly
      LogSinkGenericFile::LogSinkGenericFile() : LogSink(), _behaviour(), _log_filename(), _active_line( false )
      {
		memset( &_opened, 0, sizeof( _opened ) );
	  }
      LogSinkGenericFile::~LogSinkGenericFile( ) 
      { 
        if ( _filestream.is_open() )
        {
          _filestream.flush();
          _filestream.close( );
        }
      }
      // set behaviour and logfilename, does not work with rollover
      void LogSinkGenericFile::setBehaviour( const LogFileBehaviour* behaviour, std::string filename )
      {
        _behaviour = behaviour;
        _log_filename = filename;
      }

      // open file
      void LogSinkGenericFile::open_log_file( bool open_timestamp )
      {
        _filestream.open( _log_filename, _behaviour->openmode );
        if ( !_filestream.is_open() )
        {
          fmt::Writer tmp;
          tmp << "failed to open logfile " << _log_filename << "\n";
          getSink<LogSink_cerr>()->sink( &tmp );
          return;
        }
        if ( open_timestamp )
        {
          printCurrentTimeStamp( _filestream );
          _filestream << "Logfile opened." << std::endl;
        }
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
      void LogSinkGenericFile::sink( fmt::Writer* msg, std::string )
      {
        sink( msg );
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
          std::string archive_name = _behaviour->basename + "-" + buffer + ".log";
          _filestream.flush();
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
      void LogSink_cout::sink( fmt::Writer* msg)
      {
        std::cout << msg->c_str();
        std::cout.flush();
      }
      void LogSink_cout::sink( fmt::Writer* msg, std::string )
      {
        sink( msg );
      }

      LogSink_cerr::LogSink_cerr() : LogSink()
      {}
      // print given msg into std::cerr
      void LogSink_cerr::sink( fmt::Writer* msg )
      {
        std::cerr << msg->c_str();
        std::cerr.flush();
      }
      void LogSink_cerr::sink( fmt::Writer* msg, std::string )
      {
        sink( msg );
      }

      // on construction this opens not pol.log instead start.log
      LogSink_pollog::LogSink_pollog() : LogSinkGenericFile( &startlogBehaviour )
      {}

      // performs the switch between start.log and pol.log
      void LogSink_pollog::deinitialize_startlog()
      {
        _filestream.flush();
        _filestream.close();
        _behaviour = &pollogBehaviour;
        _log_filename = _behaviour->basename + ".log";
        open_log_file(true);
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
        {
          _filestream.flush();
          _filestream.close();
        }
        Disabled = true;
      }
      // only print the msg if not Disabled
      void LogSink_debuglog::sink( fmt::Writer* msg )
      {
        if ( !Disabled )
          LogSinkGenericFile::sink( msg );
      }
      void LogSink_debuglog::sink( fmt::Writer* msg, std::string )
      {
        sink( msg );
      }

      // on construction opens leak.log
      LogSink_leaklog::LogSink_leaklog() : LogSinkGenericFile( &leaklogBehaviour )
      {}

      LogSink_flexlog::LogSink_flexlog() : LogSink()
      {}
      LogSink_flexlog::~LogSink_flexlog()
      {}

      // create and open new logfile with given name, returns unique id
      std::string LogSink_flexlog::create( std::string logfilename, bool open_timestamp )
      {
        auto itr = _logfiles.find( logfilename );
        if ( itr != _logfiles.end() )
          return logfilename;
        _logfiles[logfilename] = std::make_shared<LogSinkGenericFile>( );
        auto log = _logfiles.at( logfilename );
        log->setBehaviour( &flexlogBehaviour, logfilename );
        log->open_log_file( open_timestamp );
        return logfilename;
      }

      // sink msg into sink of given id
      void LogSink_flexlog::sink( fmt::Writer* msg, std::string id )
      {
        auto itr = _logfiles.find( id );
        if ( itr != _logfiles.end() )
        {
          itr->second->sink( msg );
        }
      }
      void LogSink_flexlog::sink( fmt::Writer* /*msg*/ )
      {
        // empty
      }

      // closes logfile of given id
      void LogSink_flexlog::close( std::string id )
      {
        auto itr = _logfiles.find( id );
        if ( itr != _logfiles.end() )
          _logfiles.erase( itr );
      }

      template <typename log1, typename log2>
      LogSink_dual<log1, log2>::LogSink_dual() : LogSink()
      {}
      // performs the sink with given msg for both sinks
      template <typename log1, typename log2>
      void LogSink_dual<log1, log2>::sink( fmt::Writer* msg)
      {
        getSink<log1>()->sink( msg );
        getSink<log2>()->sink( msg );
      }
      template <typename log1, typename log2>
      void LogSink_dual<log1, log2>::sink( fmt::Writer* msg, std::string )
      {
        sink( msg );
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
  template Pol::Clib::Logging::sink* Pol::Clib::Logging::getSink<Pol::Clib::Logging::sink>( ); \
  template void Pol::Clib::Logging::LogFacility::save<Pol::Clib::Logging::sink>( std::unique_ptr<fmt::Writer>&& message, std::string id );

#define SINK_TEMPLATE_DEFINES_DUAL(sink1, sink2) \
  template class Pol::Clib::Logging::Message<Pol::Clib::Logging::LogSink_dual<Pol::Clib::Logging::sink1, Pol::Clib::Logging::sink2>>; \
  template Pol::Clib::Logging::LogSink_dual<Pol::Clib::Logging::sink1, Pol::Clib::Logging::sink2>* Pol::Clib::Logging::getSink<Pol::Clib::Logging::LogSink_dual<Pol::Clib::Logging::sink1, Pol::Clib::Logging::sink2>>( ); \
  template void Pol::Clib::Logging::LogFacility::save<Pol::Clib::Logging::LogSink_dual<Pol::Clib::Logging::sink1, Pol::Clib::Logging::sink2>>( std::unique_ptr<fmt::Writer>&& message, std::string id );


SINK_TEMPLATE_DEFINES( LogSink_cout )
SINK_TEMPLATE_DEFINES( LogSink_cerr )
SINK_TEMPLATE_DEFINES( LogSink_pollog )
SINK_TEMPLATE_DEFINES( LogSink_scriptlog )
SINK_TEMPLATE_DEFINES( LogSink_debuglog )
SINK_TEMPLATE_DEFINES( LogSink_leaklog )
SINK_TEMPLATE_DEFINES( LogSink_flexlog )

SINK_TEMPLATE_DEFINES_DUAL( LogSink_cout, LogSink_pollog )
SINK_TEMPLATE_DEFINES_DUAL( LogSink_cout, LogSink_scriptlog )
SINK_TEMPLATE_DEFINES_DUAL( LogSink_cout, LogSink_debuglog )
SINK_TEMPLATE_DEFINES_DUAL( LogSink_cout, LogSink_leaklog )

SINK_TEMPLATE_DEFINES_DUAL( LogSink_cerr, LogSink_pollog )
SINK_TEMPLATE_DEFINES_DUAL( LogSink_cerr, LogSink_scriptlog )
SINK_TEMPLATE_DEFINES_DUAL( LogSink_cerr, LogSink_debuglog )
SINK_TEMPLATE_DEFINES_DUAL( LogSink_cerr, LogSink_leaklog )

