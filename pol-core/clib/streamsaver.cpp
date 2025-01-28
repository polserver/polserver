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
  flush();
}

void StreamWriter::open_fstream( const std::string& filepath, std::ofstream& s )
{
  s.exceptions( std::ios_base::failbit | std::ios_base::badbit );
  s.open( filepath, std::ios::out | std::ios::trunc );
}

void StreamWriter::flush()
{
  if ( _mbuff.size() )
    _stream << std::string_view{ _mbuff.data(), _mbuff.size() };
  _mbuff.clear();
  _stream.flush();
}

}  // namespace Pol::Clib
