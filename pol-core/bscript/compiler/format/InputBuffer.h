#ifndef POLSERVER_INPUTBUFFER_H
#define POLSERVER_INPUTBUFFER_H

#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class InputBuffer
{
public:
  InputBuffer( std::vector<std::byte> contents, unsigned start_offset = 0 )
      : start_offset( start_offset ), position( 0 ), buffer( std::move( contents ) )
  {
  }

  unsigned offset() const { return start_offset + position; }

  bool more() const { return position < buffer.size(); }

  static InputBuffer read( const std::string& pathname );

  template <typename T>
  T read()
  {
    unsigned end = position + sizeof( T );
    if ( end > buffer.size() )
      throw std::runtime_error( "buffer underflow" );

    T result{};
    std::memcpy( &result, &( *buffer.begin() ) + position, sizeof( T ) );
    position += sizeof( T );
    return result;
  }

  InputBuffer read_buffer( unsigned length );

  std::vector<std::byte> get_buffer() const { return buffer; }

private:
  unsigned start_offset = 0;
  unsigned position = 0;
  std::vector<std::byte> buffer;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_INPUTBUFFER_H
