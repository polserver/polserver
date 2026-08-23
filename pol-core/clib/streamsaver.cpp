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

void StreamWriter::write_block( const std::string_view& text )
{
  if ( text.empty() )
    return;
  auto size = fwrite( text.data(), sizeof( char ), text.size(), _file );
  if ( size < text.size() )
    throw std::runtime_error{ "failed to write" };
  _bytes_written += text.size();
  ++_flush_count;
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
