#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "streamsaver.h"

namespace Pol
{
namespace Clib
{
const std::size_t flush_limit = 10000;  // 500;

void StreamWriter::flush_test()
{
  if ( _buf.size() >= flush_limit )  // guard against to big objects
    flush();
}

/// ofstream implementation (simple non threaded)
OFStreamWriter::OFStreamWriter()
    : StreamWriter(),
      _stream(),
#if 0
      _fs_time( 0 ),
#endif
      _stream_name()
{
}

OFStreamWriter::OFStreamWriter( std::ofstream* stream )
    : StreamWriter(),
      _stream( stream ),
#if 0
      _fs_time( 0 ),
#endif
      _stream_name()
{
}

OFStreamWriter::~OFStreamWriter()
{
#if 0
  if ( !_buf.empty() )
  {
    Tools::HighPerfTimer t;
    *_stream << _buf;
    _fs_time += t.ellapsed();
  }
  ERROR_PRINTLN( "streamwriter {} io time {}", _stream_name, _fs_time.count() );
#else
  if ( !_buf.empty() )
    *_stream << _buf;
#endif
}

void OFStreamWriter::init( const std::string& filepath )
{
  _stream->exceptions( std::ios_base::failbit | std::ios_base::badbit );
  _stream->open( filepath.c_str(), std::ios::out | std::ios::trunc );
  _stream_name = filepath;
}

void OFStreamWriter::flush()
{
#if 0
      Tools::HighPerfTimer t;
#endif
  if ( !_buf.empty() )
  {
    *_stream << _buf;
    _buf.clear();
  }
#if 0
      _fs_time += t.ellapsed( );
#endif
}

void OFStreamWriter::flush_file()
{
  flush();
  _stream->flush();
}

/// ostream implementation (non threaded)
OStreamWriter::OStreamWriter() : StreamWriter(), _stream() {}

OStreamWriter::OStreamWriter( std::ostream* stream ) : StreamWriter(), _stream( stream ) {}

OStreamWriter::~OStreamWriter()
{
  if ( !_buf.empty() )
    *_stream << _buf;
}

void OStreamWriter::init( const std::string& ) {}

void OStreamWriter::flush()
{
  if ( !_buf.empty() )
  {
    *_stream << _buf;
    _buf.clear();
  }
}

void OStreamWriter::flush_file()
{
  flush();
  _stream->flush();
}

/// ofstream implementation with worker thread for file io
ThreadedOFStreamWriter::ThreadedOFStreamWriter()
    : StreamWriter(), _stream(), _msg_queue(), _writethread(), _writers_hold(), _stream_name()
{
  start_worker();
}

ThreadedOFStreamWriter::ThreadedOFStreamWriter( std::ofstream* stream )
    : StreamWriter(),
      _stream( stream ),
      _msg_queue(),
      _writethread(),
      _writers_hold(),
      _stream_name()
{
  start_worker();
}
void ThreadedOFStreamWriter::start_worker()
{
  _writethread = std::thread(
      [this]()
      {
        std::list<std::string> writers;
        // small helper lambda to write into stream
        auto _write_to_stream = [&]( std::list<std::string>& l )
        {
          for ( const auto& _w : l )
          {
            if ( !_w.empty() )
              *_stream << _w;
          }
        };
        try
        {
          for ( ;; )
          {
            writers.clear();
            _msg_queue.pop_wait( &writers );
            _write_to_stream( writers );
          }
        }
        catch ( writer_queue::Canceled& )
        {
        }
        writers.clear();
        _msg_queue.pop_remaining( &writers );
        _write_to_stream( writers );
        _stream->flush();
      } );
}

ThreadedOFStreamWriter::~ThreadedOFStreamWriter()
{
  flush_file();
}

void ThreadedOFStreamWriter::init( const std::string& filepath )
{
  _stream->exceptions( std::ios_base::failbit | std::ios_base::badbit );
  _stream->open( filepath.c_str(), std::ios::out );
  _stream_name = filepath;
}

void ThreadedOFStreamWriter::flush()
{
  if ( _buf.empty() )
    return;
  _writers_hold.emplace_back( std::move( _buf ) );
  if ( _writers_hold.size() > 10 )
  {
    _msg_queue.push( _writers_hold );
  }
  _buf.clear();
}

void ThreadedOFStreamWriter::flush_file()
{
  if ( _writethread.joinable() )
  {
    flush();
    if ( !_writers_hold.empty() )
      _msg_queue.push( _writers_hold );

    _msg_queue.cancel();
    _writethread.join();
    _stream->close();
  }
}
}  // namespace Clib
}  // namespace Pol
