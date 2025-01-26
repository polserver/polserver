#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>

#include "streamsaver.h"

namespace Pol::Clib
{
StreamWriter::StreamWriter( std::ostream& stream ) : _stream( stream ) {}

StreamWriter::~StreamWriter() noexcept( false )
{
  _stream.flush();
}

void StreamWriter::open_fstream( const std::string& filepath, std::ofstream& s )
{
  _buf.reset( new char[1024 * 1024] );
  s.exceptions( std::ios_base::failbit | std::ios_base::badbit );
  s.open( filepath, std::ios::out | std::ios::trunc );
  s.rdbuf()->pubsetbuf( _buf.get(), 1024 * 1024 );
}

void StreamWriter::flush_file()
{
  _stream.flush();
}

}  // namespace Pol::Clib
