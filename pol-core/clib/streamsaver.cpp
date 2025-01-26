#include <exception>
#include <fstream>
#include <iostream>
#include <string>

#include "streamsaver.h"

namespace Pol
{
namespace Clib
{

StreamWriter::StreamWriter( std::ofstream* stream ) : _stream( stream )
#if 0
      _fs_time( 0 ),
#endif
{
}

void StreamWriter::init( const std::string& filepath )
{
  _stream->exceptions( std::ios_base::failbit | std::ios_base::badbit );
  _stream->open( filepath.c_str(), std::ios::out | std::ios::trunc );
}

void StreamWriter::flush_file()
{
  _stream->flush();
}

}  // namespace Clib
}  // namespace Pol
