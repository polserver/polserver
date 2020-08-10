#include "InputBuffer.h"

#include <fstream>

namespace Pol::Bscript::Compiler
{
InputBuffer InputBuffer::read( const std::string& pathname )
{
  std::ifstream input( pathname.c_str(), std::ios::binary );
  std::vector<unsigned char> buffer( std::istreambuf_iterator<char>( input ), {} );

  std::vector<std::byte> bytes;
  bytes.insert( bytes.begin(), reinterpret_cast<const std::byte*>( buffer.data() ),
                reinterpret_cast<const std::byte*>( buffer.data() + buffer.size() ) );
  return InputBuffer( bytes );
}

InputBuffer InputBuffer::read_buffer( unsigned length )
{
  unsigned buffer_start_offset = offset();
  std::vector<std::byte> b;
  b.insert( b.begin(), buffer.begin() + position, buffer.begin() + position + length );
  position += length;
  return InputBuffer( b, buffer_start_offset );
}

}  // namespace Pol::Bscript::Compiler
