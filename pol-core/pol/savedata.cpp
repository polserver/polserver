/** @file
 *
 * @par History
 * - 2007/06/17 Shinigami: added config.world_data_path
 */


#include "savedata.h"

#include <cerrno>
#include <exception>
#include <fstream>

#include "../clib/clib_endian.h"
#include "../clib/fileutil.h"
#include "../clib/iohelp.h"
#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "../clib/streamsaver.h"
#include "../clib/strutil.h"
#include "../clib/timer.h"
#include "../plib/systemstate.h"
#include "globals/object_storage.h"
#include "globals/uvars.h"
#include "item/item.h"
#include "item/itemdesc.h"
#include "objecthash.h"
#include "storage.h"
#include "uobject.h"

namespace Pol
{
namespace Core
{
// incremental saves. yay.
// incrementals are saved in two files: the index and the data.
// the index looks like this:
// incr-index.1.txt:
// modified
// {
//     0x40002002
//     0x40002006
//      ...
// }
// deleted
// {
//     0x40002003
//     ...
// }
//
// the data just like normal data, except for storage area root items
// which are preceeded by:
// StorageArea [name]
// {
// }
// each such storage area element is always followed by a single item,
// which is the root item.
//
// all data goes into this one file.

void write_dirty_storage( Clib::StreamWriter& sw_data )
{
  // these will all be processed again in write_dirty_data - but
  // they'll be clean then.  So we'll have to fudge the counters a little.

  for ( Storage::AreaCont::const_iterator area_itr = gamestate.storage.areas.begin();
        area_itr != gamestate.storage.areas.end(); ++area_itr )
  {
    const StorageArea* area = ( *area_itr ).second;

    for ( StorageArea::Cont::const_iterator item_itr = area->_items.begin();
          item_itr != area->_items.end(); ++item_itr )
    {
      const Items::Item* item = ( *item_itr ).second;
      if ( !item->dirty() )
      {
        continue;
      }

      ++objStorageManager.dirty_objects;
      --objStorageManager.clean_objects;  // because this will be counted again

      if ( !item->orphan() )
      {
        // write the storage area header, and the item (but not any contents)
        sw_data.write( "StorageArea {}\n{{\n}}\n\n", area->_name );

        item->printSelfOn( sw_data );
        objStorageManager.modified_serials.push_back( item->serial );
      }
      else
      {
        objStorageManager.deleted_serials.push_back( cfBEu32( item->serial_ext ) );
      }
      item->clear_dirty();
    }
  }
}


void write_object_dirty_owners( Clib::StreamWriter& sw_data, const UObject* obj,
                                bool& has_nonsaved_owner )
{
  const UObject* owner = obj->owner();
  if ( owner )
  {
    auto id = Items::find_itemdesc( owner->objtype_ );
    if ( !id.save_on_exit || owner->orphan() || !owner->saveonexit() )
    {
      has_nonsaved_owner = true;
      return;
    }

    write_object_dirty_owners( sw_data, owner, has_nonsaved_owner );
    if ( has_nonsaved_owner )
      return;

    if ( owner->dirty() )
    {
      ++objStorageManager.dirty_objects;
      // this will get counted again as we iterate through the objecthash
      --objStorageManager.clean_objects;

      owner->printSelfOn( sw_data );
      objStorageManager.modified_serials.push_back( owner->serial );
      owner->clear_dirty();
    }
  }
}

void write_dirty_data( Clib::StreamWriter& sw_data )
{
  // iterate over the object hash, writing dirty elements.
  // the only tricky bit here is we want to write dirty containers first.
  // this includes Characters.
  ObjectHash::hs::const_iterator citr = objStorageManager.objecthash.begin(),
                                 end = objStorageManager.objecthash.end();
  for ( ; citr != end; ++citr )
  {
    const UObjectRef& ref = ( *citr ).second;
    const UObject* obj = ref.get();

    auto id = Items::find_itemdesc( obj->objtype_ );
    if ( !id.save_on_exit )
      continue;

    if ( !obj->dirty() )
    {
      ++objStorageManager.clean_objects;
      continue;
    }

    bool has_nonsaved_owner = false;
    write_object_dirty_owners( sw_data, obj, has_nonsaved_owner );
    if ( has_nonsaved_owner )
      continue;

    ++objStorageManager.dirty_objects;
    if ( !obj->orphan() )
    {
      obj->printSelfOn( sw_data );
      objStorageManager.modified_serials.push_back( obj->serial );
    }
    else
    {
      objStorageManager.deleted_serials.push_back( cfBEu32( obj->serial_ext ) );
    }
    obj->clear_dirty();
  }

  ObjectHash::ds::const_iterator deleted_citr = objStorageManager.objecthash.dirty_deleted_begin(),
                                 deleted_end = objStorageManager.objecthash.dirty_deleted_end();
  for ( ; deleted_citr != deleted_end; ++deleted_citr )
  {
    u32 serial = ( *deleted_citr );
    objStorageManager.deleted_serials.push_back( serial );
  }
  objStorageManager.objecthash.CleanDeleted();
}

void write_index( std::ostream& ofs )
{
  ofs << "Modified" << pf_endl << "{" << pf_endl;
  for ( unsigned i = 0; i < objStorageManager.modified_serials.size(); ++i )
  {
    ofs << "\t0x" << std::hex << objStorageManager.modified_serials[i] << std::dec << pf_endl;
  }
  ofs << "}" << pf_endl << pf_endl;

  ofs << "Deleted" << pf_endl << "{" << pf_endl;
  for ( unsigned i = 0; i < objStorageManager.deleted_serials.size(); ++i )
  {
    ofs << "\t0x" << std::hex << objStorageManager.deleted_serials[i] << std::dec << pf_endl;
  }
  ofs << "}" << pf_endl << pf_endl;
}

bool commit_incremental( const std::string& basename )
{
  std::string datfile = Plib::systemstate.config.world_data_path + basename + ".txt";
  std::string ndtfile = Plib::systemstate.config.world_data_path + basename + ".ndt";

  bool any = false;

  if ( Clib::FileExists( datfile ) )
  {
    any = true;
    if ( unlink( datfile.c_str() ) )
    {
      int err = errno;
      POLLOG_ERRORLN( "Unable to delete {}: {} ({})", datfile, strerror( err ), err );
    }
  }
  if ( Clib::FileExists( ndtfile ) )
  {
    any = true;
    if ( rename( ndtfile.c_str(), datfile.c_str() ) )
    {
      int err = errno;
      POLLOG_ERRORLN( "Unable to rename {} to {}: {} ({})", ndtfile, datfile, strerror( err ),
                      err );
    }
  }

  return any;
}

int save_incremental( unsigned int& dirty, unsigned int& clean, long long& elapsed_ms )
{
  if ( !should_write_data() )
  {
    dirty = clean = 0;
    elapsed_ms = 0;
    return -1;
  }

  if ( objStorageManager.incremental_saves_disabled )
    throw std::runtime_error(
        "Incremental saves are disabled until the next full save, due to a previous incremental "
        "save failure (dirty flags are inconsistent)" );

  try
  {
    Tools::Timer<> timer;
    objStorageManager.clean_objects = objStorageManager.dirty_objects = 0;

    objStorageManager.modified_serials.clear();
    objStorageManager.deleted_serials.clear();

    std::ofstream ofs_data;
    std::ofstream ofs_index;

    ofs_data.exceptions( std::ios_base::failbit | std::ios_base::badbit );
    ofs_index.exceptions( std::ios_base::failbit | std::ios_base::badbit );

    unsigned save_index = objStorageManager.incremental_save_count + 1;
    std::string data_basename = "incr-data-" + Clib::tostring( save_index );
    std::string index_basename = "incr-index-" + Clib::tostring( save_index );
    std::string data_pathname = Plib::systemstate.config.world_data_path + data_basename + ".ndt";
    std::string index_pathname = Plib::systemstate.config.world_data_path + index_basename + ".ndt";
    Clib::open_file( ofs_data, data_pathname, std::ios::out );
    Clib::open_file( ofs_index, index_pathname, std::ios::out );
    Clib::OFStreamWriter sw_data( &ofs_data );
    write_system_data( sw_data );
    write_global_properties( sw_data );

    // TODO:
    //  guilds
    //  resources
    //  datastore

    write_dirty_storage( sw_data );
    write_dirty_data( sw_data );

    write_index( ofs_index );

    ofs_data.close();
    ofs_index.close();

    objStorageManager.modified_serials.clear();
    objStorageManager.deleted_serials.clear();

    commit_incremental( data_basename );
    commit_incremental( index_basename );
    ++objStorageManager.incremental_save_count;

    timer.stop();
    dirty = objStorageManager.dirty_objects;
    clean = objStorageManager.clean_objects;
    elapsed_ms = timer.ellapsed();
  }
  catch ( std::exception& ex )
  {
    POLLOG_ERRORLN( "Exception during incremental save: {}", ex.what() );
    objStorageManager.incremental_saves_disabled = true;
    throw;
  }
  return 0;
}

void commit_incremental_saves()
{
  for ( unsigned save_index = 1;; ++save_index )
  {
    std::string data_basename = "incr-data-" + Clib::tostring( save_index );
    std::string index_basename = "incr-index-" + Clib::tostring( save_index );

    bool res1 = commit( data_basename );
    bool res2 = commit( index_basename );
    if ( res1 || res2 )
    {
      continue;
    }
    else
    {
      break;
    }
  }
}
}  // namespace Core
}  // namespace Pol
