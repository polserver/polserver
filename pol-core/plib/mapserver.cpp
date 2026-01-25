/** @file
 *
 * @par History
 */


#include "mapserver.h"

#include <stddef.h>
#include <stdexcept>
#include <string>

#include "../clib/binaryfile.h"
#include "../clib/passert.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "filemapserver.h"
#include "inmemorymapserver.h"
#include "mapcell.h"
#include "mapshape.h"
#include "mapsolid.h"


namespace Pol::Plib
{
MapServer::MapServer( const RealmDescriptor& descriptor ) : _descriptor( descriptor )
{
  LoadSolids();

  // the first-level index points into the second-level index, so load the second-level index first.
  LoadSecondLevelIndex();

  LoadFirstLevelIndex();
}

void MapServer::LoadSolids()
{
  std::string filename = _descriptor.path( "solids.dat" );

  Clib::BinaryFile infile( filename, std::ios::in );
  infile.ReadVector( _shapedata );
}

void MapServer::LoadSecondLevelIndex()
{
  std::string filename = _descriptor.path( "solidx2.dat" );

  Clib::BinaryFile infile( filename, std::ios::in );
  std::fstream::pos_type filesize = infile.FileSize();
  if ( filesize < std::fstream::pos_type( SOLIDX2_FILLER_SIZE ) )
    throw std::runtime_error( filename + " must have size of at least " +
                              Clib::tostring( SOLIDX2_FILLER_SIZE ) + " bytes." );

  std::fstream::pos_type databytes = filesize - std::fstream::pos_type( SOLIDX2_FILLER_SIZE );
  if ( ( databytes % sizeof( SOLIDX2_ELEM ) ) != 0 )
    throw std::runtime_error( filename + " does not contain an integral number of elements." );

  size_t count = static_cast<size_t>( databytes / sizeof( SOLIDX2_ELEM ) );
  _index2.resize( count );
  infile.Seek( SOLIDX2_FILLER_SIZE );
  infile.Read( &_index2[0], count );

  // integrity check
  for ( const auto& elem : _index2 )
  {
    passert( elem.baseindex < _shapedata.size() );

    for ( auto x : elem.addindex )
    {
      for ( unsigned y = 0; y < SOLIDX_Y_SIZE; ++y )
      {
        size_t idx = elem.baseindex + x[y];
        passert( idx < _shapedata.size() );
      }
    }
  }
}

void MapServer::LoadFirstLevelIndex()
{
  std::string filename = _descriptor.path( "solidx1.dat" );

  Clib::BinaryFile infile( filename, std::ios::in );

  size_t n_blocks = ( _descriptor.width / SOLIDX_X_SIZE ) * ( _descriptor.height / SOLIDX_Y_SIZE );
  _index1.resize( n_blocks );

  for ( size_t i = 0; i < n_blocks; ++i )
  {
    unsigned int tmp;
    infile.Read( tmp );
    if ( tmp )
    {
      // tmp is an offset, in the file..  turn it into a pointer into the second-level index.
      tmp = ( tmp - SOLIDX2_FILLER_SIZE ) / sizeof( SOLIDX2_ELEM );
      _index1[i] = &_index2.at( tmp );
    }
    else
    {
      _index1[i] = nullptr;
    }
  }
}

void MapServer::GetMapShapes( MapShapeList& shapes, unsigned short x, unsigned short y,
                              unsigned int anyflags ) const
{
  passert( x < _descriptor.width && y < _descriptor.height );

  MAPCELL cell = GetMapCell( x, y );
  MapShape shape;

  if ( cell.flags & anyflags )
  {
    shape.flags = cell.flags;
    shape.z = cell.z - 1;  // assume now map is at z-1 with height 1 for solidity
    shape.height = 1;
    shapes.push_back( shape );
  }

  if ( cell.flags & FLAG::MORE_SOLIDS )
  {
    unsigned short xblock = x >> SOLIDX_X_SHIFT;
    unsigned short xcell = x & SOLIDX_X_CELLMASK;
    unsigned short yblock = y >> SOLIDX_Y_SHIFT;
    unsigned short ycell = y & SOLIDX_Y_CELLMASK;

    size_t block = static_cast<size_t>( yblock ) * ( _descriptor.width >> SOLIDX_X_SHIFT ) + xblock;
    SOLIDX2_ELEM* pIndex2 = _index1[block];
    unsigned int index = pIndex2->baseindex + pIndex2->addindex[xcell][ycell];
    const SOLIDS_ELEM* pElem = &_shapedata[index];
    for ( ;; )
    {
      if ( pElem->flags & anyflags )
      {
        shape.z = pElem->z;
        shape.height = pElem->height;
        shape.flags = pElem->flags;
        shapes.push_back( shape );
      }
      if ( pElem->flags & FLAG::MORE_SOLIDS )
        ++pElem;
      else
        break;
    }
  }
}

MapServer* MapServer::Create( const RealmDescriptor& descriptor )
{
  if ( descriptor.mapserver_type == "memory" )
  {
    return new InMemoryMapServer( descriptor );
  }
  if ( descriptor.mapserver_type == "file" )
  {
    return new FileMapServer( descriptor );
  }

  throw std::runtime_error( "Undefined mapserver type: " + descriptor.mapserver_type );
}

size_t MapServer::sizeEstimate() const
{
  return sizeof( *this ) + _descriptor.sizeEstimate() + Clib::memsize( _index1 ) +
         Clib::memsize( _index2 ) + Clib::memsize( _shapedata );
}
}  // namespace Pol::Plib
