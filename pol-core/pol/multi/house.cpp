/** @file
 *
 * @par History
 * - 2005/06/06 Shinigami: added readobjects - to get a list of statics
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 * - 2009/09/14 MuadDib:   Squatters code added to register.unregister mobs.
 * - 2009/09/15 MuadDib:   Better cleanup handling on house destroy. Alos clears registered_house
 * off character.
 *                         Houses now only allow mobiles to be registered. May add items later for
 * other storage.
 * - 2012/02/02 Tomi:      Added boat member MBR_MULTIID
 */

#include "house.h"

#include <boost/numeric/conversion/cast.hpp>
#include <stdlib.h>

#include "../../bscript/berror.h"
#include "../../bscript/executor.h"
#include "../../bscript/objmembers.h"
#include "../../bscript/objmethods.h"
#include "../../clib/cfgelem.h"
#include "../../clib/clib_endian.h"
#include "../../clib/logfacility.h"
#include "../../clib/passert.h"
#include "../../clib/refptr.h"
#include "../../clib/streamsaver.h"
#include "../../plib/mapcell.h"
#include "../../plib/mapshape.h"
#include "../../plib/systemstate.h"
#include "../../plib/uconst.h"
#include "../core.h"
#include "../fnsearch.h"
#include "../globals/object_storage.h"
#include "../globals/uvars.h"
#include "../item/itemdesc.h"
#include "../mobile/charactr.h"
#include "../module/uomod.h"
#include "../network/cgdata.h"
#include "../network/client.h"
#include "../realms/realm.h"
#include "../scrdef.h"
#include "../scrsched.h"
#include "../scrstore.h"
#include "../syshookscript.h"
#include "../ufunc.h"
#include "../uobject.h"
#include "../uoexec.h"
#include "../uoexhelp.h"
#include "../uoscrobj.h"
#include "../uworld.h"
#include "customhouses.h"
#include "multi.h"
#include "multidef.h"
#include <format/format.h>


namespace Pol
{
namespace Multi
{
void UHouse::list_contents( const UHouse* house, ItemList& items_in, MobileList& chrs_in )
{
  const MultiDef& md = house->multidef();
  short x1 = house->x + md.minrx, y1 = house->y + md.minry;
  short x2 = house->x + md.maxrx, y2 = house->y + md.maxry;

  Core::WorldIterator<Core::MobileFilter>::InBox(
      x1, y1, x2, y2, house->realm, [&]( Mobile::Character* chr ) {
        UMulti* multi = house->realm->find_supporting_multi( chr->x, chr->y, chr->z );
        if ( const_cast<const UMulti*>( multi ) == house )
          chrs_in.push_back( chr );
      } );
  Core::WorldIterator<Core::ItemFilter>::InBox(
      x1, y1, x2, y2, house->realm, [&]( Items::Item* item ) {
        UMulti* multi = house->realm->find_supporting_multi( item->x, item->y, item->z );
        if ( const_cast<const UMulti*>( multi ) == house )
        {
          if ( Plib::tile_flags( item->graphic ) & Plib::FLAG::WALKBLOCK )
            items_in.push_front( item );
          else
            items_in.push_back( item );
        }
      } );
}

UHouse::UHouse( const Items::ItemDesc& itemdesc )
    : UMulti( itemdesc ),
      editing( false ),
      waiting_for_accept( false ),
      editing_floor_num( 1 ),
      revision( 0 ),
      custom( false )
{
}

size_t UHouse::estimatedSize() const
{
  size_t size = base::estimatedSize() + CurrentDesign.estimatedSize() +
                WorkingDesign.estimatedSize() + BackupDesign.estimatedSize() + 3 * sizeof( u8* ) +
                CurrentCompressed.capacity() * sizeof( u8 ) + 3 * sizeof( u8* ) +
                WorkingCompressed.capacity() * sizeof( u8 ) + sizeof( bool ) /*editing*/
                + sizeof( bool )                                             /*waiting_for_accept*/
                + sizeof( int )                                              /*editing_floor_num*/
                + sizeof( u32 )                                              /*revision*/
                + sizeof( bool )                                             /*custom*/
                // no estimateSize here element is in objhash
                + 3 * sizeof( Squatter* ) + squatters_.capacity() * sizeof( Squatter ) +
                3 * sizeof( Component* ) + components_.capacity() * sizeof( Component );
  return size;
}

/**
 * Creates dynamic house components from MultiDef (multis.cfg at the time of writing)
 */
void UHouse::create_components()
{
  const MultiDef& md = multidef();
  for ( unsigned i = 0; i < md.elems.size(); ++i )
  {
    const MULTI_ELEM& elem = md.elems[i];
    if ( !elem.is_static )
    {
      Items::Item* item = Items::Item::create( elem.objtype );
      bool res = add_component( item, elem.x, elem.y, elem.z );
      passert_always_r( res,
                        "Couldn't add newly created item as house component. Please report this "
                        "bug on the forums." );
    }
  }
}

/**
 * Moves the item into the house, adding it as house component
 * (change item coordinates, set it unmovable, etc...)
 *
 * @param item Pointer to the item to be added
 * @param xoff The X offset inside the house
 * @param yoff The Y offset inside the house
 * @param zoff The Z offset inside the house
 * @return true on success, false when the item can't be added
 */
bool UHouse::add_component( Items::Item* item, s32 xoff, s32 yoff, s16 zoff )
{
  if ( !can_add_component( item ) )
    return false;

  u16 newx, newy;
  s8 newz;
  try
  {
    // These casts should be safe, but better check them - 2015-01-25 Bodom
    newx = boost::numeric_cast<u16>( x + xoff );
    newy = boost::numeric_cast<u16>( y + yoff );
    newz = boost::numeric_cast<s8>( z + zoff );
  }
  catch ( boost::bad_numeric_cast& )
  {
    // Printing an error because this is supposed to not happen,
    // so it's probably a bug.
    POLLOG_ERROR << "Out-of-range coordinates while trying to add Item "
                 << fmt::hexu( item->serial ) << " to House " << fmt::hexu( serial ) << '\n';
    return false;
  }
  item->x = newx;
  item->y = newy;
  item->z = newz;
  item->disable_decay();
  item->movable( false );
  item->realm = realm;
  update_item_to_inrange( item );
  add_item_to_world( item );
  add_component_no_check( Component( item ) );
  return true;
}

/**
 * (Re-)Adds a component to the House, without modifying it:
 * the item must already be inside the house, it just gets added to the components list
 *
 * @param item Reference to the item being added
 * @return true on success, false when the item can't be added
 */
bool UHouse::add_component( Component item )
{
  if ( !can_add_component( item.get() ) )
    return false;

  add_component_no_check( item );
  return true;
}

/**
 * Returns list of house components by allocating a new Array to be used by scripts
 *
 * @return ObjArray* pointer to the newly allocated array
 */
Bscript::ObjArray* UHouse::component_list() const
{
  std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray );
  for ( Components::const_iterator itr = components_.begin(), end = components_.end(); itr != end;
        ++itr )
  {
    Items::Item* item = ( *itr ).get();
    if ( item != nullptr && !item->orphan() )
    {
      arr->addElement( new Module::EItemRefObjImp( item ) );
    }
  }
  return arr.release();
}

Bscript::ObjArray* UHouse::items_list() const
{
  ItemList itemlist;
  MobileList moblist;
  list_contents( this, itemlist, moblist );
  std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray );
  for ( auto& item : itemlist )
  {
    if ( std::find( components_.cbegin(), components_.cend(), Component( item ) ) ==
         components_.cend() )
    {
      arr->addElement( new Module::EItemRefObjImp( item ) );
    }
  }
  return arr.release();
}

Bscript::ObjArray* UHouse::mobiles_list() const
{
  ItemList itemlist;
  MobileList moblist;
  list_contents( this, itemlist, moblist );
  std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray );
  for ( auto& chr : moblist )
  {
    arr->addElement( new Module::ECharacterRefObjImp( chr ) );
  }
  return arr.release();
}

UHouse* UHouse::as_house()
{
  return this;
}

Bscript::BObjectImp* UHouse::get_script_member_id( const int id ) const  /// id test
{
  using namespace Bscript;
  BObjectImp* imp = base::get_script_member_id( id );
  if ( imp )
    return imp;

  switch ( id )
  {
  case MBR_COMPONENTS:
    return component_list();
    break;
  case MBR_ITEMS:
    return items_list();
    break;
  case MBR_MOBILES:
    return mobiles_list();
    break;
  case MBR_CUSTOM:
    return new BLong( IsCustom() );
    break;
  case MBR_EDITING:
    return new BLong( IsEditing() );
    break;
  case MBR_MULTIID:
    return new BLong( multiid );
    break;
  case MBR_HOUSEPARTS:
    if ( !IsCustom() )
      return new BError( "House is not custom" );
    else if ( IsEditing() )
      return new BError( "House is currently been edited" );
    else
      return CurrentDesign.list_parts();
    break;
  default:
    return nullptr;
  }
}

Bscript::BObjectImp* UHouse::get_script_member( const char* membername ) const
{
  Bscript::ObjMember* objmember = Bscript::getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  else
    return nullptr;
}

Bscript::BObjectImp* UHouse::script_method_id( const int id, Bscript::Executor& ex )
{
  using namespace Bscript;
  BObjectImp* imp = base::script_method_id( id, ex );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MTH_SETCUSTOM:
  {
    int _custom;
    if ( ex.getParam( 0, _custom ) )
    {
      SetCustom( _custom ? true : false );
      return new BLong( 1 );
    }
    break;
  }
  case MTH_ADD_COMPONENT:
  {
    BApplicObjBase* aob;
    if ( ex.hasParams( 0 ) )
    {
      aob = ex.getApplicObjParam( 0, &Module::eitemrefobjimp_type );
      if ( aob )
      {
        Module::EItemRefObjImp* ir = static_cast<Module::EItemRefObjImp*>( aob );
        Core::ItemRef iref = ir->value();

        if ( add_component( iref ) )
          return new BLong( 1 );

        if ( iref->house() )
          return new BError( "Item is already an house component" );
        else
          return new BError( "Couldn't add component" );
      }
    }
    break;
  }
  case MTH_ERASE_COMPONENT:
  {
    BApplicObjBase* aob;
    if ( ex.hasParams( 0 ) )
    {
      aob = ex.getApplicObjParam( 0, &Module::eitemrefobjimp_type );
      if ( aob )
      {
        Module::EItemRefObjImp* ir = static_cast<Module::EItemRefObjImp*>( aob );
        Core::ItemRef iref = ir->value();
        Components::iterator pos;
        pos = find( components_.begin(), components_.end(), iref );
        if ( pos != components_.end() )
        {
          iref->house( nullptr );
          components_.erase( pos );
        }
        else
          return new BError( "Component not found" );
        return new BLong( 1 );
      }
    }
    break;
  }
  case MTH_ADD_HOUSE_PART:
  {
    if ( !IsCustom() )
      return new BError( "House is not custom" );
    else if ( IsEditing() )
      return new BError( "House is currently been edited" );
    else if ( !ex.hasParams( 4 ) )
      return new BError( "Not enough parameters" );
    u16 item_graphic;
    int xoff, yoff, item_z;
    if ( ex.getParam( 0, item_graphic ) && ex.getParam( 1, xoff ) && ex.getParam( 2, yoff ) &&
         ex.getParam( 3, item_z ) )
    {
      CUSTOM_HOUSE_ELEMENT elem;
      elem.graphic = item_graphic;
      elem.xoffset = xoff;
      elem.yoffset = yoff;
      elem.z = static_cast<u8>( item_z );
      CurrentDesign.Add( elem );
      // invalidate
      // invalidate
      WorkingDesign = CurrentDesign;
      std::vector<u8> newvec;
      WorkingCompressed.swap( newvec );
      std::vector<u8> newvec2;
      CurrentCompressed.swap( newvec2 );
      revision++;
      CustomHousesSendFullToInRange( this, HOUSE_DESIGN_CURRENT, RANGE_VISUAL_LARGE_BUILDINGS );
      return new BLong( 1 );
    }
    break;
  }
  case MTH_ERASE_HOUSE_PART:
  {
    if ( !IsCustom() )
      return new BError( "House is not custom" );
    else if ( IsEditing() )
      return new BError( "House is currently been edited" );
    else if ( !ex.hasParams( 4 ) )
      return new BError( "Not enough parameters" );
    int item_graphic, xoff, yoff, item_z;
    if ( ex.getParam( 0, item_graphic ) && ex.getParam( 1, xoff ) && ex.getParam( 2, yoff ) &&
         ex.getParam( 3, item_z ) )
    {
      bool ret =
          CurrentDesign.EraseGraphicAt( static_cast<u16>( item_graphic ), static_cast<u32>( xoff ),
                                        static_cast<u32>( yoff ), static_cast<u8>( item_z ) );
      if ( ret )
      {
        // invalidate
        WorkingDesign = CurrentDesign;
        std::vector<u8> newvec;
        WorkingCompressed.swap( newvec );
        std::vector<u8> newvec2;
        CurrentCompressed.swap( newvec2 );
        CustomHousesSendFullToInRange( this, HOUSE_DESIGN_CURRENT, RANGE_VISUAL_LARGE_BUILDINGS );
      }
      return new BLong( ret ? 1 : 0 );
    }
    break;
  }
  case MTH_ACCEPT_COMMIT:
  {
    if ( !IsCustom() )
      return new BError( "House is not custom" );
    // else if (!IsEditing())
    //  return new BError( "House is currently not been edited" );
    else if ( !IsWaitingForAccept() )
      return new BError( "House is currently not waiting for a commit" );
    else if ( !ex.hasParams( 2 ) )
      return new BError( "Not enough parameters" );
    int accept;
    Mobile::Character* chr;
    if ( ex.getParam( 1, accept ) && getCharacterParam( ex, 0, chr ) )
    {
      AcceptHouseCommit( chr, accept ? true : false );
      return new BLong( 1 );
    }
    break;
  }
  case MTH_CANCEL_EDITING:
  {
    if ( !IsCustom() )
      return new BError( "House is not custom" );
    else if ( !IsEditing() )
      return new BError( "House is currently not been edited" );
    else if ( !ex.hasParams( 2 ) )
      return new BError( "Not enough parameters" );
    Mobile::Character* chr;
    int drop_changes;
    if ( getCharacterParam( ex, 0, chr ) && ex.getParam( 1, drop_changes ) )
    {
      if ( chr->client->gd->custom_house_serial == serial )
        CustomHousesQuit( chr, drop_changes ? true : false );
      else
        return new BError( "Character is not editing this house" );
      return new BLong( 1 );
    }
    break;
  }

  default:
    return nullptr;
  }
  return new BError( "Invalid parameter type" );
}

Bscript::BObjectImp* UHouse::script_method( const char* methodname, Bscript::Executor& ex )
{
  Bscript::ObjMethod* objmethod = Bscript::getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->script_method_id( objmethod->id, ex );
  else
    return nullptr;
}

void UHouse::readProperties( Clib::ConfigElem& elem )
{
  base::readProperties( elem );
  u32 tmp_serial;
  multiid = elem.remove_ushort( "MultiID", this->multidef().multiid );

  while ( elem.remove_prop( "Component", &tmp_serial ) )
  {
    Items::Item* item = Core::find_toplevel_item( tmp_serial );
    if ( item != nullptr )
    {
      if ( !add_component( Component( item ) ) )
      {
        fmt::Writer os;
        os << "Couldn't add component " << fmt::hexu( item->serial ) << " to house "
           << fmt::hexu( serial ) << ".\n";
        UHouse* contHouse = item->house();
        if ( contHouse == nullptr )
        {
          os << "This is probably a core bug. Please report it on the forums.";
        }
        else
        {
          os << "This item is already part of house " << contHouse->serial << ".\n";
          os << "Allowing an item to be a component in two different houses was a bug,\n";
          os << "please also fix your save data.";
        }
        throw std::runtime_error( os.str() );
      }
    }
  }
  custom = elem.remove_bool( "Custom", false );
  if ( custom )
  {
    short ysize, xsize, xbase, ybase;
    const MultiDef& def = multidef();
    ysize = def.maxry - def.minry + 1;  //+1 to include offset 0 in -3..3
    xsize = def.maxrx - def.minrx + 1;  //+1 to include offset 0 in -3..3
    xbase = (short)abs( def.minrx );
    ybase = (short)abs( def.minry );
    CurrentDesign.InitDesign( ysize + 1, xsize, xbase,
                              ybase );  //+1 for front steps outside multidef footprint
    WorkingDesign.InitDesign( ysize + 1, xsize, xbase,
                              ybase );  //+1 for front steps outside multidef footprint
    BackupDesign.InitDesign( ysize + 1, xsize, xbase,
                             ybase );  //+1 for front steps outside multidef footprint
    CurrentDesign.readProperties( elem, "Current" );
    WorkingDesign.readProperties( elem, "Working" );
    BackupDesign.readProperties( elem, "Backup" );

    elem.remove_prop( "DesignRevision", &revision );
  }
}

void UHouse::printProperties( Clib::StreamWriter& sw ) const
{
  base::printProperties( sw );

  sw() << "\tMultiID\t" << multiid << pf_endl;

  for ( Components::const_iterator itr = components_.begin(), end = components_.end(); itr != end;
        ++itr )
  {
    Items::Item* item = ( *itr ).get();
    if ( item != nullptr && !item->orphan() )
    {
      sw() << "\tComponent\t0x" << fmt::hex( item->serial ) << pf_endl;
    }
  }
  sw() << "\tCustom\t" << custom << pf_endl;
  if ( custom )
  {
    CurrentDesign.printProperties( sw, "Current" );
    WorkingDesign.printProperties( sw, "Working" );
    BackupDesign.printProperties( sw, "Backup" );
    sw() << "\tDesignRevision\t" << revision << pf_endl;
  }
}

void UHouse::destroy_components()
{
  while ( !components_.empty() )
  {
    Items::Item* item = components_.back().get();
    if ( Plib::systemstate.config.loglevel >= 5 )
      POLLOG.Format( "Destroying component 0x{:X}, serial=0x{:X}\n" )
          << item->objtype_ << item->serial;
    if ( !item->orphan() )
      Core::destroy_item( item );
    if ( Plib::systemstate.config.loglevel >= 5 )
      POLLOG << "Component destroyed\n";
    components_.pop_back();
  }
}

bool UHouse::readshapes( Plib::MapShapeList& vec, short shape_x, short shape_y, short zbase )
{
  if ( !custom )
    return false;

  bool result = false;
  HouseFloorZColumn* elems;
  HouseFloorZColumn::iterator itr;
  CustomHouseDesign* design;
  design =
      editing
          ? &WorkingDesign
          : &CurrentDesign;  // consider having a list of players that should use the working set

  if ( shape_x + design->xoff < 0 || shape_x + design->xoff >= static_cast<s32>( design->width ) ||
       shape_y + design->yoff < 0 || shape_y + design->yoff >= static_cast<s32>( design->height ) )
    return false;
  for ( int i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
  {
    elems = design->Elements[i].GetElementsAt( shape_x, shape_y );
    for ( itr = elems->begin(); itr != elems->end(); ++itr )
    {
      Plib::MapShape shape;
      shape.z = itr->z + zbase;
      shape.height = Plib::tileheight( itr->graphic );
      shape.flags = Plib::tile_flags( itr->graphic );
      if ( !shape.height )
      {
        ++shape.height;
        --shape.z;
      }
      vec.push_back( shape );
      result = true;
    }
  }
  return result;
}

bool UHouse::readobjects( Plib::StaticList& vec, short obj_x, short obj_y, short zbase )
{
  if ( !custom )
    return false;

  bool result = false;
  HouseFloorZColumn* elems;
  HouseFloorZColumn::iterator itr;
  CustomHouseDesign* design;
  design =
      editing
          ? &WorkingDesign
          : &CurrentDesign;  // consider having a list of players that should use the working set

  if ( obj_x + design->xoff < 0 || obj_x + design->xoff >= static_cast<s32>( design->width ) ||
       obj_y + design->yoff < 0 || obj_y + design->yoff >= static_cast<s32>( design->height ) )
    return false;
  for ( int i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
  {
    elems = design->Elements[i].GetElementsAt( obj_x, obj_y );
    for ( itr = elems->begin(); itr != elems->end(); ++itr )
    {
      Plib::StaticRec rec( itr->graphic, static_cast<signed char>( itr->z + zbase ),
                           Plib::tile_flags( itr->graphic ), Plib::tileheight( itr->graphic ) );
      if ( !rec.height )
      {
        ++rec.height;
        --rec.z;
      }
      vec.push_back( rec );
      result = true;
    }
  }
  return result;
}

// consider: storing editing char serial list on the house, to validate who should see the working
// set
UHouse* UHouse::FindWorkingHouse( u32 chrserial )
{
  Mobile::Character* chr = Core::find_character( chrserial );
  if ( chr == nullptr )
    return nullptr;
  if ( chr->client == nullptr )
    return nullptr;

  u32 house_serial = chr->client->gd->custom_house_serial;

  UMulti* multi = Core::system_find_multi( house_serial );
  if ( multi == nullptr )
    return nullptr;

  UHouse* house = multi->as_house();
  if ( house == nullptr )
    return nullptr;

  return house;
}

// fixme realm
bool multis_exist_in( unsigned short mywest, unsigned short mynorth, unsigned short myeast,
                      unsigned short mysouth, Realms::Realm* realm )
{
  unsigned short wxL, wyL, wxH, wyH;

  Core::zone_convert_clip( mywest - 100, mynorth - 100, realm, &wxL, &wyL );
  Core::zone_convert_clip( myeast + 100, mysouth + 100, realm, &wxH, &wyH );
  for ( unsigned short wx = wxL; wx <= wxH; ++wx )
  {
    for ( unsigned short wy = wyL; wy <= wyH; ++wy )
    {
      for ( const auto& multi : realm->zone[wx][wy].multis )
      {
        const MultiDef& edef = multi->multidef();
        // find out if any of our walls would fall within its footprint.
        unsigned short itswest, itseast, itsnorth, itssouth;

        itswest = static_cast<unsigned short>( multi->x + edef.minrx );
        itseast = static_cast<unsigned short>( multi->x + edef.maxrx );
        itsnorth = static_cast<unsigned short>( multi->y + edef.minry );
        itssouth = static_cast<unsigned short>( multi->y + edef.maxry );

        if ( mynorth >= itsnorth && mynorth <= itssouth )  // North
        {
          if ( ( mywest >= itswest && mywest <= itseast ) ||  // NW
               ( myeast >= itswest && myeast <= itseast ) )   // NE
          {
            return true;
          }
        }
        if ( mysouth >= itsnorth && mysouth <= itssouth )  // South
        {
          if ( ( mywest >= itswest && mywest <= itseast ) ||  // SW
               ( myeast >= itswest && myeast <= itseast ) )   // SE
          {
            return true;
          }
        }

        if ( itsnorth >= mynorth && itsnorth <= mysouth )  // North
        {
          if ( ( itswest >= mywest && itswest <= myeast ) ||  // NW
               ( itseast >= mywest && itseast <= myeast ) )   // NE
          {
            return true;
          }
        }
        if ( itssouth >= mynorth && itssouth <= mysouth )  // South
        {
          if ( ( itswest >= mywest && itswest <= myeast ) ||  // SW
               ( itseast >= mywest && itseast <= myeast ) )   // SE
          {
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool objects_exist_in( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2,
                       Realms::Realm* realm )
{
  unsigned short wxL, wyL, wxH, wyH;
  Core::zone_convert_clip( x1, y1, realm, &wxL, &wyL );
  Core::zone_convert_clip( x2, y2, realm, &wxH, &wyH );
  auto includes = [&]( const Core::UObject* obj ) {
    if ( obj->x >= x1 && obj->x <= x2 && obj->y >= y1 && obj->y <= y2 )
    {
      return true;
    }
    return false;
  };
  for ( unsigned short wx = wxL; wx <= wxH; ++wx )
  {
    for ( unsigned short wy = wyL; wy <= wyH; ++wy )
    {
      for ( const auto& chr : realm->zone[wx][wy].characters )
      {
        if ( includes( chr ) )
          return true;
      }
      for ( const auto& chr : realm->zone[wx][wy].npcs )
      {
        if ( includes( chr ) )
          return true;
      }
      for ( const auto& item : realm->zone[wx][wy].items )
      {
        if ( includes( item ) )
          return true;
      }
    }
  }
  return false;
}

bool statics_cause_problems( unsigned short x1, unsigned short y1, unsigned short x2,
                             unsigned short y2, s8 z, int /*flags*/, Realms::Realm* realm )
{
  for ( unsigned short x = x1; x <= x2; ++x )
  {
    for ( unsigned short y = y1; y <= y2; ++y )
    {
      short newz;
      UMulti* multi;
      Items::Item* item;
      if ( !realm->walkheight( x, y, z, &newz, &multi, &item, true, Plib::MOVEMODE_LAND ) )
      {
        POLLOG.Format( "Refusing to place house at {},{},{}: can't stand there\n" ) << x << y << z;
        return true;
      }
      if ( labs( z - newz ) > 2 )
      {
        POLLOG.Format( "Refusing to place house at {},{},{}: result Z ({}) is too far afield\n" )
            << x << y << z << newz;
        return true;
      }
    }
  }
  return false;
}

Bscript::BObjectImp* UHouse::scripted_create( const Items::ItemDesc& descriptor, u16 x, u16 y, s8 z,
                                              Realms::Realm* realm, int flags )
{
  const MultiDef* md = MultiDefByMultiID( descriptor.multiid );
  if ( md == nullptr )
  {
    return new Bscript::BError(
        "Multi definition not found for House, objtype=" + Clib::hexint( descriptor.objtype ) +
        ", multiid=" + Clib::hexint( descriptor.multiid ) );
  }

  if ( ( !realm->valid( x + md->minrx, y + md->minry, z + md->minrz ) ) ||
       ( !realm->valid( x + md->maxrx, y + md->maxry, z + md->maxrz ) ) )
    return new Bscript::BError( "That location is out of bounds" );

  if ( ~flags & CRMULTI_IGNORE_MULTIS )
  {
    if ( multis_exist_in( x + md->minrx - 1, y + md->minry - 5, x + md->maxrx + 1,
                          y + md->maxry + 5, realm ) )
    {
      return new Bscript::BError( "Location intersects with another structure" );
    }
  }

  if ( ~flags & CRMULTI_IGNORE_OBJECTS )
  {
    if ( objects_exist_in( x + md->minrx, y + md->minry, x + md->maxrx, y + md->maxry, realm ) )
    {
      return new Bscript::BError( "Something is blocking that location" );
    }
  }
  if ( ~flags & CRMULTI_IGNORE_FLATNESS )
  {
    if ( statics_cause_problems( x + md->minrx - 1, y + md->minry - 1, x + md->maxrx + 1,
                                 y + md->maxry + 1, z, flags, realm ) )
    {
      return new Bscript::BError( "That location is not suitable" );
    }
  }

  UHouse* house = new UHouse( descriptor );
  house->serial = Core::GetNewItemSerialNumber();
  house->serial_ext = ctBEu32( house->serial );
  house->x = x;
  house->y = y;
  house->z = z;
  house->realm = realm;
  send_multi_to_inrange( house );
  // update_item_to_inrange( house );
  add_multi_to_world( house );
  house->create_components();

  ////Hash
  Core::objStorageManager.objecthash.Insert( house );
  ////

  return house->make_ref();
}


void move_to_ground( Items::Item* item )
{
  item->set_dirty();
  item->movable( true );
  item->set_decay_after( 60 );  // just a dummy in case decay=0
  item->restart_decay_timer();
  for ( unsigned short xd = 0; xd < 5; ++xd )
  {
    for ( unsigned short yd = 0; yd < 5; ++yd )
    {
      Items::Item* walkon;
      UMulti* multi;
      short newz;
      unsigned short sx = item->x;
      unsigned short sy = item->y;
      item->x = 0;  // move 'self' a bit so it doesn't interfere with itself
      item->y = 0;
      bool res = item->realm->walkheight( sx + xd, sy + yd, item->z, &newz, &multi, &walkon, true,
                                          Plib::MOVEMODE_LAND );
      item->x = sx;
      item->y = sy;
      if ( res )
      {
        move_item( item, item->x + xd, item->y + yd, static_cast<signed char>( newz ), nullptr );
        return;
      }
    }
  }
  short newz;
  if ( item->realm->groundheight( item->x, item->y, &newz ) )
  {
    move_item( item, item->x, item->y, static_cast<signed char>( newz ), nullptr );
    return;
  }
}


void move_to_ground( Mobile::Character* chr )
{
  move_character_to( chr, chr->x, chr->y, chr->z, Core::MOVEITEM_FORCELOCATION, nullptr );
}

// void send_remove_object_if_inrange( Client *client, const UObject *item );

Bscript::BObjectImp* destroy_house( UHouse* house )
{
  if ( house->IsEditing() )
    return new Bscript::BError( "House currently being customized." );

  house->destroy_components();

  ItemList item_contents;
  MobileList chr_contents;
  UHouse::list_contents( house, item_contents, chr_contents );

  send_remove_object_to_inrange( house );
  remove_multi_from_world( house );

  while ( !item_contents.empty() )
  {
    Items::Item* item = item_contents.front();
    move_to_ground( item );

    item_contents.pop_front();
  }

  while ( !chr_contents.empty() )
  {
    Mobile::Character* chr = chr_contents.back();
    move_to_ground( chr );
    chr->registered_house = 0;
    chr_contents.pop_back();
  }

  house->ClearSquatters();

  house->destroy();

  return new Bscript::BLong( 1 );
}

void UHouse::register_object( UObject* obj )
{
  if ( !obj->ismobile() )
    return;
  if ( find( squatters_.begin(), squatters_.end(), obj ) == squatters_.end() )
  {
    set_dirty();
    squatters_.push_back( Squatter( obj ) );
  }
}

void UHouse::unregister_object( UObject* obj )
{
  Squatters::iterator this_squatter = find( squatters_.begin(), squatters_.end(), obj );

  if ( this_squatter != squatters_.end() )
  {
    set_dirty();
    squatters_.erase( this_squatter );
  }
}

void UHouse::ClearSquatters()
{
  squatters_.clear();
}

void UHouse::walk_on( Mobile::Character* chr )
{
  const Items::ItemDesc& itemdesc = Items::find_itemdesc( objtype_ );
  if ( !itemdesc.walk_on_script.empty() )
  {
    ref_ptr<Bscript::EScriptProgram> prog;
    prog = find_script2( itemdesc.walk_on_script,
                         true,  // complain if not found
                         Plib::systemstate.config.cache_interactive_scripts );
    if ( prog.get() != nullptr )
    {
      std::unique_ptr<Core::UOExecutor> ex( Core::create_script_executor() );
      ex->addModule( new Module::UOExecutorModule( *ex ) );
      if ( prog->haveProgram )
      {
        ex->pushArg( new Bscript::BLong( chr->lastz ) );
        ex->pushArg( new Bscript::BLong( chr->lasty ) );
        ex->pushArg( new Bscript::BLong( chr->lastx ) );
        ex->pushArg( new Module::EItemRefObjImp( this ) );
        ex->pushArg( new Module::ECharacterRefObjImp( chr ) );
      }

      ex->priority(100);

      if ( ex->setProgram( prog.get() ) )
      {
        schedule_executor( ex.release() );
      }
    }
  }
}

void UHouse::AcceptHouseCommit( Mobile::Character* chr, bool accept )
{
  waiting_for_accept = false;
  if ( accept )
  {
    revision++;

    // commit working design to current design
    CurrentDesign = WorkingDesign;

    // invalidate old packet
    std::vector<u8> newvec;
    CurrentCompressed.swap( newvec );

    CustomHouseStopEditing( chr, this );

    // send full house
    CustomHousesSendFullToInRange( this, HOUSE_DESIGN_CURRENT, RANGE_VISUAL_LARGE_BUILDINGS );
  }
  else
  {
    WorkingDesign.AddComponents( this );
    CustomHouseDesign::ClearComponents( this );
    if ( chr && chr->client )
      CustomHousesSendFull( this, chr->client, HOUSE_DESIGN_WORKING );
  }
}

bool UHouse::get_method_hook( const char* methodname, Bscript::Executor* ex,
                              Core::ExportScript** hook, unsigned int* PC ) const
{
  if ( Core::gamestate.system_hooks.get_method_hook(
           Core::gamestate.system_hooks.house_method_script.get(), methodname, ex, hook, PC ) )
    return true;
  return base::get_method_hook( methodname, ex, hook, PC );
}
}  // namespace Multi
}  // namespace Pol
