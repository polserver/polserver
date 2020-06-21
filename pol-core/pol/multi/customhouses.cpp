/** @file
 *
 * @par History
 * - 2005/08/19 Shinigami: ZLib functionality linked directly into Core
 * - 2005/09/26 Shinigami: wrong styled break condition in CustomHouseDesign::Compress
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 *
 * @todo Will require a special Character member to check in all kinds of places.
 * Character.EditHouse=0/1
 * The purpose, is to make NPCs IGNORE THIS PERSON, and to NOT send ANY packets about this person
 * to ANYONE. This will also need to be checked when using all sorts of crap. Like when triggering
 * equip/insert/item use/etc. This char should be able to do NOTHING except edit the house when
 * in edit house mode, etc.
 * Enabling this mode can be internal only, simply by linking it with the Custom housing tool.
 * Have a script that can be made to run additional stuff with using the tool, so scripters can
 * check extra stuff if they want, etc, and that script's return decides if to activate the housing
 * tool. Should suffice.
 */


#include "customhouses.h"

#include <cstddef>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "../../bscript/bstruct.h"
#include "../../clib/cfgelem.h"
#include "../../clib/clib_endian.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/stlutil.h"
#include "../../clib/streamsaver.h"
#include "../../plib/clidata.h"
#include "../../plib/systemstate.h"
#include "../core.h"
#include "../globals/uvars.h"
#include "../item/item.h"
#include "../item/itemdesc.h"
#include "../mkscrobj.h"
#include "../mobile/charactr.h"
#include "../network/cgdata.h"
#include "../network/client.h"
#include "../network/packethelper.h"
#include "../network/packets.h"
#include "../network/pktboth.h"
#include "../network/pktout.h"
#include "../network/pktoutid.h"
#include "../realms/realm.h"
#include "../scrdef.h"
#include "../scrsched.h"
#include "../syshook.h"
#include "../ufunc.h"
#include "../uoscrobj.h"
#include "../uworld.h"
#include "house.h"
#include "multidef.h"

#ifdef USE_SYSTEM_ZLIB
#include <zlib.h>
#else
#include "../../../lib/zlib/zlib.h"
#endif


namespace Pol
{
namespace Multi
{
// bytes per tile - currently only mode 0 works, meaning we send u16 graphic, s8 x,y,z offsets
#define BYTES_PER_TILE 5

// fixed z offsets for each floor
const char CustomHouseDesign::custom_house_z_xlate_table[CUSTOM_HOUSE_NUM_PLANES] = {0,  7,  27,
                                                                                     47, 67, 80};

// translate z offset to floor number, use floor below passed-in z value, unless exact match
char CustomHouseDesign::z_to_custom_house_table( char z )
{
  unsigned char i;
  for ( i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
  {
    if ( z == custom_house_z_xlate_table[i] )
      return i;
    else if ( z < custom_house_z_xlate_table[i] )
      return i - 1;
  }
  return -1;
}

CustomHouseDesign::CustomHouseDesign() : size( 0, 0 ), offset( 0, 0 )
{
  for ( int i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
    floor_sizes[i] = 0;
}

// fixme: need a copy ctor?

CustomHouseDesign::CustomHouseDesign( Core::Pos2d width_height, Core::Vec2d xyoffset )
{
  InitDesign( std::move( width_height ), std::move( xyoffset ) );
}

size_t CustomHouseDesign::estimatedSize() const
{
  size_t size = sizeof( CustomHouseDesign );
  for ( int i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
    size += Elements[i].estimatedSize();
  return size;
}

// init the geometry of the design. the design cannot exist outside the multi foundation boundary
// (exception: front steps)
void CustomHouseDesign::InitDesign( Core::Pos2d width_height, Core::Vec2d xyoffset )
{
  size = std::move( width_height );
  offset = std::move( xyoffset );
  for ( int i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
  {
    floor_sizes[i] = 0;
    Elements[i].SetSize( size );
    Elements[i].offset = offset;
  }
}

CustomHouseDesign& CustomHouseDesign::operator=( const CustomHouseDesign& design )
{
  for ( int i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
  {
    Elements[i] = design.Elements[i];
    floor_sizes[i] = design.floor_sizes[i];
  }
  return *this;
}

// Adds the element to the design
void CustomHouseDesign::Add( CUSTOM_HOUSE_ELEMENT& elem )
{
  int floor_num = z_to_custom_house_table( elem.z );
  if ( floor_num == -1 )
    return;
  Elements[floor_num].AddElement( elem );
  floor_sizes[floor_num]++;
}

// fixme: low walls not being replaced
// Replaces an existing object depending on the 2 tile heights
void CustomHouseDesign::AddOrReplace( CUSTOM_HOUSE_ELEMENT& elem )
{
  int floor_num = z_to_custom_house_table( elem.z );
  if ( floor_num == -1 )
    return;
  char adding_height = Plib::tileheight( elem.graphic );

  Core::Pos2d idx = Core::Pos2d( 0, 0 ) + ( elem.offset + offset );
  if ( !ValidLocation( idx ) )
    return;
  HouseFloorZColumn* column = &Elements[floor_num].data.at( idx.x() ).at( idx.y() );
  for ( HouseFloorZColumn::iterator itr = column->begin(), itrend = column->end(); itr != itrend;
        ++itr )
  {
    char existing_height = Plib::tileheight( itr->graphic );

    if ( ( ( existing_height == 0 ) && ( adding_height == 0 ) ) ||  // replace floor with floor
         ( ( existing_height != 0 ) && ( adding_height != 0 ) ) )   // or nonfloor with nonfloor

    {
      column->erase( itr );
      floor_sizes[floor_num]--;
      Add( elem );
      return;
    }
  }
  // no replacement, just add
  Add( elem );
}

bool CustomHouseDesign::Erase( const Core::Vec2d& off, u8 z, int minheight )
{
  int floor_num = z_to_custom_house_table( z );
  if ( floor_num == -1 )
    return false;

  Core::Pos2d idx = Core::Pos2d( 0, 0 ) + ( off + offset );
  if ( !ValidLocation( idx ) )
    return false;

  HouseFloorZColumn* column = &Elements[floor_num].data.at( idx.x() ).at( idx.y() );
  for ( HouseFloorZColumn::iterator itr = column->begin(), itrend = column->end(); itr != itrend;
        ++itr )
  {
    char t_height = Plib::tileheight( itr->graphic );
    if ( ( itr->z == z ) && ( t_height >= minheight ) )
    {
      column->erase( itr );
      floor_sizes[floor_num]--;
      return true;
    }
  }
  return false;
}


bool CustomHouseDesign::EraseGraphicAt( u16 graphic, const Core::Vec2d& off, u8 z )
{
  int floor_num = z_to_custom_house_table( z );
  if ( floor_num == -1 )
    return false;

  Core::Pos2d idx = Core::Pos2d( 0, 0 ) + ( off + offset );
  if ( !ValidLocation( idx ) )
    return false;
  HouseFloorZColumn* column = &Elements[floor_num].data.at( idx.x() ).at( idx.y() );
  for ( HouseFloorZColumn::iterator itr = column->begin(), itrend = column->end(); itr != itrend;
        ++itr )
  {
    if ( itr->graphic == graphic )
    {
      column->erase( itr );
      floor_sizes[floor_num]--;
      return true;
    }
  }
  return false;
}

void CustomHouseDesign::ReplaceDirtFloor( const Core::Vec2d& off )
{
  int floor_num = 1;  // dirt always goes on floor 1 (z=7)

  Core::Pos2d idx = Core::Pos2d( 0, 0 ) + ( off + offset );

  if ( idx.x() == 0 || idx.y() == 0 ||
       idx.y() ==
           size.y() )  // don't replace dirt at far-west and far-north sides, check height for y + 1
    return;

  bool floor_exists = false;

  if ( !ValidLocation( idx ) )
    return;
  HouseFloorZColumn* column = &Elements[floor_num].data.at( idx.x() ).at( idx.y() );
  for ( HouseFloorZColumn::iterator itr = column->begin(), itrend = column->end(); itr != itrend;
        ++itr )
  {
    if ( Plib::tileheight( itr->graphic ) == 0 )  // a floor tile exists
    {
      floor_exists = true;
      break;
    }
  }

  if ( floor_exists == false )  // add a dirt tile
  {
    CUSTOM_HOUSE_ELEMENT elem;
    elem.graphic = DIRTY_TILE;
    elem.offset = off;
    elem.z = 7;

    Add( elem );
  }
}

void CustomHouseDesign::Clear()
{
  // delete contents of all z column lists
  for ( int i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
  {
    for ( HouseFloor::iterator xitr = Elements[i].data.begin(), xitrend = Elements[i].data.end();
          xitr != xitrend; ++xitr )
    {
      for ( HouseFloorRow::iterator yitr = xitr->begin(), yitrend = xitr->end(); yitr != yitrend;
            ++yitr )
      {
        yitr->clear();
      }
    }
    floor_sizes[i] = 0;
  }
}

// caller must delete, assume type 0
unsigned char* CustomHouseDesign::Compress( int floor, u32* uncompr_length, u32* compr_length )
{
  int numtiles = floor_sizes[floor];
  int nextindex = 0;
  unsigned int ubuflen = numtiles * BYTES_PER_TILE;
  unsigned long cbuflen =
      ( ( (unsigned long)( ( (float)( ubuflen ) ) * 1.001f ) ) + 12 );  // as per zlib spec
  unsigned char* uncompressed = new unsigned char[ubuflen];
  memset( uncompressed, 0, ubuflen );
  unsigned char* compressed = new unsigned char[cbuflen];
  memset( compressed, 0, cbuflen );

  int i = 0;
  for ( HouseFloor::const_iterator xitr = Elements[floor].data.begin(),
                                   xitrend = Elements[floor].data.end();
        xitr != xitrend; ++xitr )
  {
    i = 0;
    for ( HouseFloorRow::const_iterator yitr = xitr->begin(), yitrend = xitr->end();
          yitr != yitrend; ++yitr )
    {
      for ( HouseFloorZColumn::const_iterator zitr = yitr->begin(), zitrend = yitr->end();
            zitr != zitrend; ++zitr, ++i )
      {
        // assume type 0, I don't know how to deal with stair pieces at odd Z values for mode 1,
        // and mode 2 is just wacky. (position implied from list position, needs alot of null tiles
        // to make that work (but they compress very well)
        if ( i < numtiles )
        {
          uncompressed[nextindex++] = ( u8 )( ( zitr->graphic >> 8 ) & 0xFF );
          uncompressed[nextindex++] = ( u8 )( zitr->graphic & 0xFF );

          uncompressed[nextindex++] = (u8)zitr->offset.x();
          uncompressed[nextindex++] = (u8)zitr->offset.y();
          uncompressed[nextindex++] = (u8)zitr->z;
        }
      }
    }
  }
  *uncompr_length = nextindex;

  int ret = compress2( compressed, &cbuflen, uncompressed, nextindex, Z_DEFAULT_COMPRESSION );
  if ( ret == Z_OK )
  {
    delete[] uncompressed;
    *compr_length = cbuflen;
    return compressed;
  }
  else
  {
    *uncompr_length = 0;
    *compr_length = 0;
    delete[] compressed;
    delete[] uncompressed;
    return nullptr;
  }
}

bool CustomHouseDesign::IsEmpty() const
{
  int total = 0;
  for ( int i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
    total += floor_sizes[i];
  return total == 0 ? true : false;
}

unsigned int CustomHouseDesign::TotalSize() const
{
  unsigned int size = 0;
  for ( int i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
  {
    size += floor_sizes[i];
  }
  return size;
}

unsigned char CustomHouseDesign::NumUsedPlanes() const
{
  unsigned char size = 0;
  for ( int i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
  {
    if ( floor_sizes[i] > 0 )
      size++;
  }
  return size;
}

// used to add the base foundation to initialize a design, and multi stairs. not tested with
// anything else.
// I guess you could add a boat or something inside a house, but deleting would be impossible.
// Deleting stairs is handled explicitly (CustomHouseDesign::DeleteStairs).
void CustomHouseDesign::AddMultiAtOffset( u16 multiid, const Core::Vec3d& off )
{
  const MultiDef* multidef = MultiDefByMultiID( multiid );
  if ( multidef == nullptr )
  {
    ERROR_PRINT << "Trying to add Multi to customhouse, multiid 0x" << fmt::hexu( multiid )
                << " multi definition doesn't exist!\n";
    return;
  }

  for ( MultiDef::Components::const_iterator itr = multidef->components.begin(),
                                             end = multidef->components.end();
        itr != end; ++itr )
  {
    const MULTI_ELEM* m_elem = itr->second;
    if ( ( ( m_elem->objtype ) & Plib::systemstate.config.max_tile_id ) ==
         1 )  // don't add the invisible multi tile
      continue;
    // cout << "0x" << hex << m_elem->graphic
    //     << " 0x" << hex << m_elem->flags
    //     << ":" << dec <<  m_elem->x << "," << m_elem->y << "," << m_elem->z << endl;
    CUSTOM_HOUSE_ELEMENT ch_elem;
    ch_elem.graphic = m_elem->objtype;
    ch_elem.offset = m_elem->rel_pos.xy() + off.xy();
    ch_elem.z = static_cast<u8>( m_elem->rel_pos.z() + off.z() );
    Add( ch_elem );
  }
}

void CustomHouseDesign::readProperties( Clib::ConfigElem& elem, const std::string& prefix )
{
  std::string line;

  while ( elem.remove_prop( prefix.c_str(), &line ) )
  {
    ISTRINGSTREAM is( line );
    u16 graphic;
    s32 x, y;
    u16 z;
    is >> graphic;
    is >> x;
    is >> y;
    is >> z;

    CUSTOM_HOUSE_ELEMENT _elem;
    _elem.graphic = graphic;
    _elem.offset = Core::Vec2d( Core::Vec2d::clip( x ), Core::Vec2d::clip( y ) );
    _elem.z = (u8)z;
    Add( _elem );
  }
}

void CustomHouseDesign::printProperties( Clib::StreamWriter& sw, const std::string& prefix ) const
{
  if ( !IsEmpty() )
  {
    for ( int i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
    {
      for ( HouseFloor::const_iterator xitr = Elements[i].data.begin(),
                                       xitrend = Elements[i].data.end();
            xitr != xitrend; ++xitr )
      {
        for ( HouseFloorRow::const_iterator yitr = xitr->begin(), yitrend = xitr->end();
              yitr != yitrend; ++yitr )
        {
          for ( HouseFloorZColumn::const_iterator zitr = yitr->begin(), zitrend = yitr->end();
                zitr != zitrend; ++zitr )
          {
            sw() << "\t" << prefix << "\t " << zitr->graphic << " " << zitr->offset.x() << " "
                 << zitr->offset.y() << " " << (u16)zitr->z << '\n';
          }
        }
      }
    }
  }
}

// for testing, prints each floor's x,y,z rows
void CustomHouseDesign::testprint( std::ostream& os ) const
{
  if ( !IsEmpty() )
  {
    for ( int i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
    {
      int x = 0, y = 0;
      for ( HouseFloor::const_iterator xitr = Elements[i].data.begin(),
                                       xitrend = Elements[i].data.end();
            xitr != xitrend; ++xitr, x++ )
      {
        os << "X: " << x << std::endl;
        for ( HouseFloorRow::const_iterator yitr = xitr->begin(), yitrend = xitr->end();
              yitr != yitrend; ++yitr, y++ )
        {
          os << "\tY: " << y << std::endl;
          for ( HouseFloorZColumn::const_iterator zitr = yitr->begin(), zitrend = yitr->end();
                zitr != zitrend; ++zitr )
          {
            os << "\t\t" << zitr->graphic << " " << zitr->offset.x() << " " << zitr->offset.y()
               << " " << (u16)zitr->z << std::endl;
          }
        }
      }
    }
  }
}

/// Tells wether an item should be show in custom house design or not
bool CustomHouseDesign::isEditableItem( UHouse* house, Items::Item* item )
{
  // Give scripters the chance to keep an item alive
  if ( item->invisible() )
    return false;

  // Some items could be part of the house, but not inside the house (e.g. an exterior lamp post)
  // hide them to avoid an exception later, since this is not supported
  // only test foodprint: find_supporting_multis would also work, as long as readshapes includes the
  // teleporter components
  Core::Vec2d shape = item->pos().xy() - house->pos().xy();
  shape += offset;
  // y is +1
  if ( shape >= Core::Vec2d( 0, 0 ) && shape < ( size - Core::Vec2d( 0, -1 ) ).from_origin() )
    return true;
  return false;
}

void CustomHouseDesign::ClearComponents( UHouse* house )
{
  UHouse::Components* comp = house->get_components();
  UHouse::Components::iterator itr = comp->begin();

  while ( itr != comp->end() )
  {
    Items::Item* item = ( *itr ).get();
    if ( item != nullptr && !item->orphan() )
    {
      if ( isEditableItem( house, item ) )
      {
        itr = comp->erase( itr );
        destroy_item( item );
        continue;
      }
    }
    ++itr;
  }
}

void CustomHouseDesign::AddComponents( UHouse* house )
{
  UHouse::Components* comp = house->get_components();
  for ( UHouse::Components::const_iterator itr = comp->begin(), end = comp->end(); itr != end;
        ++itr )
  {
    Items::Item* item = ( *itr ).get();
    if ( item != nullptr && !item->orphan() )
    {
      if ( isEditableItem( house, item ) )  // give scripters the chance to keep an item alive
      {
        CUSTOM_HOUSE_ELEMENT elem;
        elem.graphic = item->graphic;
        Core::Vec3d offset = item->pos().xyz() - house->pos().xyz();
        elem.offset = offset.xy();
        elem.z = offset.z();
        AddOrReplace( elem );  // A teleporter could replace a floortile
      }
    }
  }
}
void CustomHouseDesign::FillComponents( UHouse* house, bool add_as_component )
{
  UHouse::Components* comp = house->get_components();
  for ( int i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
  {
    for ( HouseFloor::iterator xitr = Elements[i].data.begin(), xitrend = Elements[i].data.end();
          xitr != xitrend; ++xitr )
    {
      for ( HouseFloorRow::iterator yitr = xitr->begin(), yitrend = xitr->end(); yitr != yitrend;
            ++yitr )
      {
        HouseFloorZColumn::iterator zitr = yitr->begin();
        while ( zitr != yitr->end() )
        {
          const Items::ItemDesc& id = Items::find_itemdesc( zitr->graphic );
          if ( id.type == Items::ItemDesc::DOORDESC )
          {
            if ( add_as_component )
            {
              Items::Item* component = Items::Item::create( id.objtype );
              if ( component != nullptr )
              {
                bool res = house->add_component( component, Core::Vec3d( zitr->offset, zitr->z ) );
                passert_always_r( res,
                                  "Couldn't add newly created door as house component. Please "
                                  "report this bug on the forums." );
              }
            }
            else
            {
              Core::Pos4d cpos = house->pos() + Core::Vec3d( zitr->offset, zitr->z );
              // if component already exists erase from design, otherwise keep it
              bool exists = false;
              for ( const auto& c : *comp )
              {
                Items::Item* item = c.get();
                if ( item == nullptr || item->orphan() )
                  continue;
                if ( cpos == item->pos() && zitr->graphic == item->graphic )
                {
                  exists = true;
                  break;
                }
              }
              if ( !exists )
              {
                ++zitr;
                continue;
              }
            }
            zitr = yitr->erase( zitr );
            floor_sizes[i]--;
          }
          else if ( zitr->graphic >= TELEPORTER_START &&
                    zitr->graphic <= TELEPORTER_END )  // teleporters
          {
            if ( add_as_component )
            {
              Items::Item* component = Items::Item::create( zitr->graphic );
              if ( component != nullptr )
              {
                bool res = house->add_component( component, Core::Vec3d( zitr->offset, zitr->z ) );
                passert_always_r( res,
                                  "Couldn't add newly created teleporter as house component. "
                                  "Please report this bug on the forums." );
              }
            }
            else
            {
              Core::Pos4d cpos = house->pos() + Core::Vec3d( zitr->offset, zitr->z );
              // if component already exists erase from design, otherwise keep it
              bool exists = false;
              for ( const auto& c : *comp )
              {
                Items::Item* item = c.get();
                if ( item == nullptr || item->orphan() )
                  continue;
                if ( cpos == item->pos() && zitr->graphic == item->graphic )
                {
                  exists = true;
                  break;
                }
              }
              if ( !exists )
              {
                ++zitr;
                continue;
              }
            }
            zitr = yitr->erase( zitr );
            floor_sizes[i]--;
          }
          else
            ++zitr;
        }
      }
    }
  }
}

Bscript::ObjArray* CustomHouseDesign::list_parts() const
{
  std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray );
  for ( int i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
  {
    for ( HouseFloor::const_iterator xitr = Elements[i].data.begin(),
                                     xitrend = Elements[i].data.end();
          xitr != xitrend; ++xitr )
    {
      for ( HouseFloorRow::const_iterator yitr = xitr->begin(), yitrend = xitr->end();
            yitr != yitrend; ++yitr )
      {
        for ( HouseFloorZColumn::const_iterator zitr = yitr->begin(), zitrend = yitr->end();
              zitr != zitrend; ++zitr )
        {
          std::unique_ptr<Bscript::BStruct> itemstruct( new Bscript::BStruct );
          itemstruct->addMember( "graphic", new Bscript::BLong( zitr->graphic ) );
          itemstruct->addMember( "xoffset", new Bscript::BLong( zitr->offset.x() ) );
          itemstruct->addMember( "yoffset", new Bscript::BLong( zitr->offset.y() ) );
          itemstruct->addMember( "z", new Bscript::BLong( zitr->z ) );
          arr->addElement( itemstruct.release() );
        }
      }
    }
  }
  return arr.release();
}

void CustomHouseStopEditing( Mobile::Character* chr, UHouse* house, bool send_pkts )
{
  if ( send_pkts && chr->client )
  {
    Network::PktHelper::PacketOut<Network::PktOut_BF_Sub20> msg;
    msg->WriteFlipped<u16>( 17u );
    msg->offset += 2;  // sub
    msg->Write<u32>( house->serial_ext );
    msg->Write<u8>( 0x5u );          // end
    msg->offset += 2;                // u16 unk2 FIXME what's the meaning
    msg->Write<u32>( 0xFFFFFFFFu );  // fixme
    msg->Write<u8>( 0xFFu );         // fixme
    msg.Send( chr->client );
  }
  const MultiDef& def = house->multidef();
  Core::Pos4d newpos( house->pos() + Core::Vec2d( def.min_relp.x(), def.max_relp.y() + 1 ) );
  move_character_to( chr, newpos, Core::MOVEITEM_FORCELOCATION );
  if ( chr->client )
  {
    chr->client->gd->custom_house_serial = 0;
    chr->client->gd->custom_house_chrserial = 0;
  }
  house->editing = false;
  if ( send_pkts && chr->client )
  {
    ItemList itemlist;
    MobileList moblist;
    UHouse::list_contents( house, itemlist, moblist );
    while ( !itemlist.empty() )
    {
      Items::Item* item = itemlist.front();
      send_item( chr->client, item );
      itemlist.pop_front();
    }
  }
}

void CustomHousesAdd( Core::PKTBI_D7* msg )
{
  u32 serial = cfBEu32( msg->serial );
  UHouse* house = UHouse::FindWorkingHouse( serial );
  if ( house == nullptr )
    return;

  Core::CH_ADD add = msg->ch_add;
  CUSTOM_HOUSE_ELEMENT elem;
  elem.graphic = cfBEu16( add.tileID );
  elem.offset = Core::Vec2d( Core::Vec2d::clip( cfBEu32( add.xoffset ) ),
                             Core::Vec2d::clip( cfBEu32( add.yoffset ) ) );
  elem.z = CustomHouseDesign::custom_house_z_xlate_table[house->editing_floor_num];

  // the south side of the house can have stairs at z=0
  // int ysize = house->multidef().maxry - house->multidef().minry;

  if ( elem.offset.y() == house->multidef().max_relp.y() + 1 )
    elem.z = 0;

  house->WorkingDesign.AddOrReplace( elem );

  // invalidate stored packet
  std::vector<u8> newvec;
  house->WorkingCompressed.swap( newvec );

  house->revision++;
}

void CustomHousesAddMulti( Core::PKTBI_D7* msg )
{
  u32 serial = cfBEu32( msg->serial );
  UHouse* house = UHouse::FindWorkingHouse( serial );
  if ( house == nullptr )
    return;

  u16 itemID = cfBEu16( msg->ch_add_multi.multiID );
  Core::Vec3d off( static_cast<s8>( cfBEu32( msg->ch_add_multi.xoffset ) ),
                   static_cast<s8>( cfBEu32( msg->ch_add_multi.yoffset ) ),
                   CustomHouseDesign::custom_house_z_xlate_table[house->editing_floor_num] );

  // only allow stairs IDs
  if ( itemID < STAIR_MULTIID_MIN || itemID > STAIR_MULTIID_MAX )
  {
    Mobile::Character* chr = Core::find_character( serial );
    if ( chr != nullptr && chr->client != nullptr )
      CustomHousesSendFull( house, chr->client, HOUSE_DESIGN_WORKING );
    return;
  }

  house->WorkingDesign.AddMultiAtOffset( itemID, off );

  // invalidate stored packet
  std::vector<u8> newvec;
  house->WorkingCompressed.swap( newvec );

  house->revision++;
}

void CustomHousesErase( Core::PKTBI_D7* msg )
{
  u32 serial = cfBEu32( msg->serial );
  UHouse* house = UHouse::FindWorkingHouse( serial );
  if ( house == nullptr )
    return;

  Core::Vec3d off( Core::Vec2d::clip( cfBEu32( msg->ch_erase.xoffset ) ),
                   Core::Vec2d::clip( cfBEu32( msg->ch_erase.yoffset ) ),
                   static_cast<s8>( cfBEu32( msg->ch_erase.z ) ) );
  u16 graphic = cfBEu16( msg->ch_erase.tileID );

  Core::Pos3d realpos = Core::Pos3d( 0, 0, 0 ) + off + house->WorkingDesign.offset;

  // foundation walls should not be deleted
  if ( realpos.z() == 0 && realpos < ( house->WorkingDesign.size - Core::Vec2d( 0, 1 ) ) )
  {
    Mobile::Character* chr = Core::find_character( serial );
    if ( chr && chr->client )
      CustomHousesSendFull( house, chr->client, HOUSE_DESIGN_WORKING );
    return;
  }

  // check if not deleting a stairs piece (if we are, DeleteStairs will do it)
  // check z == 0 to make sure all exterior stairs are deleted with EraseGraphicAt
  if ( off.z() == 0 || !house->WorkingDesign.DeleteStairs( graphic, off ) )
  {
    house->WorkingDesign.EraseGraphicAt( graphic, off.xy(), off.z() );
    // maybe replace empty ground floor with dirt tile
    if ( off.z() == CustomHouseDesign::custom_house_z_xlate_table[1] )
      house->WorkingDesign.ReplaceDirtFloor( off.xy() );
  }

  // invalidate stored packet
  std::vector<u8> newvec;
  house->WorkingCompressed.swap( newvec );

  Mobile::Character* chr = Core::find_character( serial );
  if ( chr && chr->client )
    CustomHousesSendFull( house, chr->client, HOUSE_DESIGN_WORKING );

  house->revision++;
}

void CustomHousesClear( Core::PKTBI_D7* msg )
{
  u32 serial = cfBEu32( msg->serial );
  Mobile::Character* chr = Core::find_character( serial );
  UHouse* house = UHouse::FindWorkingHouse( serial );
  if ( house == nullptr )
    return;

  house->WorkingDesign.Clear();

  // invalidate stored packet
  std::vector<u8> newvec;
  house->WorkingCompressed.swap( newvec );

  // add foundation back to design
  house->WorkingDesign.AddMultiAtOffset( house->multiid, Core::Vec3d( 0, 0, 0 ) );
  if ( chr != nullptr && chr->client != nullptr )
    CustomHousesSendFull( house, chr->client, HOUSE_DESIGN_WORKING );

  house->revision++;
}

// if the client closed his tool
void CustomHousesQuit( Core::PKTBI_D7* msg )
{
  u32 serial = cfBEu32( msg->serial );
  UHouse* house = UHouse::FindWorkingHouse( serial );
  if ( house == nullptr )
    return;
  Mobile::Character* chr = Core::find_character( serial );
  house->CustomHousesQuit( chr, false );
}

void CustomHousesCommit( Core::PKTBI_D7* msg )
{
  u32 serial = cfBEu32( msg->serial );
  UHouse* house = UHouse::FindWorkingHouse( serial );
  Mobile::Character* chr = Core::find_character( serial );
  if ( house == nullptr || chr == nullptr )
    return;

  if ( house->waiting_for_accept )  // dont allow multiple commits
    return;

  // remove dynamic bits (teleporters, doors)
  house->WorkingDesign.FillComponents( house );

  // call a script to do post processing (calc cost, yes/no confirm, consume cost, link teleporters)
  Core::ScriptDef sd;
  if ( sd.config_nodie( "misc/customhousecommit.ecl", nullptr, nullptr ) )
  {
    if ( sd.exists() )
    {
      house->waiting_for_accept = true;
      // former part list as param is now possible via house member
      if ( Core::start_script( sd, make_mobileref( chr ), new Module::EMultiRefObjImp( house ) ) !=
           nullptr )
        return;
    }
  }
  house->AcceptHouseCommit( chr, true );
}

void CustomHousesSelectFloor( Core::PKTBI_D7* msg )
{
  u32 serial = cfBEu32( msg->serial );
  UHouse* house = UHouse::FindWorkingHouse( serial );
  Mobile::Character* chr = Core::find_character( serial );
  if ( house == nullptr )
    return;
  u8 floor = msg->ch_select_floor.floornumber;

  if ( floor < 1 || floor > 4 )
    floor = 1;

  house->editing_floor_num = floor;

  if ( chr )
  {
    Core::Pos4d newp( chr->pos().xy(),
                      house->z() + CustomHouseDesign::custom_house_z_xlate_table[floor],
                      chr->realm() );
    move_character_to( chr, newp, Core::MOVEITEM_FORCELOCATION );
    if ( chr->client )
      CustomHousesSendFull( house, chr->client, HOUSE_DESIGN_WORKING );
  }
}

void CustomHousesBackup( Core::PKTBI_D7* msg )
{
  u32 serial = cfBEu32( msg->serial );
  UHouse* house = UHouse::FindWorkingHouse( serial );
  if ( house == nullptr )
    return;

  house->BackupDesign = house->WorkingDesign;
}

void CustomHousesRestore( Core::PKTBI_D7* msg )
{
  u32 serial = cfBEu32( msg->serial );
  UHouse* house = UHouse::FindWorkingHouse( serial );
  Mobile::Character* chr = Core::find_character( serial );
  if ( house == nullptr )
    return;

  if ( !house->BackupDesign.IsEmpty() )
    house->WorkingDesign = house->BackupDesign;

  std::vector<u8> newvec;
  house->WorkingCompressed.swap( newvec );
  if ( chr != nullptr && chr->client != nullptr )
    CustomHousesSendFull( house, chr->client, HOUSE_DESIGN_WORKING );
}

void CustomHousesSynch( Core::PKTBI_D7* msg )
{
  u32 serial = cfBEu32( msg->serial );
  Mobile::Character* chr = Core::find_character( serial );
  UHouse* house = UHouse::FindWorkingHouse( serial );
  if ( chr != nullptr && chr->client != nullptr && house != nullptr )
    CustomHousesSendFull( house, chr->client, HOUSE_DESIGN_WORKING );
}

// replace working design with currently committed design
void CustomHousesRevert( Core::PKTBI_D7* msg )
{
  u32 serial = cfBEu32( msg->serial );
  UHouse* house = UHouse::FindWorkingHouse( serial );

  if ( house == nullptr )
    return;

  house->WorkingDesign = house->CurrentDesign;
  std::vector<u8> newvec;
  house->WorkingCompressed.swap( newvec );
  Mobile::Character* chr = Core::find_character( serial );
  if ( chr != nullptr && chr->client != nullptr )
    CustomHousesSendFull( house, chr->client, HOUSE_DESIGN_WORKING );
}

void CustomHousesRoofSelect( Core::PKTBI_D7* msg )
{
  u32 serial = cfBEu32( msg->serial );
  UHouse* house = UHouse::FindWorkingHouse( serial );
  if ( house == nullptr )
    return;

  Core::CH_SELECT_ROOF add = msg->ch_select_roof;
  CUSTOM_HOUSE_ELEMENT elem;
  elem.graphic = cfBEu16( add.tileID );
  elem.offset = Core::Vec2d( Core::Vec2d::clip( cfBEu32( add.xoffset ) ),
                             Core::Vec2d::clip( cfBEu32( add.yoffset ) ) );
  s8 z = static_cast<s8>( cfBEu32( add.zoffset ) );
  if ( z < -3 || z > 12 || z % 3 != 0 )
    z = -3;
  elem.z = z + CustomHouseDesign::custom_house_z_xlate_table[house->editing_floor_num];

  house->WorkingDesign.AddOrReplace( elem );

  // invalidate stored packet
  std::vector<u8> newvec;
  house->WorkingCompressed.swap( newvec );

  house->revision++;
}
void CustomHousesRoofRemove( Core::PKTBI_D7* msg )
{
  u32 serial = cfBEu32( msg->serial );
  UHouse* house = UHouse::FindWorkingHouse( serial );
  if ( house == nullptr )
    return;

  Core::CH_DELETE_ROOF remove = msg->ch_delete_roof;
  Core::Vec2d off( Core::Vec2d::clip( cfBEu32( remove.xoffset ) ),
                   Core::Vec2d::clip( cfBEu32( remove.yoffset ) ) );
  u8 z = static_cast<u8>( cfBEu32( remove.zoffset ) );
  u16 graphic = cfBEu16( remove.tileID );
  if ( !house->WorkingDesign.EraseGraphicAt( graphic, off, z ) )
  {
    Mobile::Character* chr = Core::find_character( serial );
    if ( chr != nullptr && chr->client != nullptr )
      CustomHousesSendFull( house, chr->client, HOUSE_DESIGN_WORKING );
    return;
  }

  // invalidate stored packet
  std::vector<u8> newvec;
  house->WorkingCompressed.swap( newvec );

  house->revision++;
}

void CustomHousesSendFull( UHouse* house, Network::Client* client, int design )
{
  u32 clen;
  u32 ulen;
  unsigned char* data;
  // unsigned char** stored_packet;
  std::vector<u8>* stored_packet;

  u32 planeheader = 0;
  u32 buffer_len = 0;
  CustomHouseDesign* pdesign;
  const unsigned int data_offset = 17;
  const u32 mode = 0;  // we only know how to do mode 0 at this point.

  // choose between sending working or current designs
  switch ( design )
  {
  case HOUSE_DESIGN_CURRENT:
    if ( house->CurrentCompressed.empty() )  // no design stored, create below
    {
      pdesign = &house->CurrentDesign;
      stored_packet = &house->CurrentCompressed;
    }
    else  // send stored packet
    {
      Core::networkManager.clientTransmit->AddToQueue(
          client, &house->CurrentCompressed[0],
          ctBEu16( *( reinterpret_cast<u16*>( &house->CurrentCompressed[1] ) ) ) );
      return;
    }
    break;
  case HOUSE_DESIGN_WORKING:
    if ( house->WorkingCompressed.empty() )  // no design stored, create below
    {
      pdesign = &house->WorkingDesign;
      stored_packet = &house->WorkingCompressed;
    }
    else  // send stored packet
    {
      Core::networkManager.clientTransmit->AddToQueue(
          client, &house->WorkingCompressed[0],
          ctBEu16( *( reinterpret_cast<u16*>( &house->WorkingCompressed[1] ) ) ) );
      return;
    }
    break;
  default:
    return;
  }

  // create compressed house message

  unsigned char planes = pdesign->NumUsedPlanes();
  unsigned long sbuflen =
      ( (unsigned long)( ( (float)( pdesign->TotalSize() * BYTES_PER_TILE ) ) * 1.001f ) + 12 );
  sbuflen += planes * BYTES_PER_TILE;  // for plane header dword
  sbuflen += 17;                       // for packet header

  std::vector<u8> packet( sbuflen );


  Core::PKTOUT_D8* msg = reinterpret_cast<Core::PKTOUT_D8*>( &packet[0] );
  msg->msgtype = Core::PKTOUT_D8_ID;
  msg->compressiontype = 0x3;
  msg->unk = 0;
  msg->serial = house->serial_ext;
  msg->revision = ctBEu32( house->revision );
  msg->numtiles = ctBEu16( static_cast<u16>( pdesign->TotalSize() ) );

  msg->buffer->planecount = planes;
  buffer_len = 1;
  for ( int i = 0; i < planes; i++ )
  {
    planeheader = 0;
    data = pdesign->Compress( i, &ulen, &clen );
    if ( data == nullptr )  // compression error
    {
      return;
    }
    if ( ulen == 0 )
      clen = 0;
    planeheader |= ( ( mode << 4 ) << 24 );
    planeheader |= ( ( i & 0xF ) << 24 );
    planeheader |= ( ( ulen & 0xFF ) << 16 );
    planeheader |= ( ( clen & 0xFF ) << 8 );
    planeheader |= ( ( ( ulen >> 4 ) & 0xF0 ) | ( ( clen >> 8 ) & 0xF ) );
    u32* p_planeheader = reinterpret_cast<u32*>( &( packet[buffer_len + data_offset] ) );
    *p_planeheader = ctBEu32( planeheader );
    buffer_len += 4;
    memcpy( &( packet[buffer_len + data_offset] ), data, clen );
    buffer_len += clen;
    delete[] data;
  }
  msg->msglen = ctBEu16( static_cast<u16>( buffer_len ) + data_offset );
  msg->planebuffer_len = ctBEu16( static_cast<u16>( buffer_len ) );

  Core::networkManager.clientTransmit->AddToQueue( client, &packet[0], cfBEu16( msg->msglen ) );
  stored_packet->swap( packet );
}

void CustomHousesSendFullToInRange( UHouse* house, int design, int range )
{
  Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
      house->pos(), range,
      [&]( Mobile::Character* chr ) { CustomHousesSendFull( house, chr->client, design ); } );
}

void CustomHousesSendShort( UHouse* house, Network::Client* client )
{
  Network::PktHelper::PacketOut<Network::PktOut_BF_Sub1D> msg;
  msg->WriteFlipped<u16>( 13u );
  msg->offset += 2;
  msg->Write<u32>( house->serial_ext );
  msg->WriteFlipped<u32>( house->revision );
  msg.Send( client );
}

void UHouse::SetCustom( bool _custom )
{
  if ( custom == false && _custom == true )
    CustomHouseSetInitialState();

  custom = _custom;
}
void UHouse::CustomHouseSetInitialState()
{
  const MultiDef& def = multidef();
  auto size = Core::Pos2d( 0, 0 ) + ( def.max_relp - def.min_relp ).xy() + Core::Vec2d( 1, 1 );
  //+1 to include offset 0 in -3..3
  size += Core::Vec2d( 0, 1 );  //+1 for front steps outside multidef footprint
  Core::Vec2d xybase( (short)abs( def.min_relp.x() ), (short)abs( def.min_relp.y() ) );
  CurrentDesign.Clear();
  CurrentDesign.InitDesign( size, xybase );
  WorkingDesign.Clear();
  WorkingDesign.InitDesign( size, xybase );
  BackupDesign.Clear();
  BackupDesign.InitDesign( size, xybase );

  CurrentDesign.AddMultiAtOffset( multiid, Core::Vec3d( 0, 0, 0 ) );
  WorkingDesign = CurrentDesign;
  BackupDesign = CurrentDesign;
  std::vector<u8> newvec;
  WorkingCompressed.swap( newvec );

  std::vector<u8> newvec2;
  CurrentCompressed.swap( newvec2 );
}

void UHouse::CustomHousesQuit( Mobile::Character* chr, bool drop_changes, bool send_pkts )
{
  if ( drop_changes )
    WorkingDesign = CurrentDesign;
  else
  {
    CurrentDesign.FillComponents( this );
    WorkingDesign.FillComponents( this, false );  // keep in sync
  }
  revision++;
  std::vector<u8> newvec;
  WorkingCompressed.swap( newvec );

  std::vector<u8> newvec2;
  CurrentCompressed.swap( newvec2 );
  if ( chr )
  {
    CustomHouseStopEditing( chr, this, send_pkts );
    if ( chr->client && send_pkts )
      CustomHousesSendFull( this, chr->client, HOUSE_DESIGN_CURRENT );
    if ( Core::gamestate.system_hooks.close_customhouse_hook )
    {
      Core::gamestate.system_hooks.close_customhouse_hook->call(
          send_pkts ? make_mobileref( chr ) : new Module::EOfflineCharacterRefObjImp( chr ),
          new Module::EMultiRefObjImp( this ) );
    }
  }
}
}  // namespace Multi
}  // namespace Pol
