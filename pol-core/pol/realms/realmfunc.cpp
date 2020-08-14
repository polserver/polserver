/** @file
 *
 * @par History
 * - 2005/06/01 Shinigami: added getmapshapes - to get access to mapshapes
 *                         added getstatics - to fill a list with statics
 * - 2005/06/06 Shinigami: added readmultis derivative - to get a list of statics
 * - 2005/09/03 Shinigami: fixed: Realm::walkheight ignored param doors_block
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 * - 2009/11/13 Turley:    Rewrote MovementCode
 * - 2009/12/03 Turley:    basic gargoyle fly support
 *
 * @note In standingheight checks there is a nasty bug. Items NOT locked down
 * (unmovable), are NOT being checked correctly to block movement.
 */


#include <stddef.h>

#include "clib/rawtypes.h"
#include "plib/clidata.h"
#include "plib/mapcell.h"
#include "plib/mapserver.h"
#include "plib/mapshape.h"
#include "plib/maptile.h"
#include "plib/maptileserver.h"
#include "plib/poltype.h"
#include "plib/staticserver.h"
#include "plib/systemstate.h"
#include "plib/tiles.h"
#include "plib/uconst.h"
#include "plib/udatfile.h"

#include "fnsearch.h"
#include "globals/uvars.h"
#include "item/itemdesc.h"
#include "landtile.h"
#include "mobile/charactr.h"
#include "multi/customhouses.h"
#include "multi/house.h"
#include "multi/multi.h"
#include "multi/multidef.h"
#include "network/cgdata.h"
#include "network/client.h"
#include "objtype.h"
#include "realms/realm.h"
#include "uworld.h"



#define HULL_HEIGHT_BUFFER 2
namespace Pol
{
namespace Core
{
Items::Item* find_walkon_item( Core::ItemsVector& ivec, short z );
unsigned char flags_from_tileflags( unsigned int uoflags );
}  // namespace Core
// namespace Multi {
// class UMulti;
// UMulti* find_supporting_multi( Realms::MultiList& mvec, short z );
// }

namespace Realms
{
bool Realm::lowest_standheight( unsigned short x, unsigned short y, short* z ) const
{
  static Plib::MapShapeList vec;
  vec.clear();
  getmapshapes(
      vec, x, y,
      Plib::FLAG::MOVELAND | Plib::FLAG::MOVESEA | Plib::FLAG::BLOCKING | Plib::FLAG::GRADUAL );

  bool res = true;
  lowest_standheight( Plib::MOVEMODE_LAND, vec, *z, &res, z );
  return res;
}

void Realm::standheight( Plib::MOVEMODE movemode, Plib::MapShapeList& shapes, short oldz,
                         bool* result_out, short* newz_out, short* gradual_boost )
{
  static std::vector<const Plib::MapShape*> possible_shapes;
  possible_shapes.clear();
  bool land_ok = ( movemode & Plib::MOVEMODE_LAND ) ? true : false;
  bool sea_ok = ( movemode & Plib::MOVEMODE_SEA ) ? true : false;
  bool fly_ok = ( movemode & Plib::MOVEMODE_FLY ) ? true : false;
  short the_boost = 0;
  short new_boost = 0;

  if ( gradual_boost != nullptr )
    the_boost = *gradual_boost;
  if ( the_boost < 5 )
    the_boost = 5;

  short newz = -200;

  // first check only possible walkon shapes and build a list
  for ( const auto& shape : shapes )
  {
    unsigned int flags = shape.flags;
    short ztop = shape.z + shape.height;
#if ENABLE_POLTEST_OUTPUT
    if ( static_debug_on )
    {
      INFO_PRINT << "static: graphic=0x" << fmt::hexu( srec.graphic ) << ", z=" << int( srec.z )
                 << ", ht=" << int( srec.height ) << "\n";
    }
#endif

    if ( ( land_ok && ( flags & Plib::FLAG::MOVELAND ) ) ||
         ( sea_ok && ( flags & Plib::FLAG::MOVESEA ) ) ||
         ( fly_ok && ( flags & Plib::FLAG::OVERFLIGHT ) ) )
    {
      if ( ( ( ztop <= oldz + 2 + the_boost ) ||
             ( ( flags & Plib::FLAG::GRADUAL ) &&
               ( ztop <= oldz + 15 ) )  // not too high to step onto
             || ( fly_ok && ( flags & Plib::FLAG::OVERFLIGHT ) &&
                  ( ztop <= oldz + 20 ) ) ) &&  // seems that flying allows higher steps
           ( ztop >= newz ) )                   // but above or same as the highest yet seen
      {
#if ENABLE_POLTEST_OUTPUT
        if ( static_debug_on )
          INFO_PRINT << "Setting Z to " << int( ztemp ) << "\n";
#endif
        possible_shapes.push_back( &shape );
        newz = ztop;
      }
    }
    if ( newz < shape.z &&
         shape.z < newz + Core::settingsManager.ssopt
                              .default_character_height )  // space too small to stand?
    {
      if ( !possible_shapes.empty() )
        possible_shapes.pop_back();  // remove the last pos_shape
    }
  }

  bool ret_result = false;
  short ret_newz = -200;
  if ( !possible_shapes.empty() )
  {
    // loop through all possible shapes and test if other shape blocks
    for ( const auto& pos_shape : possible_shapes )
    {
      bool result = true;
      newz = pos_shape->z + pos_shape->height;
      for ( const auto& shape : shapes )
      {
        if ( ( shape.flags &
               ( Plib::FLAG::MOVELAND | Plib::FLAG::MOVESEA | Plib::FLAG::BLOCKING ) ) == 0 )
          continue;
        int shape_top = shape.z + shape.height;

        unsigned char default_character_height =
            Core::settingsManager.ssopt.default_character_height;
        if ( ( newz < shape.z &&
               //        Must be left lower than 15 height like
               //        other checks, it will block char from walking down
               //        ladders in homes/dungeons if over 9.
               shape.z <=
                   oldz + ( default_character_height < 9 ? default_character_height : 9 ) ) ||
             ( shape.z < newz + default_character_height && shape_top > newz ) )
        {
#if ENABLE_POLTEST_OUTPUT
          if ( static_debug_on )
          {
            INFO_PRINT << "static: objtype=0x" << fmt::hexu( srec.graphic )
                       << ", z=" << int( srec.z ) << ", ht=" << int( srec.height )
                       << " blocks movement to z=" << int( newz ) << "\n";
          }
#endif

          result = false;
          break;
        }
      }
      if ( result )
      {
        // if something was already found use the smallest step diff
        if ( ( ret_result ) && ( labs( oldz - ret_newz ) < labs( oldz - newz ) ) )
          continue;
        ret_result = true;
        ret_newz = newz;
        if ( pos_shape->flags & Plib::FLAG::GRADUAL )
          new_boost = pos_shape->height;
      }
    }
  }

  *result_out = ret_result;
  *newz_out = ret_newz;
  if ( ret_result && ( gradual_boost != nullptr ) )
  {
    if ( new_boost > 11 )
      *gradual_boost = 11;
    else
      *gradual_boost = new_boost;
  }
}


void Realm::lowest_standheight( Plib::MOVEMODE movemode, Plib::MapShapeList& shapes, short minz,
                                bool* result_out, short* newz_out, short* gradual_boost )
{
  bool land_ok = ( movemode & Plib::MOVEMODE_LAND ) ? true : false;
  bool sea_ok = ( movemode & Plib::MOVEMODE_SEA ) ? true : false;
  bool fly_ok = ( movemode & Plib::MOVEMODE_FLY ) ? true : false;
  short the_boost = 0;
  short new_boost = 0;

  if ( gradual_boost != nullptr )
    the_boost = *gradual_boost;

  if ( shapes.size() == 1 )  // map only
  {
    const Plib::MapShape& shape = shapes.at( 0 );
    if ( land_ok && ( shape.flags & ( Plib::FLAG::MOVELAND ) ) )
    {
      *result_out = true;
      *newz_out = shape.z + shape.height;
      return;
    }
  }


  short newz = 255;
  bool result = false;

  for ( const auto& shape : shapes )
  {
    unsigned int flags = shape.flags;
    short ztop = shape.z + shape.height;
#if ENABLE_POLTEST_OUTPUT
    if ( static_debug_on )
    {
      INFO_PRINT << "static: graphic=0x" << fmt::hexu( srec.graphic ) << ", z=" << int( srec.z )
                 << ", ht=" << int( srec.height ) << "\n";
    }
#endif

    if ( the_boost < 5 )
      the_boost = 5;

    if ( ( land_ok && ( flags & Plib::FLAG::MOVELAND ) ) ||
         ( sea_ok && ( flags & Plib::FLAG::MOVESEA ) ) ||
         ( fly_ok && ( flags & Plib::FLAG::OVERFLIGHT ) ) )
    {
      if ( ( ztop >= minz ) &&  // higher than our base
           ( ztop < newz ) )    // lower than we've seen before
      {
#if ENABLE_POLTEST_OUTPUT
        if ( static_debug_on )
          INFO_PRINT << "Setting Z to " << int( ztemp ) << "\n";
#endif
        bool valid = true;
        // validate that its actually standable
        for ( const auto& shapeCheck : shapes )
        {
          if ( ( shapeCheck.flags &
                 ( Plib::FLAG::MOVELAND | Plib::FLAG::MOVESEA | Plib::FLAG::BLOCKING ) ) == 0 )
            continue;

          unsigned char default_character_height =
              Core::settingsManager.ssopt.default_character_height;
          if ( ( ( ztop < shapeCheck.z ) &&  // if the check-shape is higher than the location
                 ( shapeCheck.z < ztop + ( default_character_height < 9
                                               ? default_character_height
                                               : 9 ) ) )  // but low enough to block standing (9
                                                          // instead of 15 for ladders)
               ||                                         // OR
               ( ( shapeCheck.z <
                   ztop + default_character_height ) &&  // if the check-shape is sitting below the
                                                         // standing space of my new location
                 ( shapeCheck.z + shapeCheck.height >
                   ztop ) ) )  // and the top of the object is above my new location

          {
#if ENABLE_POLTEST_OUTPUT
            if ( static_debug_on )
            {
              INFO_PRINT << "static: objtype=0x" << fmt::hexu( srec.graphic )
                         << ", z=" << int( srec.z ) << ", ht=" << int( srec.height )
                         << " blocks movement to z=" << int( newz ) << "\n";
            }
#endif
            valid = false;
            break;
          }
        }

        // its the lowest and its legal
        if ( valid )
        {
          newz = ztop;
          result = true;
          if ( flags & Plib::FLAG::GRADUAL )
            new_boost = shape.height;
        }
      }
    }
  }

  *result_out = result;
  *newz_out = newz;
  if ( result && ( gradual_boost != nullptr ) )
  {
    if ( new_boost > 11 )
      *gradual_boost = 11;
    else
      *gradual_boost = new_boost;
  }
}


void Realm::readdynamics( Plib::MapShapeList& vec, unsigned short x, unsigned short y,
                          Core::ItemsVector& walkon_items, bool doors_block )
{
  Core::ZoneItems& witems = Core::getzone( x, y, this ).items;
  for ( const auto& item : witems )
  {
    if ( ( item->x == x ) && ( item->y == y ) )
    {
      if ( Plib::tile_flags( item->graphic ) & Plib::FLAG::WALKBLOCK )
      {
        if ( doors_block || item->itemdesc().type != Items::ItemDesc::DOORDESC )
        {
          Plib::MapShape shape;
          shape.z = item->z;
          shape.height = item->height;
          shape.flags = Plib::systemstate.tile[item->graphic].flags;
          vec.push_back( shape );
        }
      }

      if ( !item->itemdesc().walk_on_script.empty() )
      {
        walkon_items.push_back( item );
      }
    }
  }
}


// new Z given new X, Y, and old Z.
bool Realm::walkheight( unsigned short x, unsigned short y, short oldz, short* newz,
                        Multi::UMulti** pmulti, Items::Item** pwalkon, bool doors_block,
                        Plib::MOVEMODE movemode, short* gradual_boost )
{
  if ( x >= width() || y >= height() )
  {
    return false;
  }

  static Plib::MapShapeList shapes;
  static MultiList mvec;
  static Core::ItemsVector walkon_items;
  shapes.clear();
  mvec.clear();
  walkon_items.clear();

  readdynamics( shapes, x, y, walkon_items, doors_block /* true */ );
  unsigned int flags = Plib::FLAG::MOVE_FLAGS;
  if ( movemode & Plib::MOVEMODE_FLY )
    flags |= Plib::FLAG::OVERFLIGHT;
  readmultis( shapes, x, y, flags, mvec );
  getmapshapes( shapes, x, y, flags );

  bool result;
  standheight( movemode, shapes, oldz, &result, newz, gradual_boost );

  if ( result && ( pwalkon != nullptr ) )
  {
    if ( !mvec.empty() )
    {
      *pmulti = find_supporting_multi( mvec, *newz );
    }
    else
    {
      *pmulti = nullptr;
    }

    if ( !walkon_items.empty() )
    {
      *pwalkon = Core::find_walkon_item( walkon_items, *newz );
    }
    else
    {
      *pwalkon = nullptr;
    }
  }

  return result;
}

// new Z given new X, Y, and old Z.
// dave: todo: return false if walking onto a custom house and not in the list of editing players,
// and no cmdlevel
bool Realm::walkheight( const Mobile::Character* chr, unsigned short x, unsigned short y,
                        short oldz, short* newz, Multi::UMulti** pmulti, Items::Item** pwalkon,
                        short* gradual_boost )
{
  if ( x >= width() || y >= height() )
  {
    return false;
  }

  static Plib::MapShapeList shapes;
  static MultiList mvec;
  static Core::ItemsVector walkon_items;
  shapes.clear();
  mvec.clear();
  walkon_items.clear();

  readdynamics( shapes, x, y, walkon_items, chr->doors_block() );
  unsigned int flags = Plib::FLAG::MOVE_FLAGS;
  if ( chr->movemode & Plib::MOVEMODE_FLY )
    flags |= Plib::FLAG::OVERFLIGHT;
  readmultis( shapes, x, y, flags, mvec );
  getmapshapes( shapes, x, y, flags );

  bool result;
  standheight( chr->movemode, shapes, oldz, &result, newz, gradual_boost );

  if ( result && ( pwalkon != nullptr ) )
  {
    if ( !mvec.empty() )
    {
      *pmulti = find_supporting_multi( mvec, *newz );
      if ( *pmulti != nullptr )
      {
        Multi::UHouse* house = ( *pmulti )->as_house();
        if ( house && house->editing && chr->client &&
             ( chr->client->gd->custom_house_serial != house->serial ) )
          result = false;  // sendsysmessage to client ?
      }
    }
    else
    {
      *pmulti = nullptr;
      if ( chr->is_house_editing() )
      {
        Multi::UMulti* multi = Core::system_find_multi( chr->client->gd->custom_house_serial );
        if ( multi != nullptr )
        {
          Multi::UHouse* house = multi->as_house();
          if ( house != nullptr )
          {
            Multi::CustomHouseStopEditing( const_cast<Mobile::Character*>( chr ), house );
            Multi::CustomHousesSendFull( house, chr->client, Multi::HOUSE_DESIGN_CURRENT );
          }
        }
      }
    }

    if ( !walkon_items.empty() )
    {
      *pwalkon = find_walkon_item( walkon_items, *newz );
    }
    else
    {
      *pwalkon = nullptr;
    }
  }

  return result;
}


bool Realm::lowest_walkheight( unsigned short x, unsigned short y, short oldz, short* newz,
                               Multi::UMulti** pmulti, Items::Item** pwalkon, bool doors_block,
                               Plib::MOVEMODE movemode, short* gradual_boost )
{
  if ( x >= width() || y >= height() )
  {
    return false;
  }

  static Plib::MapShapeList shapes;
  static MultiList mvec;
  static Core::ItemsVector walkon_items;
  shapes.clear();
  mvec.clear();
  walkon_items.clear();

  readdynamics( shapes, x, y, walkon_items, doors_block /* true */ );
  unsigned int flags = Plib::FLAG::MOVE_FLAGS;
  if ( movemode & Plib::MOVEMODE_FLY )
    flags |= Plib::FLAG::OVERFLIGHT;
  readmultis( shapes, x, y, flags, mvec );
  getmapshapes( shapes, x, y, flags );

  bool result;
  lowest_standheight( movemode, shapes, oldz, &result, newz, gradual_boost );

  if ( result )
  {
    if ( !mvec.empty() )
    {
      *pmulti = find_supporting_multi( mvec, *newz );
    }
    else
    {
      *pmulti = nullptr;
    }

    if ( !walkon_items.empty() )
    {
      *pwalkon = find_walkon_item( walkon_items, *newz );
    }
    else
    {
      *pwalkon = nullptr;
    }
  }

  return result;
}


bool Realm::dropheight( unsigned short dropx, unsigned short dropy, short dropz, short chrz,
                        short* newz, Multi::UMulti** pmulti )
{
  if ( dropx >= width() || dropy >= height() )
  {
    return false;
  }

  static Plib::MapShapeList shapes;
  static MultiList mvec;
  static Core::ItemsVector ivec;
  shapes.clear();
  mvec.clear();
  ivec.clear();

  readdynamics( shapes, dropx, dropy, ivec, true /* doors_block */ );
  readmultis( shapes, dropx, dropy, Plib::FLAG::DROP_FLAGS, mvec );
  getmapshapes( shapes, dropx, dropy, Plib::FLAG::DROP_FLAGS );

  bool result = dropheight( shapes, dropz, chrz, newz );
  if ( result )
  {
    if ( !mvec.empty() )
    {
      *pmulti = find_supporting_multi( mvec, *newz );
    }
    else
    {
      *pmulti = nullptr;
    }
  }
  return result;
}

// used to be statics_dropheight
bool Realm::dropheight( Plib::MapShapeList& shapes, short dropz, short chrz, short* newz )
{
  short z = Core::ZCOORD_MIN;
  bool result = false;

  for ( const auto& shape : shapes )
  {
    unsigned int flags = shape.flags;
    short ztop = shape.z + shape.height;
#if ENABLE_POLTEST_OUTPUT
    if ( static_debug_on )
    {
      INFO_PRINT << "static: graphic=0x" << fmt::hexu( srec.graphic ) << ", z=" << int( srec.z )
                 << ", ht=" << int( Plib::tileheight( srec.graphic ) ) << "\n";
    }
#endif

    if ( flags & Plib::FLAG::ALLOWDROPON )
    {
      if ( ( ztop <=
             chrz +
                 Core::settingsManager.ssopt.default_character_height ) &&  // not too high to reach
           ( ztop >= z ) )  // but above or same as the highest yet seen
      {                     // NOTE, the >= here is so statics at ground height
                            // will override a blocking map tile.
#if ENABLE_POLTEST_OUTPUT
        if ( static_debug_on )
          INFO_PRINT << "Setting Z to " << int( ztemp ) << "\n";
#endif
        z = ztop;
        result = true;
      }
    }
  }

  *newz = z;

  if ( result )
  {
    for ( const auto& shape : shapes )
    {
      if ( shape.flags & Plib::FLAG::BLOCKING )
      {
        short ztop = shape.z + shape.height;

        if ( shape.z <= z &&  // location blocked by some object
             z < ztop )
        {
#if ENABLE_POLTEST_OUTPUT
          if ( static_debug_on )
          {
            INFO_PRINT << "static: objtype=0x" << fmt::hexu( srec.graphic )
                       << ", z=" << int( srec.z )
                       << ", ht=" << int( Plib::tileheight( srec.graphic ) )
                       << " blocks movement to z=" << int( z ) << "\n";
          }
#endif
          result = false;
          break;
        }
        else if ( z < ztop && ztop <= dropz )
        {
          // a blocking item somewhere between where they _tried_ to drop it,
          // and where we decided to place it.
          result = false;
          break;
        }
      }
    }
  }

  return result;
}

void Realm::readmultis( Plib::MapShapeList& vec, unsigned short x, unsigned short y,
                        unsigned int anyflags ) const
{
  Core::WorldIterator<Core::MultiFilter>::InRange( x, y, this, 64, [&]( Multi::UMulti* multi ) {
    Multi::UHouse* house = multi->as_house();
    if ( house != nullptr && house->IsCustom() )  // readshapes switches to working design if the
                                                  // house is being edited,
      // everyone in the house would use it for walking...
      multi->readshapes( vec, s16( x ) - multi->x, s16( y ) - multi->y, multi->z );
    else
    {
      const Multi::MultiDef& def = multi->multidef();
      def.readshapes( vec, s16( x ) - multi->x, s16( y ) - multi->y, multi->z, anyflags );
    }
  } );
}

void Realm::readmultis( Plib::MapShapeList& vec, unsigned short x, unsigned short y,
                        unsigned int anyflags, MultiList& mvec ) const
{
  Core::WorldIterator<Core::MultiFilter>::InRange( x, y, this, 64, [&]( Multi::UMulti* multi ) {
    Multi::UHouse* house = multi->as_house();
    if ( house != nullptr && house->IsCustom() )
    {
      if ( multi->readshapes( vec, s16( x ) - multi->x, s16( y ) - multi->y, multi->z ) )
        mvec.push_back( multi );
    }
    else
    {
      const Multi::MultiDef& def = multi->multidef();
      if ( def.readshapes( vec, s16( x ) - multi->x, s16( y ) - multi->y, multi->z, anyflags ) )
      {
        mvec.push_back( multi );
      }
    }
  } );
}

void Realm::readmultis( Plib::StaticList& vec, unsigned short x, unsigned short y ) const
{
  Core::WorldIterator<Core::MultiFilter>::InRange( x, y, this, 64, [&]( Multi::UMulti* multi ) {
    Multi::UHouse* house = multi->as_house();
    if ( house != nullptr && house->IsCustom() )  // readshapes switches to working design if the
                                                  // house is being edited,
      // everyone in the house would use it for walking...
      multi->readobjects( vec, int( x ) - multi->x, int( y ) - multi->y, multi->z );
    else
    {
      const Multi::MultiDef& def = multi->multidef();
      def.readobjects( vec, int( x ) - multi->x, int( y ) - multi->y, multi->z );
    }
  } );
}

bool Realm::navigable( unsigned short x, unsigned short y, short z, short height = 0 ) const
{
  if ( !valid( x, y, z ) )
  {
    return false;
  }

  bool onwater = false;

  static Plib::MapShapeList shapes;
  shapes.clear();

  // possible: readdynamic, readmultis
  getmapshapes( shapes, x, y, static_cast<u32>( Plib::FLAG::ALL ) );

  for ( const auto& shape : shapes )
  {
    if ( shape.flags & Plib::FLAG::MOVESEA )
    {
      onwater = true;
    }
    else
    {
      if ( ( ( z + height ) >= ( shape.z - HULL_HEIGHT_BUFFER ) ) &&
           ( z < ( shape.z + shape.height + HULL_HEIGHT_BUFFER ) ) )
        return false;
    }
  }

  return onwater;
}

Multi::UMulti* Realm::find_supporting_multi( unsigned short x, unsigned short y, short z ) const
{
  if ( !valid( x, y, z ) )
  {
    return nullptr;
  }

  static Plib::MapShapeList vec;
  static MultiList mvec;
  vec.clear();
  mvec.clear();
  readmultis( vec, x, y, Plib::FLAG::MOVE_FLAGS, mvec );

  return find_supporting_multi( mvec, z );
}

/* The supporting multi is the highest multi that is below or equal
 * to the Z-coord of the supported object.
 */
Multi::UMulti* Realm::find_supporting_multi( MultiList& mvec, short z ) const
{
  Multi::UMulti* found = nullptr;
  for ( auto& multi : mvec )
  {
    if ( multi->z <= z )
    {
      if ( ( found == nullptr ) || ( multi->z > found->z ) )
      {
        found = multi;
      }
    }
  }
  return found;
}

bool Realm::findstatic( unsigned short x, unsigned short y, unsigned short objtype ) const
{
  if ( is_shadowrealm )
    return baserealm->_staticserver->findstatic( x, y, objtype );
  else
    return _staticserver->findstatic( x, y, objtype );
}

void Realm::getstatics( Plib::StaticEntryList& statics, unsigned short x, unsigned short y ) const
{
  if ( is_shadowrealm )
    return baserealm->_staticserver->getstatics( statics, x, y );
  else
    _staticserver->getstatics( statics, x, y );
}

bool Realm::groundheight( unsigned short x, unsigned short y, short* z ) const
{
  Plib::MAPTILE_CELL cell = _maptileserver->GetMapTile( x, y );
  *z = cell.z;

  if ( cell.landtile == GRAPHIC_NODRAW )  // it's a nodraw tile
    *z = Core::ZCOORD_MIN;

  return ( ( cell.landtile < 0x4000 ) &&
           ( ( Core::landtile_flags( cell.landtile ) & Plib::FLAG::BLOCKING ) == 0 ) );
}

Plib::MAPTILE_CELL Realm::getmaptile( unsigned short x, unsigned short y ) const
{
  if ( is_shadowrealm )
    return baserealm->_maptileserver->GetMapTile( x, y );
  else
    return _maptileserver->GetMapTile( x, y );
}

void Realm::getmapshapes( Plib::MapShapeList& shapes, unsigned short x, unsigned short y,
                          unsigned int anyflags ) const
{
  if ( is_shadowrealm )
    baserealm->_mapserver->GetMapShapes( shapes, x, y, anyflags );
  else
    _mapserver->GetMapShapes( shapes, x, y, anyflags );
}
}  // namespace Realms
}  // namespace Pol
