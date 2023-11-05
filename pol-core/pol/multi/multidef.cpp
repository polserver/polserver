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

      minrxyz(),
      maxrxyz(),
      max_radius( 0 )
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
    s16 x, y, z;
    if ( is >> std::hex >> multielem.objtype >> std::dec >> x >> y >> z )
    {
      multielem.relpos.x( x ).y( y ).z( z );
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
    s16 x, y, z;
    if ( is >> std::hex >> multielem.objtype >> std::dec >> x >> y >> z )
    {
      multielem.relpos.x( x ).y( y ).z( z );
      elems.push_back( multielem );
    }
    else
    {
      std::string message = "Badly formed dynamic directive: '" + tmp + "'";
      elem.throw_error( message );
    }
  }
}

bool MultiDef::findcomponents( Components::const_iterator& beg, Components::const_iterator& end,
                               const Core::Vec2d& rxy ) const
{
  ItrPair pr = components.equal_range( getkey( rxy ) );
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
const MULTI_ELEM* MultiDef::find_component( const Core::Vec2d& rxy ) const
{
  if ( body_contains( rxy ) )
    return ( *components.find( getkey( rxy ) ) ).second;
  else
    return nullptr;
}

void MultiDef::add_to_hull( const MULTI_ELEM* elem )
{
  unsigned short k = getkey( elem->relpos.xy() );

  if ( !hull2.count( k ) )
  {
    hull.push_back( elem );
    hull2.insert( k );
  }


  if ( type == BOAT )
  {
    Core::Vec2d rxy = elem->relpos.xy();
    if ( ( multiid & 1 ) == 0 )  // N/S hull, so squeeze X
    {
      if ( rxy.x() == minrxyz.x() )
        rxy.x( minrxyz.x() + 1 );
      else if ( rxy.x() == maxrxyz.x() )
        rxy.x( maxrxyz.x() - 1 );
    }
    else
    {
      if ( rxy.y() == minrxyz.y() )
        rxy.y( minrxyz.y() + 1 );
      else if ( rxy.y() > 0 )
        rxy.y( maxrxyz.y() - 1 );
    }
    elem = find_component( rxy );
    if ( elem )
      add_to_internal_hull( elem );
  }
}

void MultiDef::add_to_internal_hull( const MULTI_ELEM* elem )
{
  unsigned short k = getkey( elem->relpos.xy() );

  if ( !internal_hull2.count( k ) )
  {
    internal_hull.push_back( elem );
    internal_hull2.insert( k );
  }
}

void MultiDef::add_body_tohull()
{
  short rx, ry;
  for ( ry = minrxyz.y(); ry <= maxrxyz.y(); ++ry )
  {
    for ( rx = minrxyz.x(); rx <= maxrxyz.x(); ++rx )
    {
      const MULTI_ELEM* elem = find_component( Core::Vec2d( rx, ry ) );
      if ( elem != nullptr )
      {
        add_to_hull( elem );

        break;
      }
    }
    for ( rx = maxrxyz.x(); rx >= minrxyz.x(); --rx )
    {
      const MULTI_ELEM* elem = find_component( Core::Vec2d( rx, ry ) );
      if ( elem != nullptr )
      {
        add_to_hull( elem );

        break;
      }
    }
  }


  for ( rx = minrxyz.x(); rx <= maxrxyz.x(); ++rx )
  {
    for ( ry = minrxyz.y(); ry <= maxrxyz.y(); ++ry )
    {
      const MULTI_ELEM* elem = find_component( Core::Vec2d( rx, ry ) );
      if ( elem != nullptr )
      {
        add_to_hull( elem );
        break;
      }
    }
    for ( ry = maxrxyz.y(); ry >= minrxyz.y(); --ry )
    {
      const MULTI_ELEM* elem = find_component( Core::Vec2d( rx, ry ) );
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
  minrxyz.x( std::min( minrxyz.x(), elem->relpos.x() ) )
      .y( std::min( minrxyz.y(), elem->relpos.y() ) )
      .z( std::min( minrxyz.z(), elem->relpos.z() ) );
  maxrxyz.x( std::max( maxrxyz.x(), elem->relpos.x() ) )
      .y( std::max( maxrxyz.y(), elem->relpos.y() ) )
      .z( std::max( maxrxyz.z(), elem->relpos.z() ) );
  max_radius = (u8)std::max( { std::abs( minrxyz.x() ), std::abs( minrxyz.y() ),
                               std::abs( maxrxyz.x() ), std::abs( maxrxyz.y() ) } );
  components.insert( Components::value_type( getkey( elem->relpos.xy() ), elem ) );
}

void MultiDef::init()
{
  for ( unsigned i = 0; i < elems.size(); i++ )
  {
    addrec( &elems[i] );
  }
  computehull();
}

bool MultiDef::within_multi( const Core::Vec2d& relxy ) const
{
  return relxy.x() >= minrxyz.x() && relxy.x() <= maxrxyz.x() && relxy.y() >= minrxyz.y() &&
         relxy.y() <= maxrxyz.y();
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
}  // namespace Multi
}  // namespace Pol
