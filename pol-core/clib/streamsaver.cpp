#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>

#include "clib/streamsaver.h"

namespace Pol::Clib
{
StreamWriter::StreamWriter( const std::string& path ) : _file( fopen( path.c_str(), "wb+" ) )
{
  if ( !_file )
    throw std::runtime_error{ fmt::format( "failed to open {}", path ) };
  setbuf( _file, nullptr );  // disable buffer
  // Every flush check fires above the threshold, so this is where the buffer ends up regardless.
  // Taking it in one allocation instead of growing into it saves a handful of reallocate-and-copy
  // rounds per file. A detached writer gets no reserve: it never flushes, and grows to whatever
  // one chunk needs.
  _mbuff.reserve( FLUSH_THRESHOLD );
}

StreamWriter::StreamWriter() : _file( nullptr ) {}

StreamWriter::~StreamWriter() noexcept( false )
{
  auto stack_unwinding = std::uncaught_exceptions();
  try
  {
    flush_close();
  }
  catch ( ... )
  {
    // during stack unwinding an exception would terminate
    if ( !stack_unwinding )
      throw;
  }
}

void StreamWriter::write_block( std::string_view text )
{
  if ( text.empty() )
    return;
  auto size = fwrite( text.data(), sizeof( char ), text.size(), _file );
  if ( size < text.size() )
    throw std::runtime_error{ "failed to write" };
  _bytes_written += text.size();
}

void StreamWriter::flush()
{
  write_block( { _mbuff.data(), _mbuff.size() } );
  _mbuff.clear();
}

void StreamWriter::flush_close()
{
  if ( !_file )
    return;
  if ( _mbuff.size() )
    flush();
  fclose( _file );
  _file = nullptr;
}

}  // namespace Pol::Clib
