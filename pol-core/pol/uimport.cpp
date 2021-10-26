/** @file
 *
 * @par History
 * - 2005/01/23 Shinigami: for_all_mobiles, write_items & write_multis - Tokuno MapDimension doesn't
 * fit blocks of 64x64 (WGRID_SIZE)
 * - 2007/06/17 Shinigami: added config.world_data_path
 * - 2009/09/03 MuadDib:   Relocation of account related cpp/h
 *                         Relocation of multi related cpp/h
 * - 2009/09/14 MuadDib:   All UOX3 Import Code commented out. You can script this.
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 * - 2011/11/28 MuadDib:   Removed last of uox referencing code.
 */

#include "uimport.h"

#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <exception>
#include <future>
#include <string>
#include <time.h>

#include "../clib/Program/ProgramConfig.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/clib.h"
#include "../clib/clib_endian.h"
#include "../clib/esignal.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "../clib/threadhelp.h"
#include "../clib/timer.h"
#include "../plib/poltype.h"
#include "../plib/systemstate.h"
#include "accounts/accounts.h"
#include "containr.h"
#include "fnsearch.h"
#include "gameclck.h"
#include "globals/network.h"
#include "globals/object_storage.h"
#include "globals/settings.h"
#include "globals/state.h"
#include "globals/uvars.h"
#include "item/item.h"
#include "item/itemdesc.h"
#include "loaddata.h"
#include "mobile/charactr.h"
#include "mobile/npc.h"
#include "multi/house.h"
#include "multi/multi.h"
#include "objecthash.h"
#include "polvar.h"
#include "regions/resource.h"
#include "savedata.h"
#include "servdesc.h"
#include "startloc.h"
#include "storage.h"
#include "ufunc.h"
#include "uobject.h"
#include "uworld.h"

namespace Pol
{
namespace Module
{
void commit_datastore();
void read_datastore_dat();
void write_datastore( Clib::StreamWriter& sw );
}  // namespace Module
namespace Plib
{
bool version_greater_or_equal( const std::string& version_have, const std::string& version_need );
bool version_equal( const std::string& version_have, const std::string& version_need );
}  // namespace Plib
namespace Core
{
void read_party_dat();
void write_party( Clib::StreamWriter& sw );
void read_guilds_dat();
void write_guilds( Clib::StreamWriter& sw );

std::shared_future<bool> SaveContext::finished;

/****************** POL Native Files *******************/
// Dave changed 3/8/3 to use objecthash
void read_character( Clib::ConfigElem& elem )
{
  // if this object is modified in a subsequent incremental save,
  // don't load it now.
  pol_serial_t serial = 0;
  elem.get_prop( "SERIAL", &serial );
  if ( get_save_index( serial ) > objStorageManager.current_incremental_save )
    return;

  CharacterRef chr( new Mobile::Character( elem.remove_ushort( "OBJTYPE" ) ) );

  try
  {
    // note chr->logged_in is true..
    chr->readProperties( elem );

    // Allows the realm to recognize this char as offline
    chr->realm()->add_mobile( *chr, Realms::WorldChangeReason::PlayerLoad );

    chr->clear_dirty();

    // readProperties gets the serial, so we can't add to the objecthash until now.
    objStorageManager.objecthash.Insert( chr.get() );
  }
  catch ( std::exception& )
  {
    if ( chr.get() != nullptr )
      chr->destroy();
    throw;
  }
}

// Dave changed 3/8/3 to use objecthash
void read_npc( Clib::ConfigElem& elem )
{
  // if this object is modified in a subsequent incremental save,
  // don't load it now.
  pol_serial_t serial = 0;
  elem.get_prop( "SERIAL", &serial );
  if ( get_save_index( serial ) > objStorageManager.current_incremental_save )
    return;

  NpcRef npc( new Mobile::NPC( elem.remove_ushort( "OBJTYPE" ), elem ) );

  try
  {
    npc->readProperties( elem );

    SetCharacterWorldPosition( npc.get(), Realms::WorldChangeReason::NpcLoad );
    npc->clear_dirty();

    ////HASH
    objStorageManager.objecthash.Insert( npc.get() );
    ////
  }
  catch ( std::exception& )
  {
    if ( npc.get() != nullptr )
      npc->destroy();
    throw;
  }
}

// hmm, an interesting idea, what if there was an
// Item::create( ConfigElem& elem ) member,
// which would do this?
// Item would need a constructor (ConfigElem&) also.
// Polymorphism would then take
// care of it, and move the logic into the derived
// classes.
Items::Item* read_item( Clib::ConfigElem& elem )
{
  u32 serial;
  u32 objtype;
  if ( elem.remove_prop( "SERIAL", &serial ) == false )
  {
    ERROR_PRINT << "Item element has no SERIAL property, omitting.\n";
    return nullptr;
  }

  if ( !IsItem( serial ) )
  {
    elem.warn_with_line(
        "Item element does not have an item serial\n(should be larger than 0x40000000)" );
    if ( Plib::systemstate.config.check_integrity )
    {
      throw std::runtime_error( "Data integrity error" );
    }
    return nullptr;
  }

  if ( Plib::systemstate.config.check_integrity )
  {
    if ( system_find_item( serial ) )
    {
      ERROR_PRINT.Format( "Duplicate item read from datafiles (Serial=0x{:X})\n" ) << serial;
      throw std::runtime_error( "Data integrity error" );
    }
  }
  if ( elem.remove_prop( "OBJTYPE", &objtype ) == false )
  {
    ERROR_PRINT.Format( "Item (Serial 0x{:X}) has no OBJTYPE property, omitting." ) << serial;
    return nullptr;
  }
  if ( gamestate.old_objtype_conversions.count( objtype ) )
    objtype = gamestate.old_objtype_conversions[objtype];

  Items::Item* item = Items::Item::create( objtype, serial );
  if ( item == nullptr )
  {
    ERROR_PRINT.Format( "Unable to create item: objtype=0x{:X}, serial=0x{:X}" )
        << objtype << serial;
    if ( !Plib::systemstate.config.ignore_load_errors )
      throw std::runtime_error( "Item::create failed!" );
    else
      return nullptr;
  }
  item->setposition( Pos4d( item->pos().xyz(), find_realm( "britannia" ) ) );

  item->readProperties( elem );

  item->clear_dirty();

  return item;
}

#define USE_PARENT_CONTS 1

typedef std::stack<UContainer*> ContStack;
static ContStack parent_conts;

void read_global_item( Clib::ConfigElem& elem, int /*sysfind_flags*/ )
{
  // if this object is modified in a subsequent incremental save,
  // don't load it now.
  pol_serial_t serial = 0;
  elem.get_prop( "SERIAL", &serial );
  if ( get_save_index( serial ) > objStorageManager.current_incremental_save )
    return;


  u32 container_serial = 0;  // defaults to item in the world's top-level
  (void)elem.remove_prop( "CONTAINER",
                          &container_serial );  // therefore we don't need to check the return value

  Items::Item* item = read_item( elem );
  // dave added 1/15/3, protect against further crash if item is null. Should throw instead?
  if ( item == nullptr )
  {
    elem.warn_with_line( "Error reading item SERIAL or OBJTYPE." );
    return;
  }

  ItemRef itemref( item );  // dave 1/28/3 prevent item from being destroyed before function ends
  if ( container_serial == 0 )
  {
    add_item_to_world( item );
    if ( item->isa( UOBJ_CLASS::CLASS_CONTAINER ) )
      parent_conts.push( static_cast<UContainer*>( item ) );
  }
  else
  {
    if ( IsCharacter( container_serial ) )  // it's equipped on a character
    {
      Mobile::Character* chr = system_find_mobile( container_serial );
      if ( chr != nullptr )
      {
        equip_loaded_item( chr, item );
      }
      else
      {
        defer_item_insertion( item, container_serial );
      }
      return;
    }
    Items::Item* cont_item = nullptr;
    // bool new_parent_cont = false;

    while ( !parent_conts.empty() )
    {
      UContainer* cont = parent_conts.top();
      if ( cont->serial == container_serial )
      {
        cont_item = cont;
        break;
      }
      else
      {
        parent_conts.pop();
      }
    }

    if ( cont_item == nullptr )
    {
      cont_item = system_find_item( container_serial );
      // new_parent_cont = true;
    }

    if ( cont_item )
    {
      add_loaded_item( cont_item, item );
    }
    else
    {
      defer_item_insertion( item, container_serial );
    }
  }
}

void read_system_vars( Clib::ConfigElem& elem )
{
  settingsManager.polvar.DataWrittenBy = elem.remove_string( "CoreVersion" );
  settingsManager.polvar.DataWrittenBy99OrLater =
      Plib::version_greater_or_equal( settingsManager.polvar.DataWrittenBy, "99.0.0" );
  settingsManager.polvar.DataWrittenBy93 =
      Plib::version_equal( settingsManager.polvar.DataWrittenBy, "93.0.0" );
  stateManager.stored_last_item_serial =
      elem.remove_ulong( "LastItemSerialNumber", UINT_MAX );  // dave 3/9/3
  stateManager.stored_last_char_serial =
      elem.remove_ulong( "LastCharSerialNumber", UINT_MAX );  // dave 3/9/3
}

void read_shadow_realms( Clib::ConfigElem& elem )
{
  std::string name = elem.remove_string( "Name" );
  Realms::Realm* baserealm = find_realm( elem.remove_string( "BaseRealm" ) );
  if ( !baserealm )
    elem.warn_with_line( "BaseRealm not found." );
  else if ( defined_realm( name ) )
    elem.warn_with_line( "Realmname already defined" );
  else
  {
    add_realm( name, baserealm );
    INFO_PRINT << "\nShadowrealm " << name << "\n";
  }
}

void read_multi( Clib::ConfigElem& elem )
{
  // if this object is modified in a subsequent incremental save,
  // don't load it now.
  pol_serial_t serial = 0;
  elem.get_prop( "SERIAL", &serial );
  if ( get_save_index( serial ) > objStorageManager.current_incremental_save )
    return;

  u32 objtype;
  if ( elem.remove_prop( "SERIAL", &serial ) == false )
  {
    ERROR_PRINT << "A Multi has no SERIAL property.\n";
    throw std::runtime_error( "Config File error." );
  }
  if ( system_find_multi( serial ) || system_find_item( serial ) )
  {
    ERROR_PRINT.Format( "Duplicate item read from datafiles (Serial=0x{:X})\n" ) << serial;
    throw std::runtime_error( "Data integrity error" );
  }
  if ( elem.remove_prop( "OBJTYPE", &objtype ) == false )
  {
    ERROR_PRINT.Format( "Multi (Serial 0x{:X}) has no OBJTYPE property, omitting." ) << serial;
    return;
  }
  if ( gamestate.old_objtype_conversions.count( objtype ) )
    objtype = gamestate.old_objtype_conversions[objtype];

  Multi::UMulti* multi = Multi::UMulti::create( Items::find_itemdesc( objtype ), serial );
  if ( multi == nullptr )
  {
    ERROR_PRINT.Format( "Unable to create multi: objtype=0x{:X}, serial=0x{:X}\n" )
        << objtype << serial;
    throw std::runtime_error( "Multi::create failed!" );
  }
  multi->readProperties( elem );

  add_multi_to_world( multi );
}

std::string elapsed( clock_t start, clock_t end )
{
  size_t ms = static_cast<size_t>( ( end - start ) * 1000.0 / CLOCKS_PER_SEC );
  return Clib::tostring( ms ) + " ms";
}

void slurp( const char* filename, const char* tags, int sysfind_flags )
{
  static int num_until_dot = 1000;

  if ( Clib::FileExists( filename ) )
  {
    INFO_PRINT << "  " << filename << ":";
    Clib::ConfigFile cf( filename, tags );
    Clib::ConfigElem elem;

    Tools::Timer<> timer;

    unsigned int nobjects = 0;
    while ( cf.read( elem ) )
    {
      if ( --num_until_dot == 0 )
      {
        INFO_PRINT << ".";
        num_until_dot = 1000;
      }
      try
      {
        if ( stricmp( elem.type(), "CHARACTER" ) == 0 )
          read_character( elem );
        else if ( stricmp( elem.type(), "NPC" ) == 0 )
          read_npc( elem );
        else if ( stricmp( elem.type(), "ITEM" ) == 0 )
          read_global_item( elem, sysfind_flags );
        else if ( stricmp( elem.type(), "GLOBALPROPERTIES" ) == 0 )
          gamestate.global_properties->readProperties( elem );
        else if ( elem.type_is( "SYSTEM" ) )
          read_system_vars( elem );
        else if ( elem.type_is( "MULTI" ) )
          read_multi( elem );
        else if ( elem.type_is( "STORAGEAREA" ) )
        {
          StorageArea* storage_area = gamestate.storage.create_area( elem );
          // this will be followed by an item
          if ( !cf.read( elem ) )
            throw std::runtime_error( "Expected an item to exist after the storagearea." );

          storage_area->load_item( elem );
        }
        else if ( elem.type_is( "REALM" ) )
          read_shadow_realms( elem );
      }
      catch ( std::exception& )
      {
        if ( !Plib::systemstate.config.ignore_load_errors )
          throw;
      }
      ++nobjects;
    }

    timer.stop();

    INFO_PRINT << " " << nobjects << " elements in " << timer.ellapsed() << " ms.\n";
  }
}

void read_pol_dat()
{
  std::string polfile = Plib::systemstate.config.world_data_path + "pol.txt";

  slurp( polfile.c_str(), "GLOBALPROPERTIES SYSTEM REALM" );

  if ( settingsManager.polvar.DataWrittenBy.empty() )
  {
    ERROR_PRINT
        << "CoreVersion not found in " << polfile << "\n\n"
        << polfile << " must contain a section similar to: \n"
        << "System\n"
        << "{\n"
        << "\tCoreVersion 99\n"
        << "}\n\n"
        << "Ensure that the CoreVersion matches the version that created your data files!\n";
    throw std::runtime_error( "Data file error" );
  }
}

void read_objects_dat()
{
  slurp( ( Plib::systemstate.config.world_data_path + "objects.txt" ).c_str(),
         "CHARACTER NPC ITEM GLOBALPROPERTIES" );
}

void read_pcs_dat()
{
  slurp( ( Plib::systemstate.config.world_data_path + "pcs.txt" ).c_str(), "CHARACTER ITEM",
         SYSFIND_SKIP_WORLD );
}

void read_pcequip_dat()
{
  slurp( ( Plib::systemstate.config.world_data_path + "pcequip.txt" ).c_str(), "ITEM",
         SYSFIND_SKIP_WORLD );
}

void read_npcs_dat()
{
  slurp( ( Plib::systemstate.config.world_data_path + "npcs.txt" ).c_str(), "NPC ITEM",
         SYSFIND_SKIP_WORLD );
}

void read_npcequip_dat()
{
  slurp( ( Plib::systemstate.config.world_data_path + "npcequip.txt" ).c_str(), "ITEM",
         SYSFIND_SKIP_WORLD );
}

void read_items_dat()
{
  slurp( ( Plib::systemstate.config.world_data_path + "items.txt" ).c_str(), "ITEM" );
}

void read_multis_dat()
{
  slurp( ( Plib::systemstate.config.world_data_path + "multis.txt" ).c_str(), "MULTI" );
  //  string multisfile = config.world_data_path + "multis.txt";
  //  if (FileExists( multisfile ))
  //  {
  //    cout << multisfile << ":";
  //    ConfigFile cf( multisfile, "MULTI" );
  //    ConfigElem elem;
  //    while( cf.read( elem ))
  //    {
  //      UMulti* multi = read_multi( elem );
  //      if (multi == nullptr) throw runtime_error( "multi creation returned nullptr!" );
  //
  //      add_multi_to_world( multi );
  //    }
  //  }
}

void read_storage_dat()
{
  std::string storagefile = Plib::systemstate.config.world_data_path + "storage.txt";

  if ( Clib::FileExists( storagefile ) )
  {
    INFO_PRINT << "  " << storagefile << ":";
    Clib::ConfigFile cf2( storagefile );
    gamestate.storage.read( cf2 );
  }
}

Items::Item* find_existing_item( u32 objtype, const Pos4d& pos )
{
  Pos2d gridp = zone_convert( pos );
  for ( auto& item : pos.realm()->getzone_grid( gridp ).items )
  {
    // FIXME won't find doors which have been perturbed
    if ( item->objtype_ == objtype && item->pos() == pos )
    {
      return item;
    }
  }
  return nullptr;
}

int import_count;
int dupe_count;

void import( Clib::ConfigElem& elem )
{
  u32 objtype;
  objtype = elem.remove_unsigned( "OBJTYPE" );
  if ( objtype > Plib::systemstate.config.max_tile_id )
  {
    ERROR_PRINT.Format( "Importing file: 0x{:X} is out of range.\n" ) << objtype;
    throw std::runtime_error( "Error while importing file." );
  }

  Items::Item* item = Items::Item::create( objtype, 0x40000000 );  // dummy serial

  if ( item == nullptr )
  {
    ERROR_PRINT.Format( "Unable to import item: objtype=0x{:X}\n" ) << objtype;
    throw std::runtime_error( "Item::create failed!" );
  }

  item->readProperties( elem );

  if ( find_existing_item( item->objtype_, item->pos() ) )
  {
    item->destroy();
    ++dupe_count;
  }
  else
  {
    item->serial = GetNewItemSerialNumber();

    item->serial_ext = ctBEu32( item->serial );

    add_item_to_world( item );
    register_with_supporting_multi( item );
    ++import_count;
  }
}

void import_new_data()
{
  std::string importfile = Plib::systemstate.config.world_data_path + "import.txt";

  if ( Clib::FileExists( importfile ) )
  {
    Clib::ConfigFile cf( importfile, "ITEM" );
    Clib::ConfigElem elem;
    while ( cf.read( elem ) )
    {
      import( elem );
    }
    unlink( importfile.c_str() );
    INFO_PRINT << "Import Results: " << import_count << " imported, " << dupe_count
               << " duplicates.\n";
  }
}

void rndat( const std::string& basename )
{
  std::string datname = Plib::systemstate.config.world_data_path + basename + ".dat";
  std::string txtname = Plib::systemstate.config.world_data_path + basename + ".txt";

  if ( Clib::FileExists( datname.c_str() ) )
  {
    rename( datname.c_str(), txtname.c_str() );
  }
}

void rename_dat_files()
{
  rndat( "pol" );
  rndat( "objects" );
  rndat( "pcs" );
  rndat( "pcequip" );
  rndat( "npcs" );
  rndat( "npcequip" );
  rndat( "items" );
  rndat( "multis" );
  rndat( "storage" );
  rndat( "resource" );
  rndat( "guilds" );
  rndat( "parties" );
}

int read_data()
{
  std::string objectsndtfile = Plib::systemstate.config.world_data_path + "objects.ndt";
  std::string storagendtfile = Plib::systemstate.config.world_data_path + "storage.ndt";

  stateManager.gflag_in_system_load = true;
  if ( Clib::FileExists( objectsndtfile ) )
  {
    // Display reads "Reading data files..."
    ERROR_PRINT << "Error!\n"
                << "'" << objectsndtfile << " exists.  This probably means the system\n"
                << "exited while writing its state.  To avoid loss of data,\n"
                << "forcing human intervention.\n";
    throw std::runtime_error( "Human intervention required." );
  }
  if ( Clib::FileExists( storagendtfile ) )
  {
    ERROR_PRINT << "Error!\n"
                << "'" << storagendtfile << " exists.  This probably means the system\n"
                << "exited while writing its state.  To avoid loss of data,\n"
                << "forcing human intervention.\n";
    throw std::runtime_error( "Human intervention required." );
  }

  rename_dat_files();

  load_incremental_indexes();

  read_pol_dat();

  // POL clock should be paused at this point.
  start_gameclock();

  read_objects_dat();
  read_pcs_dat();
  read_pcequip_dat();
  read_npcs_dat();
  read_npcequip_dat();
  read_items_dat();
  read_multis_dat();
  read_storage_dat();
  read_resources_dat();
  read_guilds_dat();
  Module::read_datastore_dat();
  read_party_dat();

  read_incremental_saves();
  insert_deferred_items();

  register_deleted_serials();
  clear_save_index();

  import_new_data();
  //  import_wsc();

  // dave 3/9/3
  if ( stateManager.stored_last_item_serial < GetCurrentItemSerialNumber() )
    SetCurrentItemSerialNumber( stateManager.stored_last_item_serial );
  if ( stateManager.stored_last_char_serial < GetCurrentCharSerialNumber() )
    SetCurrentCharSerialNumber( stateManager.stored_last_char_serial );

  while ( !parent_conts.empty() )
    parent_conts.pop();

  for ( ObjectHash::hs::const_iterator citr = objStorageManager.objecthash.begin(),
                                       citrend = objStorageManager.objecthash.end();
        citr != citrend; ++citr )
  {
    UObject* obj = ( *citr ).second.get();
    if ( obj->ismobile() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );

      if ( chr->acct != nullptr )
        chr->logged_in( false );
    }
  }

  stateManager.gflag_in_system_load = false;
  return 0;
}


SaveContext::SaveContext()
    : _pol(),
      _objects(),
      _pcs(),
      _pcequip(),
      _npcs(),
      _npcequip(),
      _items(),
      _multis(),
      _storage(),
      _resource(),
      _guilds(),
      _datastore(),
      _party(),
      pol( &_pol ),
      objects( &_objects ),
      pcs( &_pcs ),
      pcequip( &_pcequip ),
      npcs( &_npcs ),
      npcequip( &_npcequip ),
      items( &_items ),
      multis( &_multis ),
      storage( &_storage ),
      resource( &_resource ),
      guilds( &_guilds ),
      datastore( &_datastore ),
      party( &_party )
{
  pol.init( Plib::systemstate.config.world_data_path + "pol.ndt" );
  objects.init( Plib::systemstate.config.world_data_path + "objects.ndt" );
  pcs.init( Plib::systemstate.config.world_data_path + "pcs.ndt" );
  pcequip.init( Plib::systemstate.config.world_data_path + "pcequip.ndt" );
  npcs.init( Plib::systemstate.config.world_data_path + "npcs.ndt" );
  npcequip.init( Plib::systemstate.config.world_data_path + "npcequip.ndt" );
  items.init( Plib::systemstate.config.world_data_path + "items.ndt" );
  multis.init( Plib::systemstate.config.world_data_path + "multis.ndt" );
  storage.init( Plib::systemstate.config.world_data_path + "storage.ndt" );
  resource.init( Plib::systemstate.config.world_data_path + "resource.ndt" );
  guilds.init( Plib::systemstate.config.world_data_path + "guilds.ndt" );
  datastore.init( Plib::systemstate.config.world_data_path + "datastore.ndt" );
  party.init( Plib::systemstate.config.world_data_path + "parties.ndt" );

  pcs() << "#" << pf_endl << "#  PCS.TXT: Player-Character Data" << pf_endl << "#" << pf_endl
        << "#  In addition to PC data, this also contains hair, beards, death shrouds," << pf_endl
        << "#  and backpacks, but not the contents of each backpack." << pf_endl << "#" << pf_endl
        << pf_endl;

  pcequip() << "#" << pf_endl << "#  PCEQUIP.TXT: Player-Character Equipment Data" << pf_endl << "#"
            << pf_endl
            << "#  This file can be deleted to wipe all items held/equipped by characters"
            << pf_endl
            << "#  Note that hair, beards, empty backpacks, and death shrouds are in PCS.TXT."
            << pf_endl << "#" << pf_endl << pf_endl;

  npcs() << "#" << pf_endl << "#  NPCS.TXT: Nonplayer-Character Data" << pf_endl << "#" << pf_endl
         << "#  If you delete this file to perform an NPC wipe," << pf_endl
         << "#  be sure to also delete NPCEQUIP.TXT" << pf_endl << "#" << pf_endl << pf_endl;

  npcequip() << "#" << pf_endl << "#  NPCEQUIP.TXT: Nonplayer-Character Equipment Data" << pf_endl
             << "#" << pf_endl << "#  Delete this file along with NPCS.TXT to perform an NPC wipe"
             << pf_endl << "#" << pf_endl << pf_endl;

  items() << "#" << pf_endl << "#  ITEMS.TXT: Item data" << pf_endl << "#" << pf_endl
          << "#  This file also contains ship and house components (doors, planks etc)" << pf_endl
          << "#" << pf_endl << pf_endl;

  multis() << "#" << pf_endl << "#  MULTIS.TXT: Ship and House data" << pf_endl << "#" << pf_endl
           << "#  Deleting this file will not properly wipe houses and ships," << pf_endl
           << "#  because doors, planks, and tillermen will be left in the world." << pf_endl << "#"
           << pf_endl << pf_endl;

  storage() << "#" << pf_endl
            << "#  STORAGE.TXT: Contains bank boxes, vendor inventories, and other data." << pf_endl
            << "#" << pf_endl
            << "#  This file can safely be deleted to wipe bank boxes and vendor inventories."
            << pf_endl << "#  Note that scripts may use this for other types of storage as well"
            << pf_endl << "#" << pf_endl << pf_endl;

  resource() << "#" << pf_endl << "#  RESOURCE.TXT: Resource System Data" << pf_endl << "#"
             << pf_endl << pf_endl;

  guilds() << "#" << pf_endl << "#  GUILDS.TXT: Guild Data" << pf_endl << "#" << pf_endl << pf_endl;

  datastore() << "#" << pf_endl << "#  DATASTORE.TXT: DataStore Data" << pf_endl << "#" << pf_endl
              << pf_endl;
  party() << "#" << pf_endl << "#  PARTIES.TXT: Party Data" << pf_endl << "#" << pf_endl << pf_endl;
}

SaveContext::~SaveContext()
{
  pol.flush_file();
  objects.flush_file();
  pcs.flush_file();
  pcequip.flush_file();
  npcs.flush_file();
  npcequip.flush_file();
  items.flush_file();
  multis.flush_file();
  storage.flush_file();
  resource.flush_file();
  guilds.flush_file();
  datastore.flush_file();
  party.flush_file();
}

/// blocks till possible last commit finishes
void SaveContext::ready()
{
  if ( SaveContext::finished.valid() )
  {
    // Tools::Timer<Tools::DebugT> t("future");
    SaveContext::finished.wait();
  }
}


void write_global_properties( Clib::StreamWriter& sw )
{
  sw() << "GlobalProperties" << pf_endl << "{" << pf_endl;
  gamestate.global_properties->printProperties( sw );
  sw() << "}" << pf_endl << pf_endl;
  // sw.flush();
}

void write_system_data( Clib::StreamWriter& sw )
{
  sw() << "System" << pf_endl << "{" << pf_endl << "\tCoreVersion\t" << POL_VERSION_STR << pf_endl
       << "\tCoreVersionString\t" << POL_VERSION_ID << pf_endl << "\tCompileDateTime\t"
       << Clib::ProgramConfig::build_datetime() << pf_endl << "\tLastItemSerialNumber\t"
       << GetCurrentItemSerialNumber() << pf_endl                                // dave 3/9/3
       << "\tLastCharSerialNumber\t" << GetCurrentCharSerialNumber() << pf_endl  // dave 3/9/3
       << "}" << pf_endl << pf_endl;
  // sw.flush();
}

void write_shadow_realms( Clib::StreamWriter& sw )
{
  for ( const auto& realm : gamestate.Realms )
  {
    if ( realm->is_shadowrealm )
    {
      sw() << "Realm" << pf_endl << "{" << pf_endl << "\tName\t" << realm->shadowname << pf_endl
           << "\tBaseRealm\t" << realm->baserealm->name() << pf_endl << "}" << pf_endl << pf_endl;
    }
  }
  // sw.flush();
}

// Austin (Oct. 17, 2006)
// Added to handle gotten item saving.
inline void WriteGottenItem( Mobile::Character* chr, Items::Item* item, Clib::StreamWriter& sw )
{
  if ( item == nullptr || item->orphan() )
    return;
  // For now, it just saves the item in items.txt
  item->setposition( chr->pos() );

  item->printOn( sw );

  item->setposition( Pos4d( 0, 0, 0, item->realm() ) );
}

void write_characters( Core::SaveContext& sc )
{
  for ( const auto& objitr : objStorageManager.objecthash )
  {
    UObject* obj = objitr.second.get();
    if ( obj->ismobile() && !obj->orphan() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
      if ( !chr->isa( UOBJ_CLASS::CLASS_NPC ) )
      {
        chr->printOn( sc.pcs );
        chr->clear_dirty();
        chr->printWornItems( sc.pcs, sc.pcequip );
      }
    }
  }
}

void write_npcs( Core::SaveContext& sc )
{
  for ( const auto& objitr : objStorageManager.objecthash )
  {
    UObject* obj = objitr.second.get();
    if ( obj->ismobile() && !obj->orphan() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
      if ( chr->isa( UOBJ_CLASS::CLASS_NPC ) )
      {
        if ( chr->saveonexit() )
        {
          chr->printOn( sc.npcs );
          chr->clear_dirty();
          chr->printWornItems( sc.npcs, sc.npcequip );
        }
      }
    }
  }
}

void write_items( Clib::StreamWriter& sw_items )
{
  for ( const auto& realm : gamestate.Realms )
  {
    for ( const auto& p : realm->gridarea() )
    {
      for ( const auto& item : realm->getzone_grid( p ).items )
      {
        if ( item->itemdesc().save_on_exit && item->saveonexit() )
        {
          sw_items << *item;
          item->clear_dirty();
        }
      }
    }
  }

  for ( const auto& objitr : objStorageManager.objecthash )
  {
    UObject* obj = objitr.second.get();
    if ( obj->ismobile() && !obj->orphan() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
      if ( !chr->isa( UOBJ_CLASS::CLASS_NPC ) )
      {
        // Figure out where to save the 'gotten item' - Austin (Oct. 17, 2006)
        if ( chr->has_gotten_item() )
          WriteGottenItem( chr, chr->gotten_item(), sw_items );
      }
    }
  }
}

void write_multis( Clib::StreamWriter& ofs )
{
  for ( const auto& realm : gamestate.Realms )
  {
    for ( const auto& p : realm->gridarea() )
    {
      for ( auto& multi : realm->getzone_grid( p ).multis )
      {
        if ( Clib::exit_signalled )  // drop waiting commit on shutdown
        {
          Multi::UHouse* house = multi->as_house();
          if ( house != nullptr )
          {
            if ( house->IsCustom() )
            {
              if ( house->IsWaitingForAccept() )
                house->AcceptHouseCommit( nullptr, false );
            }
          }
        }
        ofs << *multi;
        multi->clear_dirty();
      }
    }
  }
}

bool commit( const std::string& basename )
{
  std::string bakfile = Plib::systemstate.config.world_data_path + basename + ".bak";
  std::string datfile = Plib::systemstate.config.world_data_path + basename + ".txt";
  std::string ndtfile = Plib::systemstate.config.world_data_path + basename + ".ndt";
  const char* bakfile_c = bakfile.c_str();
  const char* datfile_c = datfile.c_str();
  const char* ndtfile_c = ndtfile.c_str();

  bool any = false;

  if ( Clib::FileExists( bakfile_c ) )
  {
    any = true;
    if ( unlink( bakfile_c ) )
    {
      int err = errno;
      POLLOG_ERROR.Format( "Unable to remove {}: {} ({})\n" )
          << bakfile_c << strerror( err ) << err;
    }
  }

  if ( Clib::FileExists( datfile_c ) )
  {
    any = true;
    if ( rename( datfile_c, bakfile_c ) )
    {
      int err = errno;
      POLLOG_ERROR.Format( "Unable to rename {} to {}: {} ({})\n" )
          << datfile_c << bakfile_c << strerror( err ) << err;
    }
  }

  if ( Clib::FileExists( ndtfile_c ) )
  {
    any = true;
    if ( rename( ndtfile_c, datfile_c ) )
    {
      int err = errno;
      POLLOG_ERROR.Format( "Unable to rename {} to {}: {} ({})\n" )
          << ndtfile_c << datfile_c << strerror( err ) << err;
    }
  }

  return any;
}

bool should_write_data()
{
  if ( Plib::systemstate.config.inhibit_saves )
    return false;
  if ( Clib::passert_shutdown_due_to_assertion && Clib::passert_nosave )
    return false;

  return true;
}

int write_data( unsigned int& dirty_writes, unsigned int& clean_writes, long long& elapsed_ms )
{
  SaveContext::ready();  // allow only one active
  if ( !should_write_data() )
  {
    dirty_writes = clean_writes = 0;
    elapsed_ms = 0;
    return -1;
  }

  UObject::dirty_writes = 0;
  UObject::clean_writes = 0;

  Tools::Timer<> timer;
  // launch complete save as seperate thread
  // but wait till the first critical part is finished
  // which means all objects got written into a format object
  // the remaining operations are only pure buffered i/o
  auto critical_promise = std::make_shared<std::promise<bool>>();
  auto critical_future = critical_promise->get_future();
  SaveContext::finished = std::async(
      std::launch::async,
      [&, critical_promise]() -> bool
      {
        std::atomic<bool> result( true );
        try
        {
          SaveContext sc;
          std::vector<std::future<bool>> critical_parts;
          critical_parts.push_back( gamestate.task_thread_pool.checked_push(
              [&]()
              {
                try
                {
                  sc.pol() << "#" << pf_endl << "#  Created by Version: " << POL_VERSION_ID
                           << pf_endl << "#  Mobiles: " << get_mobile_count() << pf_endl
                           << "#  Top-level Items: " << get_toplevel_item_count() << pf_endl << "#"
                           << pf_endl << pf_endl;

                  write_system_data( sc.pol );
                  write_global_properties( sc.pol );
                  write_shadow_realms( sc.pol );
                }
                catch ( ... )
                {
                  POLLOG_ERROR << "failed to store pol datafile!\n";
                  Clib::force_backtrace();
                  result = false;
                }
              } ) );
          critical_parts.push_back( gamestate.task_thread_pool.checked_push(
              [&]()
              {
                try
                {
                  write_items( sc.items );
                }
                catch ( ... )
                {
                  POLLOG_ERROR << "failed to store items datafile!\n";
                  Clib::force_backtrace();
                  result = false;
                }
              } ) );
          critical_parts.push_back( gamestate.task_thread_pool.checked_push(
              [&]()
              {
                try
                {
                  write_characters( sc );
                }
                catch ( ... )
                {
                  POLLOG_ERROR << "failed to store character datafile!\n";
                  Clib::force_backtrace();
                  result = false;
                }
              } ) );
          critical_parts.push_back( gamestate.task_thread_pool.checked_push(
              [&]()
              {
                try
                {
                  write_npcs( sc );
                }
                catch ( ... )
                {
                  POLLOG_ERROR << "failed to store npcs datafile!\n";
                  Clib::force_backtrace();
                  result = false;
                }
              } ) );
          critical_parts.push_back( gamestate.task_thread_pool.checked_push(
              [&]()
              {
                try
                {
                  write_multis( sc.multis );
                }
                catch ( ... )
                {
                  POLLOG_ERROR << "failed to store multis datafile!\n";
                  Clib::force_backtrace();
                  result = false;
                }
              } ) );
          critical_parts.push_back( gamestate.task_thread_pool.checked_push(
              [&]()
              {
                try
                {
                  gamestate.storage.print( sc.storage );
                }
                catch ( ... )
                {
                  POLLOG_ERROR << "failed to store storage datafile!\n";
                  Clib::force_backtrace();
                  result = false;
                }
              } ) );
          critical_parts.push_back( gamestate.task_thread_pool.checked_push(
              [&]()
              {
                try
                {
                  write_resources_dat( sc.resource );
                }
                catch ( ... )
                {
                  POLLOG_ERROR << "failed to store resource datafile!\n";
                  Clib::force_backtrace();
                  result = false;
                }
              } ) );
          critical_parts.push_back( gamestate.task_thread_pool.checked_push(
              [&]()
              {
                try
                {
                  write_guilds( sc.guilds );
                }
                catch ( ... )
                {
                  POLLOG_ERROR << "failed to store guilds datafile!\n";
                  Clib::force_backtrace();
                  result = false;
                }
              } ) );
          critical_parts.push_back( gamestate.task_thread_pool.checked_push(
              [&]()
              {
                try
                {
                  Module::write_datastore( sc.datastore );
                  // Atomically (hopefully) perform the switch.
                  Module::commit_datastore();
                }
                catch ( ... )
                {
                  POLLOG_ERROR << "failed to store datastore datafile!\n";
                  Clib::force_backtrace();
                  result = false;
                }
              } ) );
          critical_parts.push_back( gamestate.task_thread_pool.checked_push(
              [&]()
              {
                try
                {
                  write_party( sc.party );
                }
                catch ( ... )
                {
                  POLLOG_ERROR << "failed to store party datafile!\n";
                  Clib::force_backtrace();
                  result = false;
                }
              } ) );
          for ( auto& task : critical_parts )
            task.wait();

          critical_promise->set_value( result );  // critical part end
          // TODO: since promise can only be set one time move it into a method with a dedicated try
          // block, now when in theory an upper part fails the promise gets never set
        }  // deconstructor of the SaveContext flushes and joins the queues
        catch ( std::ios_base::failure& e )
        {
          POLLOG_ERROR << "failed to save datafiles! " << e.what() << ":" << std::strerror( errno )
                       << "\n";
          Clib::force_backtrace();
          result = false;
        }
        catch ( ... )
        {
          POLLOG_ERROR << "failed to save datafiles!\n";
          Clib::force_backtrace();
          result = false;
        }
        if ( result )
        {
          commit( "pol" );
          commit( "objects" );
          commit( "pcs" );
          commit( "pcequip" );
          commit( "npcs" );
          commit( "npcequip" );
          commit( "items" );
          commit( "multis" );
          commit( "storage" );
          commit( "resource" );
          commit( "guilds" );
          commit( "datastore" );
          commit( "parties" );
        }
        return true;
      } );
  critical_future.wait();  // wait for end of critical part

  if ( Plib::systemstate.accounts_txt_dirty )  // write accounts extra, since it uses extra thread
                                               // for io operations would be to many threads working
  {
    Accounts::write_account_data();
  }

  commit_incremental_saves();
  objStorageManager.incremental_save_count = 0;
  timer.stop();
  objStorageManager.objecthash.ClearDeleted();
  // optimize_zones(); // shrink zone vectors TODO this takes way to much time!

  // cout << "Clean: " << UObject::clean_writes << " Dirty: " <<
  // UObject::dirty_writes << endl;
  clean_writes = UObject::clean_writes;
  dirty_writes = UObject::dirty_writes;
  elapsed_ms = timer.ellapsed();

  objStorageManager.incremental_saves_disabled = false;
  return 0;
}

void read_starting_locations()
{
  Clib::ConfigFile cf( "config/startloc.cfg" );

  Clib::ConfigElem elem;
  while ( cf.read( elem ) )
  {
    if ( stricmp( elem.type(), "StartingLocation" ) != 0 )
    {
      ERROR_PRINT << "Unknown element type in startloc.cfg: " << elem.type() << "\n";
      throw std::runtime_error( "Error in configuration file." );
    }

    std::unique_ptr<StartingLocation> loc( new StartingLocation );
    loc->city = elem.remove_string( "CITY" );
    loc->desc = elem.remove_string( "DESCRIPTION" );
    loc->mapid = elem.remove_ushort( "MAPID", 0 );
    loc->cliloc_desc = elem.remove_unsigned( "CLILOC", 1075072 );
    loc->realm = find_realm( elem.remove_string( "REALM", "britannia" ) );

    std::string coord;
    while ( elem.remove_prop( "Coordinate", &coord ) )
    {
      int x, y, z;
      if ( sscanf( coord.c_str(), "%d,%d,%d", &x, &y, &z ) == 3 )
      {
        loc->coords.push_back(
            Pos3d( static_cast<u16>( x ), static_cast<u16>( y ), static_cast<s8>( z ) ) );
      }
      else
      {
        ERROR_PRINT << "Poorly formed coordinate in startloc.cfg: '" << coord << "' for city "
                    << loc->city << ", description " << loc->desc << "\n";
        throw std::runtime_error( "Configuration file error in startloc.cfg." );
      }
    }
    if ( loc->coords.size() == 0 )
    {
      ERROR_PRINT << "STARTLOC.CFG: StartingLocation (" << loc->city << "," << loc->desc
                  << ") has no Coordinate properties."
                  << "\n";
      throw std::runtime_error( "Configuration file error." );
    }
    gamestate.startlocations.push_back( loc.release() );
  }

  if ( gamestate.startlocations.empty() )
    throw std::runtime_error(
        "STARTLOC.CFG: No starting locations found.  Clients will crash on character creation." );
}

ServerDescription::ServerDescription() : name( "" ), port( 0 ), hostname( "" )
{
  memset( ip, 0, sizeof ip );
}

size_t ServerDescription::estimateSize() const
{
  size_t size = name.capacity() + 4 * sizeof( unsigned char ) /*ip*/
                + sizeof( unsigned short )                    /*port*/
                + 3 * sizeof( unsigned int* ) + ip_match.capacity() * sizeof( unsigned int ) +
                3 * sizeof( unsigned int* ) + ip_match_mask.capacity() * sizeof( unsigned int ) +
                3 * sizeof( std::string* ) + hostname.capacity();
  for ( const auto& s : acct_match )
    size += s.capacity();
  return size;
}

void read_gameservers()
{
  std::string accttext;

  Clib::ConfigFile cf( "config/servers.cfg" );

  Clib::ConfigElem elem;
  while ( cf.read( elem ) )
  {
    if ( !elem.type_is( "GameServer" ) )
      continue;

    std::unique_ptr<ServerDescription> svr( new ServerDescription );

    svr->name = elem.remove_string( "NAME" );

    std::string iptext;
    int ip0, ip1, ip2, ip3;
    iptext = elem.remove_string( "IP" );
    if ( iptext == "--ip--" )
    {
      iptext = networkManager.ipaddr_str;
      if ( iptext == "" )
      {
        INFO_PRINT << "Skipping server " << svr->name
                   << " because there is no Internet IP address.\n";
        continue;
      }
    }
    else if ( iptext == "--lan--" )
    {
      iptext = networkManager.lanaddr_str;
      if ( iptext == "" )
      {
        INFO_PRINT << "Skipping server " << svr->name << " because there is no LAN IP address.\n";
        continue;
      }
    }

    if ( isdigit( iptext[0] ) )
    {
      if ( sscanf( iptext.c_str(), "%d.%d.%d.%d", &ip0, &ip1, &ip2, &ip3 ) != 4 )
      {
        ERROR_PRINT << "SERVERS.CFG: Poorly formed IP (" << iptext << ") for GameServer '"
                    << svr->name << "'.\n";
        throw std::runtime_error( "Configuration file error." );
      }
      svr->ip[0] = static_cast<unsigned char>( ip3 );
      svr->ip[1] = static_cast<unsigned char>( ip2 );
      svr->ip[2] = static_cast<unsigned char>( ip1 );
      svr->ip[3] = static_cast<unsigned char>( ip0 );
    }
    else
    {
      svr->hostname = iptext;

#ifdef __linux__
      /* try to look up */
      struct hostent host_ret;
      struct hostent* host_result = nullptr;
      char tmp_buf[1024];
      int my_h_errno;
      int res = gethostbyname_r( svr->hostname.c_str(), &host_ret, tmp_buf, sizeof tmp_buf,
                                 &host_result, &my_h_errno );
      if ( res == 0 && host_result && host_result->h_addr_list[0] )
      {
        char* addr = host_result->h_addr_list[0];
        svr->ip[0] = addr[3];
        svr->ip[1] = addr[2];
        svr->ip[2] = addr[1];
        svr->ip[3] = addr[0];
        /*
                        struct sockaddr_in saddr;
                        memcpy( &saddr.sin_addr, he->h_addr_list[0], he->h_length);
                        server->ip[0] = saddr.sin_addr.S_un.S_un_b.s_b1;
                        server->ip[1] = saddr.sin_addr.S_un.S_un_b.s_b2;
                        server->ip[2] = saddr.sin_addr.S_un.S_un_b.s_b3;
                        server->ip[3] = saddr.sin_addr.S_un.S_un_b.s_b4;
                        */
      }
      else
      {
        POLLOG_ERROR.Format( "Warning: gethostbyname_r failed for server {} ({}): {}\n" )
            << svr->name << svr->hostname << my_h_errno;
      }
#endif
    }

    svr->port = elem.remove_ushort( "PORT" );

    while ( elem.remove_prop( "IPMATCH", &iptext ) )
    {
      auto delim = iptext.find_first_of( '/' );
      if ( delim != std::string::npos )
      {
        std::string ipaddr_str = iptext.substr( 0, delim );
        std::string ipmask_str = iptext.substr( delim + 1 );
        unsigned int ipaddr = inet_addr( ipaddr_str.c_str() );
        unsigned int ipmask = inet_addr( ipmask_str.c_str() );
        svr->ip_match.push_back( ipaddr );
        svr->ip_match_mask.push_back( ipmask );
      }
      else
      {
        unsigned int ipaddr = inet_addr( iptext.c_str() );
        svr->ip_match.push_back( ipaddr );
        svr->ip_match_mask.push_back( 0xFFffFFffLu );
      }
    }

    while ( elem.remove_prop( "ACCTMATCH", &accttext ) )
    {
      svr->acct_match.push_back( accttext );
    }

    networkManager.servers.push_back( svr.release() );
  }
  if ( networkManager.servers.empty() )
    throw std::runtime_error( "There must be at least one GameServer in SERVERS.CFG." );
}
}  // namespace Core
}  // namespace Pol
