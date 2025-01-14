#include <exception>
#include <fstream>
#include <iostream>
#include <string>

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

StreamWriter::StreamWriter( std::ofstream* stream )
    : _stream( stream ),
#if 0
      _fs_time( 0 ),
#endif
      _stream_name()
{
}

StreamWriter::~StreamWriter() noexcept( false )
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
  auto stack_unwinding = std::uncaught_exceptions();
  try
  {
    if ( !_buf.empty() && _stream )
      *_stream << _buf;
  }
  catch ( ... )
  {
    // during stack unwinding an exception would terminate
    if ( !stack_unwinding )
      throw;
  }
#endif
}

void StreamWriter::init( const std::string& filepath )
{
  if ( _stream )
  {
    _stream->exceptions( std::ios_base::failbit | std::ios_base::badbit );
    _stream->open( filepath.c_str(), std::ios::out | std::ios::trunc );
  }
  _stream_name = filepath;
}

void StreamWriter::flush()
{
#if 0
      Tools::HighPerfTimer t;
#endif
  if ( !_buf.empty() && _stream )
  {
    *_stream << _buf;
    _buf.clear();
  }
#if 0
      _fs_time += t.ellapsed( );
#endif
}

void StreamWriter::flush_file()
{
  flush();
  if ( _stream )
    _stream->flush();
}

}  // namespace Clib
}  // namespace Pol
