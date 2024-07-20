/** @file
 *
 * @par History
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2009/07/23 MuadDib:   updates for new Enum::Packet Out ID
 * - 2012/04/14 Tomi:      added new map message packet 0xF5 for new clients >= 7.0.13.0 and facetid
 * member for maps
 */

#include "umap.h"

#include <stdio.h>
#include <string>

#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/bstruct.h"
#include "../bscript/executor.h"
#include "../bscript/objmethods.h"
#include "../clib/cfgelem.h"
#include "../clib/clib_endian.h"
#include "../clib/stlutil.h"
#include "../clib/streamsaver.h"
#include "../clib/strutil.h"
#include "globals/uvars.h"
#include "item/itemdesc.h"
#include "network/client.h"
#include "network/packethelper.h"
#include "network/packets.h"
#include "network/pktboth.h"
#include "realms/realm.h"
#include "syshookscript.h"
#include "ufunc.h"
#include "uobject.h"
#include "uoexec.h"

namespace Pol
{
namespace Core
{
Map::Map( const Items::MapDesc& mapdesc )
    : Item( mapdesc, UOBJ_CLASS::CLASS_ITEM ),
      gumpsize(),
      editable( mapdesc.editable ),
      plotting( false ),
      pin_points(),
      xwest( 0 ),
      xeast( 0 ),
      ynorth( 0 ),
      ysouth( 0 ),
      facetid( 0 )
{
}

void Map::printProperties( Clib::StreamWriter& sw ) const
{
  base::printProperties( sw );
  Range2d area = getrange();
  sw.add( "xwest", area.nw().x() );
  sw.add( "ynorth", area.nw().y() );
  sw.add( "xeast", area.se().x() );
  sw.add( "ysouth", area.se().y() );
  sw.add( "gumpwidth", gumpsize.x() );
  sw.add( "gumpheight", gumpsize.y() );
  sw.add( "facetid", facetid );

  sw.add( "editable", editable );

  printPinPoints( sw );
}

void Map::printPinPoints( Clib::StreamWriter& sw ) const
{
  int i = 0;
  sw.add( "NumPins", pin_points.size() );

  for ( auto itr = pin_points.begin(); itr != pin_points.end(); ++itr, ++i )
  {
    sw.add( fmt::format( "Pin{}", i ), fmt::format( "{},{}", itr->x(), itr->y() ) );
  }
}

void Map::readProperties( Clib::ConfigElem& elem )
{
  base::readProperties( elem );

  Range2d area( Pos2d( elem.remove_ushort( "xwest", 0 ), elem.remove_ushort( "ynorth", 0 ) ),
                Pos2d( elem.remove_ushort( "xeast", 0 ), elem.remove_ushort( "ysouth", 0 ) ),
                nullptr );
  xwest = area.nw().x();
  ynorth = area.nw().y();
  xeast = area.se().x();
  ysouth = area.se().y();

  gumpsize = Pos2d( elem.remove_ushort( "gumpwidth", 0 ), elem.remove_ushort( "gumpheight", 0 ) );
  facetid = elem.remove_ushort( "facetid", 0 );
  editable = elem.remove_bool( "editable", false );

  unsigned short numpins = elem.remove_ushort( "NumPins", 0 );
  std::string pinval;
  int i, px, py;

  for ( i = 0; i < numpins; i++ )
  {
    std::string search_string( "Pin" );
    search_string += Clib::tostring( i );
    pinval = elem.remove_string( search_string.c_str() );
    sscanf( pinval.c_str(), "%i,%i", &px, &py );

    pin_points.push_back(
        Pos2d( static_cast<unsigned short>( px ), static_cast<unsigned short>( py ) ) );
  }
}

void Map::builtin_on_use( Network::Client* client )
{
  if ( gumpsize == Pos2d( 0, 0 ) )
    return;
  Range2d area = getrange();
  if ( client->ClientType & Network::CLIENTTYPE_70130 )
  {
    Network::PktHelper::PacketOut<Network::PktOut_F5> msgF5;
    msgF5->Write<u32>( serial_ext );
    msgF5->Write<u8>( 0x13u );
    msgF5->Write<u8>( 0x9du );
    msgF5->WriteFlipped<u16>( area.nw().x() );
    msgF5->WriteFlipped<u16>( area.nw().y() );
    msgF5->WriteFlipped<u16>( area.se().x() );
    msgF5->WriteFlipped<u16>( area.se().y() );
    msgF5->WriteFlipped<u16>( gumpsize.x() );
    msgF5->WriteFlipped<u16>( gumpsize.y() );
    msgF5->WriteFlipped<u16>( facetid );
    msgF5.Send( client );
  }
  else
  {
    Network::PktHelper::PacketOut<Network::PktOut_90> msg90;
    msg90->Write<u32>( serial_ext );
    msg90->Write<u8>( 0x13u );
    msg90->Write<u8>( 0x9du );
    msg90->WriteFlipped<u16>( area.nw().x() );
    msg90->WriteFlipped<u16>( area.nw().y() );
    msg90->WriteFlipped<u16>( area.se().x() );
    msg90->WriteFlipped<u16>( area.se().y() );
    msg90->WriteFlipped<u16>( gumpsize.x() );
    msg90->WriteFlipped<u16>( gumpsize.y() );
    msg90.Send( client );
  }

  Network::PktHelper::PacketOut<Network::PktOut_56> msg56;
  msg56->Write<u32>( serial_ext );
  msg56->Write<u8>( PKTBI_56::TYPE_REMOVE_ALL );
  msg56->offset += 5;  // u8 pinidx,u16 pinx,piny
  msg56.Send( client );

  // need to send each point to the client

  if ( !pin_points.empty() )
  {
    msg56->offset = 5;  // msgtype+serial_ext
    msg56->Write<u8>( PKTBI_56::TYPE_ADD );
    // u8 pinidx still 0

    for ( const auto& pin : pin_points )
    {
      msg56->offset = 7;  // msgtype+serial_ext+type,pinidx
      Pos2d gp = worldToGump( pin, area );
      msg56->WriteFlipped<u16>( gp.x() );
      msg56->WriteFlipped<u16>( gp.y() );
      msg56.Send( client );
    }
  }
}

Bscript::BObjectImp* Map::script_method_id( const int id, UOExecutor& ex )
{
  using namespace Bscript;
  BObjectImp* imp = base::script_method_id( id, ex );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MTH_GETPINS:
  {
    std::unique_ptr<ObjArray> arr( new ObjArray );
    for ( const auto& pin : pin_points )
    {
      std::unique_ptr<BStruct> struc( new BStruct );
      struc->addMember( "x", new BLong( pin.x() ) );
      struc->addMember( "y", new BLong( pin.y() ) );

      arr->addElement( struc.release() );
    }
    return arr.release();
  }

  case MTH_INSERTPIN:
  {
    int idx;
    Pos2d pin;
    if ( ex.getParam( 0, idx, static_cast<int>( pin_points.size() ) ) &&
         ex.getPos2dParam( 1, 2, &pin, realm() ) )
    {
      auto itr = pin_points.begin();
      itr += idx;

      set_dirty();
      pin_points.insert( itr, pin );

      return new BLong( 1 );
    }
    else
    {
      return new BError( "Invalid parameter type" );
    }
  }

  case MTH_APPENDPIN:
  {
    Pos2d pin;
    if ( ex.getPos2dParam( 0, 1, &pin, realm() ) )
    {
      set_dirty();
      pin_points.push_back( pin );
      return new BLong( 1 );
    }
    else
    {
      return new BError( "Invalid parameter type" );
    }
  }

  case MTH_ERASEPIN:
  {
    int idx;
    if ( ex.getParam( 0, idx, static_cast<int>( pin_points.size() - 1 ) ) )
    {
      auto itr = pin_points.begin();
      itr += idx;

      set_dirty();
      pin_points.erase( itr );
      return new BLong( 1 );
    }
    else
    {
      return new BError( "Index Out of Range" );
    }
  }

  default:
    return nullptr;
  }
}


Bscript::BObjectImp* Map::script_method( const char* methodname, UOExecutor& ex )
{
  Bscript::BObjectImp* imp = base::script_method( methodname, ex );
  if ( imp != nullptr )
    return imp;

  Bscript::ObjMethod* objmethod = Bscript::getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->script_method_id( objmethod->id, ex );
  else
    return nullptr;
}

Range2d Map::getrange() const
{
  return Range2d( Pos2d( xwest, ynorth ), Pos2d( xeast, ysouth ), nullptr );
}

bool Map::msgCoordsInBounds( PKTBI_56* msg, const Range2d& area ) const
{
  Pos2d newpos( cfBEu16( msg->pinx ), cfBEu16( msg->piny ) );
  newpos += area.nw().from_origin();
  return area.contains( newpos );
}

Pos2d Map::gumpToWorld( const Pos2d& gump, const Range2d& area ) const
{
  Vec2d size = area.se() - area.nw();
  float world_xtiles_per_pixel = (float)( size.x() ) / gumpsize.x();
  float world_ytiles_per_pixel = (float)( size.y() ) / gumpsize.y();
  return Pos2d( (u16)( area.nw().x() + ( world_xtiles_per_pixel * gump.x() ) ),
                (u16)( area.nw().y() + ( world_ytiles_per_pixel * gump.y() ) ) );
}

Pos2d Map::worldToGump( const Pos2d& world, const Range2d& area ) const
{
  Vec2d size = area.se() - area.nw();
  float world_xtiles_per_pixel = (float)( size.x() ) / gumpsize.x();
  float world_ytiles_per_pixel = (float)( size.y() ) / gumpsize.y();
  Vec2d delta( world - area.nw() );
  return Pos2d( (u16)( ( delta.x() ) / world_xtiles_per_pixel ),
                (u16)( ( delta.y() ) / world_ytiles_per_pixel ) );
}

// dave 12-20
Items::Item* Map::clone() const
{
  Map* map = static_cast<Map*>( base::clone() );

  map->gumpsize = gumpsize;
  map->editable = editable;
  map->plotting = plotting;
  map->pin_points = pin_points;
  map->xwest = xwest;
  map->ynorth = ynorth;
  map->xeast = xeast;
  map->ysouth = ysouth;
  map->facetid = facetid;
  return map;
}

size_t Map::estimatedSize() const
{
  return base::estimatedSize() + sizeof( Pos2d ) /*gumpsize*/
         + sizeof( bool )                        /*editable*/
         + sizeof( bool )                        /*plotting*/
         + 4 * sizeof( u16 )                     /*area*/
         + sizeof( u16 )                         /*facetid*/
         + Clib::memsize( pin_points );
}

bool Map::get_method_hook( const char* methodname, Bscript::Executor* ex, ExportScript** hook,
                           unsigned int* PC ) const
{
  if ( gamestate.system_hooks.get_method_hook( gamestate.system_hooks.map_method_script.get(),
                                               methodname, ex, hook, PC ) )
    return true;
  return base::get_method_hook( methodname, ex, hook, PC );
}

void handle_map_pin( Network::Client* client, PKTBI_56* msg )
{
  // FIXME you really need to check that the item is in fact a map.
  // Can cause crash if someone is messing with their packets to script
  // pin movement on a non-map item.
  Map* my_map = (Map*)find_legal_item( client->chr, cfBEu32( msg->serial ) );
  if ( my_map == nullptr )
    return;
  if ( my_map->editable == false )
    return;

  Range2d area = my_map->getrange();
  switch ( msg->type )
  {
  case PKTBI_56::TYPE_TOGGLE_EDIT:
  {
    // hmm msg->plotstate never seems to be 1 when type is 6
    my_map->plotting = my_map->plotting ? false : true;
    Network::PktHelper::PacketOut<Network::PktOut_56> msg56;
    msg56->Write<u32>( msg->serial );
    msg56->Write<u8>( PKTBI_56::TYPE_TOGGLE_RESPONSE );
    msg56->Write<u8>( my_map->plotting ? 1u : 0u );
    msg56->Write<u16>( msg->pinx );
    msg56->Write<u16>( msg->piny );
    msg56.Send( client );
    break;
  }

  case PKTBI_56::TYPE_ADD:
    if ( !( my_map->plotting ) )
      return;
    if ( !my_map->msgCoordsInBounds( msg, area ) )
      return;
    my_map->pin_points.push_back(
        my_map->gumpToWorld( Pos2d( cfBEu16( msg->pinx ), cfBEu16( msg->piny ) ), area ) );
    break;

  case PKTBI_56::TYPE_INSERT:
  {
    if ( !( my_map->plotting ) )
      return;
    if ( msg->pinidx >= my_map->pin_points.size() )  // pinidx out of range
      return;
    if ( !my_map->msgCoordsInBounds( msg, area ) )
      return;

    auto itr = my_map->pin_points.begin();
    itr += msg->pinidx;
    my_map->pin_points.insert(
        itr, my_map->gumpToWorld( Pos2d( cfBEu16( msg->pinx ), cfBEu16( msg->piny ) ), area ) );
    break;
  }

  case PKTBI_56::TYPE_CHANGE:
  {
    if ( !( my_map->plotting ) )
      return;
    if ( msg->pinidx >= my_map->pin_points.size() )  // pinidx out of range
      return;
    if ( !my_map->msgCoordsInBounds( msg, area ) )
      return;

    auto itr = my_map->pin_points.begin();
    itr += msg->pinidx;
    *itr = my_map->gumpToWorld( Pos2d( cfBEu16( msg->pinx ), cfBEu16( msg->piny ) ), area );
    break;
  }

  case PKTBI_56::TYPE_REMOVE:
  {
    if ( !( my_map->plotting ) )
      return;
    if ( msg->pinidx >= my_map->pin_points.size() )  // pinidx out of range
      return;

    auto itr = my_map->pin_points.begin();
    itr += msg->pinidx;

    my_map->pin_points.erase( itr );

    break;
  }

  case PKTBI_56::TYPE_REMOVE_ALL:
    if ( !( my_map->plotting ) )
      return;

    my_map->pin_points.clear();

    break;
  }
}
}  // namespace Core
}  // namespace Pol
