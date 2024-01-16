/** @file
 *
 * @par History
 * - 2005/06/01 Shinigami: added getstatics - to fill a list with statics
 */


#include "staticserver.h"

#include <string>

#include "../clib/binaryfile.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/strutil.h"
#include "../clib/timer.h"
#include "staticblock.h"

namespace Pol
{
namespace Plib
{
StaticServer::StaticServer( const RealmDescriptor& descriptor )
    : _descriptor( descriptor ), _index(), _statics()
{
  Clib::BinaryFile index_file( _descriptor.path( "statidx.dat" ), std::ios::in );
  index_file.ReadVector( _index );
  if ( _index.empty() )
  {
    std::string message = "Empty file: " + _descriptor.path( "statidx.dat" );
    throw std::runtime_error( message );
  }

  Clib::BinaryFile statics_file( _descriptor.path( "statics.dat" ), std::ios::in );
  statics_file.ReadVector( _statics );
  if ( _statics.empty() )
  {
    std::string message = "Empty file: " + _descriptor.path( "statics.dat" );
    throw std::runtime_error( message );
  }

  Validate();
}

void StaticServer::Validate() const
{
  POLLOG_INFO( "Validating statics files: " );
  Tools::Timer<> timer;
  for ( unsigned short y = 0; y < _descriptor.height; y += STATICBLOCK_CHUNK )
  {
    for ( unsigned short x = 0; x < _descriptor.width; x += STATICBLOCK_CHUNK )
    {
      ValidateBlock( x, y );
    }
  }
  POLLOG_INFOLN( "Completed in {} ms.", timer.ellapsed() );
}

void StaticServer::ValidateBlock( unsigned short x, unsigned short y ) const
{
  unsigned short x_block = x / STATICBLOCK_CHUNK;
  unsigned short y_block = y / STATICBLOCK_CHUNK;

  size_t block_index =
      static_cast<size_t>( y_block ) * ( _descriptor.width >> STATICBLOCK_SHIFT ) + x_block;
  if ( block_index + 1 >= _index.size() )
  {
    std::string message =
        "statics integrity error(1): x=" + Clib::tostring( x ) + ", y=" + Clib::tostring( y );
    throw std::runtime_error( message );
  }
  unsigned int first_entry_index = _index[block_index].index;
  unsigned int num = _index[block_index + 1].index - first_entry_index;
  if ( first_entry_index + num > _statics.size() )
  {
    std::string message =
        "statics integrity error(2): x=" + Clib::tostring( x ) + ", y=" + Clib::tostring( y );
    throw std::runtime_error( message );
  }
}

bool StaticServer::findstatic( unsigned short x, unsigned short y, unsigned short objtype ) const
{
  passert( x < _descriptor.width && y < _descriptor.height );

  unsigned short x_block = x >> STATICBLOCK_SHIFT;
  unsigned short y_block = y >> STATICBLOCK_SHIFT;
  unsigned short xy = ( ( x & STATICCELL_MASK ) << 4 ) | ( y & STATICCELL_MASK );

  unsigned int block_index = x_block + y_block * ( _descriptor.width >> STATICBLOCK_SHIFT );
  unsigned int first_entry_index = _index[block_index].index;
  unsigned int num = _index[block_index + 1].index - first_entry_index;

  if ( num )
  {
    const STATIC_ENTRY* entry = &_statics[first_entry_index];
    while ( num-- )
    {
      if ( entry->xy == xy && entry->objtype == objtype )
      {
        return true;
      }
      ++entry;
    }
  }
  return false;
}

void StaticServer::getstatics( StaticEntryList& statics, unsigned short x, unsigned short y ) const
{
  passert( x < _descriptor.width && y < _descriptor.height );

  unsigned short x_block = x >> STATICBLOCK_SHIFT;
  unsigned short y_block = y >> STATICBLOCK_SHIFT;
  unsigned short xy = ( ( x & STATICCELL_MASK ) << 4 ) | ( y & STATICCELL_MASK );

  unsigned int block_index = x_block + y_block * ( _descriptor.width >> STATICBLOCK_SHIFT );
  unsigned int first_entry_index = _index[block_index].index;
  unsigned int num = _index[block_index + 1].index - first_entry_index;

  if ( num )
  {
    const STATIC_ENTRY* entry = &_statics[first_entry_index];
    while ( num-- )
    {
      if ( entry->xy == xy )
      {
        statics.push_back( *entry );
      }
      ++entry;
    }
  }
}

size_t StaticServer::sizeEstimate() const
{
  size_t size = sizeof( *this ) + _descriptor.sizeEstimate();
  size += 3 * sizeof( STATIC_INDEX* ) + _index.capacity() * sizeof( STATIC_INDEX );
  size += 3 * sizeof( STATIC_ENTRY* ) + _statics.capacity() * sizeof( STATIC_ENTRY );
  return size;
}
}
}
