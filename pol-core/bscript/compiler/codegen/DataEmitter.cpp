#include "DataEmitter.h"

#include <algorithm>
#include <stdexcept>
#include <limits>

namespace Pol::Bscript::Compiler
{
DataEmitter::DataEmitter( DataSection& data_section ) : data_section( data_section ) {}

unsigned DataEmitter::append( double value )
{
  // The old compiler always appends doubles.  For parity,
  // we'll do the same here.
  return append( reinterpret_cast<const std::byte*>( &value ), sizeof value );
}

unsigned DataEmitter::append( int value )
{
  // The old compiler always appends integers.  For parity,
  // we'll do the same here.
  return append( reinterpret_cast<const std::byte*>( &value ), sizeof value );
}

unsigned DataEmitter::store( const std::string& s )
{
  return store( reinterpret_cast<const std::byte*>( s.c_str() ),
                static_cast<unsigned>( s.length() + 1 ) );
}

unsigned DataEmitter::store( const std::byte* data, size_t len )
{
  if ( auto existing = find( data, len ) )
    return existing;
  else
    return append( data, len );
}

unsigned DataEmitter::append( const std::byte* data, size_t len )
{
  size_t position = data_section.size();
  if ( position + len > std::numeric_limits<unsigned>::max() ) {
    throw std::runtime_error( "Data offset overflow" );
  }
  data_section.insert( data_section.end(), data, data + len );

  return static_cast<unsigned>( position );
}

unsigned DataEmitter::find( const std::byte* data, size_t len )
{
  if ( data_section.empty() )
    return 0;

  auto start = data_section.begin();
  ++start;  // offset=0 means no data, not data at offset 0
  auto itr = std::search( start, data_section.end(), data, data + len );
  if ( itr != data_section.end() )
    return static_cast<unsigned>( itr - data_section.begin() );
  return 0;
}

}  // namespace Pol::Bscript::Compiler
