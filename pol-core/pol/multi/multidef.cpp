/** @file
 *
 * @par History
 * - 2009/09/01 Turley:    VS2005/2008 Support moved inline MultiDef::getkey to .h
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 */


#include "multidef.h"

#include <cstdio>
#include <sstream>
#include <string>

#include "../../bscript/bstruct.h"
#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/passert.h"
#include "../../clib/strutil.h"
#include "../globals/multidefs.h"

namespace Pol
{
namespace Multi
{
bool BoatShapeExists( u16 graphic );

MultiDef::MultiDef( Clib::ConfigElem& elem, u16 multiid )
    : multiid( multiid ),
      type( UNKNOWN ),
      elems(),

      xbase( 0 ),
      xsize( 0 ),
      ybase( 0 ),
      ysize( 0 ),

      minrx( 0 ),
      minry( 0 ),
      minrz( 0 ),
      maxrx( 0 ),
      maxry( 0 ),
      maxrz( 0 )
{
  if ( elem.type_is( "BOAT" ) )
  {
    if ( !BoatShapeExists( multiid ) )
      elem.throw_error( "Entry for Boat (multiid " + Clib::hexint( multiid ) +
                        ") not found in boats.cfg" );
    type = BOAT;
  }
  else if ( elem.type_is( "HOUSE" ) )
  {
    type = HOUSE;
  }
  else if ( elem.type_is( "STAIRS" ) )
  {
    type = STAIRS;
  }

  std::string tmp;
  while ( elem.remove_prop( "static", &tmp ) )
  {
    std::istringstream is( tmp );
    MULTI_ELEM multielem;
    multielem.is_static = true;
    if ( is >> std::hex >> multielem.objtype >> std::dec >> multielem.x >> multielem.y >>
         multielem.z )
    {
      elems.push_back( multielem );
    }
    else
    {
      std::string message = "Badly formed static directive: '" + tmp + "'";
      elem.throw_error( message );
    }
  }
  while ( elem.remove_prop( "dynamic", &tmp ) )
  {
    std::istringstream is( tmp );
    MULTI_ELEM multielem;
    multielem.is_static = false;
    if ( is >> std::hex >> multielem.objtype >> std::dec >> multielem.x >> multielem.y >>
         multielem.z )
    {
      elems.push_back( multielem );
    }
    else
    {
      std::string message = "Badly formed dynamic directive: '" + tmp + "'";
      elem.throw_error( message );
    }
  }
}

MultiDef::~MultiDef() {}

bool MultiDef::findcomponents( Components::const_iterator& beg, Components::const_iterator& end,
                               short rx, short ry ) const
{
  ItrPair pr = components.equal_range( getkey( rx, ry ) );
  if ( pr.first == components.end() )
  {
    return false;
  }
  else
  {
    beg = pr.first;
    end = pr.second;
    return true;
  }
}

bool MultiDef::body_contains( const Core::Vec2d& rxy ) const
{
  return ( components.count( getkey( rxy ) ) != 0 );
}
const MULTI_ELEM* MultiDef::find_component( short rx, short ry ) const
{
  if ( body_contains( rx, ry ) )
    return ( *components.find( getkey( rx, ry ) ) ).second;
  else
    return nullptr;
}

void MultiDef::add_to_hull( const MULTI_ELEM* elem )
{
  unsigned short k = getkey( elem->x, elem->y );

  if ( !hull2.count( k ) )
  {
    hull.push_back( elem );
    hull2.insert( k );
  }


  if ( type == BOAT )
  {
    short int_rx = elem->x, int_ry = elem->y;
    if ( ( multiid & 1 ) == 0 )  // N/S hull, so squeeze X
    {
      if ( elem->x == minrx )
        int_rx = minrx + 1;
      else if ( elem->x == maxrx )
        int_rx = maxrx - 1;
    }
    else
    {
      if ( elem->y == minry )
        int_ry = minry + 1;
      else if ( elem->y > 0 )
        int_ry = maxry - 1;
    }
    elem = find_component( int_rx, int_ry );
    if ( elem )
      add_to_internal_hull( elem );
  }
}

void MultiDef::add_to_internal_hull( const MULTI_ELEM* elem )
{
  unsigned short k = getkey( elem->x, elem->y );

  if ( !internal_hull2.count( k ) )
  {
    internal_hull.push_back( elem );
    internal_hull2.insert( k );
  }
}

void MultiDef::add_body_tohull()
{
  short rx, ry;
  for ( ry = minry; ry <= maxry; ++ry )
  {
    for ( rx = minrx; rx <= maxrx; ++rx )
    {
      const MULTI_ELEM* elem = find_component( rx, ry );
      if ( elem != nullptr )
      {
        add_to_hull( elem );

        break;
      }
    }
    for ( rx = maxrx; rx >= minrx; --rx )
    {
      const MULTI_ELEM* elem = find_component( rx, ry );
      if ( elem != nullptr )
      {
        add_to_hull( elem );

        break;
      }
    }
  }


  for ( rx = minrx; rx <= maxrx; ++rx )
  {
    for ( ry = minry; ry <= maxry; ++ry )
    {
      const MULTI_ELEM* elem = find_component( rx, ry );
      if ( elem != nullptr )
      {
        add_to_hull( elem );
        break;
      }
    }
    for ( ry = maxry; ry >= minry; --ry )
    {
      const MULTI_ELEM* elem = find_component( rx, ry );
      if ( elem != nullptr )
      {
        add_to_hull( elem );
        break;
      }
    }
  }
}


void MultiDef::computehull()
{
  add_body_tohull();
}

void MultiDef::addrec( const MULTI_ELEM* elem )
{
  if ( elem->x < minrx )
    minrx = elem->x;
  if ( elem->y < minry )
    minry = elem->y;
  if ( elem->z < minrz )
    minrz = elem->z;

  if ( elem->x > maxrx )
    maxrx = elem->x;
  if ( elem->y > maxry )
    maxry = elem->y;
  if ( elem->z > maxrz )
    maxrz = elem->z;

  if ( elem->x < global_minrx )
    global_minrx = elem->x;
  if ( elem->y < global_minry )
    global_minry = elem->y;
  if ( elem->z < global_minrz )
    global_minrz = elem->z;

  if ( elem->x > global_maxrx )
    global_maxrx = elem->x;
  if ( elem->y > global_maxry )
    global_maxry = elem->y;
  if ( elem->z > global_maxrz )
    global_maxrz = elem->z;

  components.insert( Components::value_type( getkey( elem->x, elem->y ), elem ) );
}

void MultiDef::init()
{
  for ( unsigned i = 0; i < elems.size(); i++ )
  {
    addrec( &elems[i] );
  }
  computehull();
}

size_t MultiDef::estimateSize() const
{
  size_t size = sizeof( MultiDef );
  size += 3 * sizeof( MULTI_ELEM* ) + elems.capacity() * sizeof( MULTI_ELEM );
  size += 3 * sizeof( MULTI_ELEM** ) + hull.capacity() * sizeof( MULTI_ELEM* );
  size += 3 * sizeof( MULTI_ELEM** ) + internal_hull.capacity() * sizeof( MULTI_ELEM* );

  size += 3 * sizeof( void* ) + hull2.size() * ( sizeof( unsigned short ) + 3 * sizeof( void* ) );
  size += 3 * sizeof( void* ) +
          internal_hull2.size() * ( sizeof( unsigned short ) + 3 * sizeof( void* ) );

  size += ( sizeof( unsigned short ) + sizeof( MULTI_ELEM* ) + ( sizeof( void* ) * 3 + 1 ) / 2 ) *
          components.size();
  return size;
}

short MultiDef::global_minrx;
short MultiDef::global_minry;
short MultiDef::global_minrz;
short MultiDef::global_maxrx;
short MultiDef::global_maxry;
short MultiDef::global_maxrz;

bool MultiDefByMultiIDExists( u16 multiid )
{
  return multidef_buffer.multidefs_by_multiid.count( multiid ) != 0;
}
const MultiDef* MultiDefByMultiID( u16 multiid )
{
  passert( multidef_buffer.multidefs_by_multiid.count( multiid ) != 0 );

  MultiDefs::const_iterator citr = multidef_buffer.multidefs_by_multiid.find( multiid );
  if ( citr != multidef_buffer.multidefs_by_multiid.end() )
    return ( *citr ).second;
  else
    return nullptr;
}


void read_multidefs()
{
  Clib::ConfigFile cf( "config/multis.cfg", "BOAT HOUSE STAIRS" );
  Clib::ConfigElem elem;
  while ( cf.read( elem ) )
  {
    u16 multiid = static_cast<u16>( strtoul( elem.rest(), nullptr, 0 ) );

    MultiDef* mdef = new MultiDef( elem, multiid );
    mdef->init();

    multidef_buffer.multidefs_by_multiid[mdef->multiid] = mdef;
  }
}
}
}
