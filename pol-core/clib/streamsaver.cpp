#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>

#include "streamsaver.h"

namespace Pol::Clib
{
StreamWriter::StreamWriter( const std::string& path ) : _file( fopen( path.c_str(), "wb+" ) )
{
  if ( !_file )
    throw std::runtime_error{ fmt::format( "failed to open {}", path ) };
  setbuf( _file, nullptr );  // disable buffer
}

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

void StreamWriter::flush_close()
{
  if ( !_file )
    return;
  if ( _mbuff.size() )
  {
    auto size = fwrite( _mbuff.data(), sizeof( char ), _mbuff.size(), _file );
    if ( size < _mbuff.size() )
      throw std::runtime_error{ "failed to write" };
  }
  _mbuff.clear();
  fclose( _file );
  _file = nullptr;
}

}  // namespace Pol::Clib
