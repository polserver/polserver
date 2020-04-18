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
      gumpwidth( 0 ),
      gumpheight( 0 ),
      editable( mapdesc.editable ),
      plotting( false ),
      pin_points( 0 ),
      area( Pos2d( 0, 0 ), Pos2d( 0, 0 ), nullptr ),
      facetid( 0 )
{
}

Map::~Map() {}

void Map::printProperties( Clib::StreamWriter& sw ) const
{
  base::printProperties( sw );

  sw() << "\txwest\t" << area.posL().x() << pf_endl;
  sw() << "\tynorth\t" << area.posL().y() << pf_endl;
  sw() << "\txeast\t" << area.posH().x() << pf_endl;
  sw() << "\tysouth\t" << area.posH().y() << pf_endl;
  sw() << "\tgumpwidth\t" << gumpwidth << pf_endl;
  sw() << "\tgumpheight\t" << gumpheight << pf_endl;
  sw() << "\tfacetid\t" << facetid << pf_endl;

  sw() << "\teditable\t" << editable << pf_endl;

  printPinPoints( sw );

  sw() << pf_endl;
}

void Map::printPinPoints( Clib::StreamWriter& sw ) const
{
  PinPoints::const_iterator itr;
  int i = 0;
  sw() << "\tNumPins " << pin_points.size() << pf_endl;

  for ( itr = pin_points.begin(); itr != pin_points.end(); ++itr, ++i )
  {
    sw() << "\tPin" << i << " " << itr->x() << "," << itr->y() << pf_endl;
  }
}

void Map::readProperties( Clib::ConfigElem& elem )
{
  base::readProperties( elem );

  area.posL( Pos2d( elem.remove_ushort( "xwest", 0 ), elem.remove_ushort( "ynorth", 0 ) ) );
  area.posH( Pos2d( elem.remove_ushort( "xeast", 0 ), elem.remove_ushort( "ysouth", 0 ) ) );
  gumpwidth = elem.remove_ushort( "gumpwidth", 0 );
  gumpheight = elem.remove_ushort( "gumpheight", 0 );
  facetid = elem.remove_ushort( "facetid", 0 );
  editable = elem.remove_bool( "editable", false );

  unsigned short numpins = elem.remove_ushort( "NumPins", 0 );
  std::string pinval;
  int i, px, py;
  Pos2d pp;

  for ( i = 0; i < numpins; i++ )
  {
    std::string search_string( "Pin" );
    search_string += Clib::tostring( i );
    pinval = elem.remove_string( search_string.c_str() );
    sscanf( pinval.c_str(), "%i,%i", &px, &py );

    pp.x( static_cast<unsigned short>( px ) );
    pp.y( static_cast<unsigned short>( py ) );

    pin_points.push_back( pp );
  }
}

void Map::builtin_on_use( Network::Client* client )
{
  if ( gumpwidth && gumpheight )
  {
    if ( client->ClientType & Network::CLIENTTYPE_70130 )
    {
      Network::PktHelper::PacketOut<Network::PktOut_F5> msgF5;
      msgF5->Write<u32>( serial_ext );
      msgF5->Write<u8>( 0x13u );
      msgF5->Write<u8>( 0x9du );
      msgF5->WriteFlipped<u16>( area.posL().x() );
      msgF5->WriteFlipped<u16>( area.posL().y() );
      msgF5->WriteFlipped<u16>( area.posH().x() );
      msgF5->WriteFlipped<u16>( area.posH().y() );
      msgF5->WriteFlipped<u16>( gumpwidth );
      msgF5->WriteFlipped<u16>( gumpheight );
      msgF5->WriteFlipped<u16>( facetid );
      msgF5.Send( client );
    }
    else
    {
      Network::PktHelper::PacketOut<Network::PktOut_90> msg90;
      msg90->Write<u32>( serial_ext );
      msg90->Write<u8>( 0x13u );
      msg90->Write<u8>( 0x9du );
      msg90->WriteFlipped<u16>( area.posL().x() );
      msg90->WriteFlipped<u16>( area.posL().y() );
      msg90->WriteFlipped<u16>( area.posH().x() );
      msg90->WriteFlipped<u16>( area.posH().y() );
      msg90->WriteFlipped<u16>( gumpwidth );
      msg90->WriteFlipped<u16>( gumpheight );
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

      for ( pin_points_itr itr = pin_points.begin(), end = pin_points.end(); itr != end; ++itr )
      {
        msg56->offset = 7;  // msgtype+serial_ext+type,pinidx
        Pos2d g = worldToGump( *itr );
        msg56->WriteFlipped<u16>( g.x() );
        msg56->WriteFlipped<u16>( g.y() );
        msg56.Send( client );
      }
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
    for ( PinPoints::iterator itr = pin_points.begin(); itr != pin_points.end(); ++itr )
    {
      std::unique_ptr<BStruct> struc( new BStruct );
      struc->addMember( "x", new BLong( itr->x() ) );
      struc->addMember( "y", new BLong( itr->y() ) );

      arr->addElement( struc.release() );
    }
    return arr.release();
  }

  case MTH_INSERTPIN:
  {
    int idx;
    Pos2d p;
    if ( ex.getParam( 0, idx, static_cast<int>( pin_points.size() ) ) &&
         ex.getPos2dParam( 1, 2, &p ) )
    {
      pin_points_itr itr;

      if ( !realm()->valid( p ) )
        return new BError( "Invalid Coordinates for Realm" );

      itr = pin_points.begin();
      itr += idx;

      set_dirty();
      pin_points.insert( itr, p );

      return new BLong( 1 );
    }
    else
    {
      return new BError( "Invalid parameter type" );
    }
  }

  case MTH_APPENDPIN:
  {
    Pos2d p;
    if ( ex.getPos2dParam( 0, 1, &p ) )
    {
      if ( !realm()->valid( p ) )
        return new BError( "Invalid Coordinates for Realm" );
      set_dirty();
      pin_points.push_back( p );
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
      pin_points_itr itr;

      itr = pin_points.begin();
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

bool Map::msgCoordsInBounds( PKTBI_56* msg ) const
{
  Pos2d newp( cfBEu16( msg->pinx ), cfBEu16( msg->piny ) );
  newp += area.posL().from_origin();
  return area.contains( newp );
}

Pos2d Map::gumpToWorld( const Pos2d& gump ) const
{
  Vec2d size = area.posH() - area.posL();
  float world_xtiles_per_pixel = (float)( size.x() ) / (float)gumpwidth;
  float world_ytiles_per_pixel = (float)( size.y() ) / (float)gumpheight;
  return Pos2d( ( u16 )( area.posL().x() + ( world_xtiles_per_pixel * gump.x() ) ),
                ( u16 )( area.posL().y() + ( world_ytiles_per_pixel * gump.y() ) ) );
}

Pos2d Map::worldToGump( const Pos2d& world ) const
{
  Vec2d size = area.posH() - area.posL();
  float world_xtiles_per_pixel = (float)( size.x() ) / (float)gumpwidth;
  float world_ytiles_per_pixel = (float)( size.y() ) / (float)gumpheight;
  Vec2d delta( world - area.posL() );
  return Pos2d( ( u16 )( ( delta.x() ) / world_xtiles_per_pixel ),
                ( u16 )( ( delta.y() ) / world_ytiles_per_pixel ) );
}

Items::Item* Map::clone() const
{
  Map* map = static_cast<Map*>( base::clone() );

  map->gumpwidth = gumpwidth;
  map->gumpheight = gumpheight;
  map->editable = editable;
  map->plotting = plotting;
  map->pin_points = pin_points;
  map->area = area;
  map->facetid = facetid;
  return map;
}

size_t Map::estimatedSize() const
{
  return base::estimatedSize() + sizeof( u16 ) /*gumpwidth*/
         + sizeof( u16 )                       /*gumpheight*/
         + sizeof( bool )                      /*editable*/
         + sizeof( bool )                      /*plotting*/
         + sizeof( Area2d )                    /*area*/
         + sizeof( u16 )                       /*facetid*/
         + 3 * sizeof( Pos2d* ) + pin_points.capacity() * sizeof( Pos2d );
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
  {
    if ( !( my_map->plotting ) )
      return;
    if ( !my_map->msgCoordsInBounds( msg ) )
      return;

    Pos2d p( my_map->gumpToWorld( Pos2d( cfBEu16( msg->pinx ), cfBEu16( msg->piny ) ) ) );
    my_map->pin_points.push_back( p );
    break;
  }
  case PKTBI_56::TYPE_INSERT:
  {
    if ( !( my_map->plotting ) )
      return;
    if ( msg->pinidx >= my_map->pin_points.size() )  // pinidx out of range
      return;
    if ( !my_map->msgCoordsInBounds( msg ) )
      return;

    auto itr = my_map->pin_points.begin();
    itr += msg->pinidx;

    Pos2d p( my_map->gumpToWorld( Pos2d( cfBEu16( msg->pinx ), cfBEu16( msg->piny ) ) ) );

    my_map->pin_points.insert( itr, p );
    break;
  }

  case PKTBI_56::TYPE_CHANGE:
  {
    if ( !( my_map->plotting ) )
      return;
    if ( msg->pinidx >= my_map->pin_points.size() )  // pinidx out of range
      return;
    if ( !my_map->msgCoordsInBounds( msg ) )
      return;

    auto itr = my_map->pin_points.begin();
    itr += msg->pinidx;

    *itr = my_map->gumpToWorld( Pos2d( cfBEu16( msg->pinx ), cfBEu16( msg->piny ) ) );

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
