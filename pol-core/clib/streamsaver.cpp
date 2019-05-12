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

/// BaseClass implements only writer operator logic
StreamWriter::StreamWriter() : _writer( new fmt::Writer ) {}

fmt::Writer& StreamWriter::operator()()
{
  if ( _writer->size() >= flush_limit )  // guard against to big objects
  {
    this->flush();
  }
  return *( _writer.get() );
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
      if ( _writer->size() )
      {
        Tools::HighPerfTimer t;
        *_stream << _writer->c_str();
        _fs_time += t.ellapsed();
      }
      ERROR_PRINT << "streamwriter " << _stream_name << " io time " << _fs_time.count( ) << "\n";
#else
  if ( _writer->size() )
    *_stream << _writer->str();
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
  if ( _writer->size() )
  {
    *_stream << _writer->str();
    _writer->Clear();
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
  if ( _writer->size() )
    *_stream << _writer->str();
}

void OStreamWriter::init( const std::string& ) {}

void OStreamWriter::flush()
{
  if ( _writer->size() )
  {
    *_stream << _writer->str();
    _writer.reset( new fmt::Writer );
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
  _writethread = std::thread( [this]() {
    std::list<WriterPtr> writers;
    // small helper lambda to write into stream
    auto _write_to_stream = [&]( std::list<WriterPtr>& l ) {
      for ( const auto& _w : l )
      {
        if ( _w->size() )
          *_stream << _w->str();
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
  if ( _writer->size() )
  {
    _writers_hold.emplace_back( std::move( _writer ) );
    if ( _writers_hold.size() > 10 )
    {
      _msg_queue.push( _writers_hold );
    }
    _writer.reset( new fmt::Writer );
  }
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
