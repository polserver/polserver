/*
History
=======
2007/06/17 Shinigami: added config.world_data_path

Notes
=======

*/

#include "../clib/stl_inc.h"

#ifdef __unix__
#include <unistd.h>
#endif

#include <errno.h>

#include "../clib/endian.h"
#include "../clib/fileutil.h"
#include "../clib/iohelp.h"
#include "../clib/logfile.h"
#include "../clib/strutil.h"
#include "../clib/wallclock.h"

#include "item/item.h"
#include "item/itemdesc.h"
#include "loaddata.h"
#include "objecthash.h"
#include "polcfg.h"
#include "storage.h"
#include "savedata.h"

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

vector< u32 > modified_serials;
vector< u32 > deleted_serials;
static unsigned int clean_objects;
static unsigned int dirty_objects;
bool incremental_saves_disabled = false;
void write_dirty_storage( ofstream& ofs_data )
{
    // these will all be processed again in write_dirty_data - but
    // they'll be clean then.  So we'll have to fudge the counters a little.

    for( Storage::AreaCont::const_iterator area_itr = storage.areas.begin();
         area_itr != storage.areas.end();
         ++area_itr )
    {
        const StorageArea* area = (*area_itr).second;

        for( StorageArea::Cont::const_iterator item_itr = area->_items.begin();
             item_itr != area->_items.end();
             ++item_itr )
        {
            const Item* item = (*item_itr).second;
            if (!item->dirty())
            {
                continue;
            }

            ++dirty_objects;
            --clean_objects; // because this will be counted again

            if (!item->orphan())
            {
                // write the storage area header, and the item (but not any contents)
                ofs_data << "StorageArea " << area->_name << pf_endl
                    << "{" << pf_endl
                    << "}" << pf_endl
                    << pf_endl;

                item->printSelfOn( ofs_data );
                modified_serials.push_back( item->serial );
            }
            else
            {
                deleted_serials.push_back( cfBEu32(item->serial_ext) );
            }
            item->clear_dirty();
        }
    }
}


void write_object_dirty_owners( ostream& ofs_data, const UObject* obj, bool& has_nonsaved_owner )
{
    const UObject* owner = obj->owner();
    if (owner)
    {
        if (dont_save_itemtype(owner->objtype_) ||
			owner->orphan() || !owner->saveonexit())
        {
            has_nonsaved_owner = true;
            return;
        }

        write_object_dirty_owners( ofs_data, owner, has_nonsaved_owner );
        if (has_nonsaved_owner)
            return;

        if (owner->dirty())
        {
            ++dirty_objects;
            // this will get counted again as we iterate through the objecthash
            --clean_objects; 

            owner->printSelfOn( ofs_data );
            modified_serials.push_back( owner->serial );
            owner->clear_dirty();
        }
    }
}

void write_dirty_data( ostream& ofs_data )
{
    // iterate over the object hash, writing dirty elements.
    // the only tricky bit here is we want to write dirty containers first.
    // this includes Characters.
    ObjectHash::hs::const_iterator citr = objecthash.begin(), end = objecthash.end();
    for( ; citr != end; ++citr )
    {
        const UObjectRef& ref = (*citr).second;
        const UObject* obj = ref.get();

        if (dont_save_itemtype(obj->objtype_))
            continue;

        if (!obj->dirty())
        {
            ++clean_objects;
            continue;
        }

        bool has_nonsaved_owner = false;
        write_object_dirty_owners( ofs_data, obj, has_nonsaved_owner );
        if (has_nonsaved_owner)
            continue;

        ++dirty_objects;
        if (!obj->orphan())
        {
            obj->printSelfOn( ofs_data );
            modified_serials.push_back( obj->serial );
        }
        else
        {
            deleted_serials.push_back( cfBEu32( obj->serial_ext ) );
        }
        obj->clear_dirty();
    }

    ObjectHash::ds::const_iterator deleted_citr = objecthash.dirty_deleted_begin(), deleted_end = objecthash.dirty_deleted_end();
    for( ; deleted_citr != deleted_end; ++deleted_citr )
    {
        u32 serial = (*deleted_citr);
        deleted_serials.push_back( serial );
    }
    objecthash.CleanDeleted();
}

void write_index( ostream& ofs )
{
    ofs << "Modified" << pf_endl
        << "{" << pf_endl;
    for( unsigned i = 0; i < modified_serials.size(); ++i )
    {
        ofs << "\t0x" << hex << modified_serials[i] << dec << pf_endl;
    }
    ofs << "}" << pf_endl
        << pf_endl;

    ofs << "Deleted" << pf_endl
        << "{" << pf_endl;
    for( unsigned i = 0; i < deleted_serials.size(); ++i )
    {
        ofs << "\t0x" << hex << deleted_serials[i] << dec << pf_endl;
    }
    ofs << "}" << pf_endl
        << pf_endl;
}

bool commit_incremental( const string& basename )
{
    string datfile = config.world_data_path + basename + ".txt";
    string ndtfile = config.world_data_path + basename + ".ndt";
    
    bool any = false;

    if (FileExists( datfile ))
    {
        any = true;
        if (unlink( datfile.c_str() ))
        {
            int err = errno;
            Log2( "Unable to delete %s: %s (%d)\n", datfile.c_str(), strerror(err), err );
        }
    }
    if (FileExists( ndtfile ))
    {
        any = true;
        if (rename( ndtfile.c_str(), datfile.c_str() ))
        {
            int err = errno;
            Log2( "Unable to rename %s to %s: %s (%d)\n", ndtfile.c_str(), datfile.c_str(), strerror(err), err );
        }
    }

    return any;
}

int save_incremental( unsigned int& dirty, unsigned int& clean, wallclock_diff_t& elapsed_ms )
{
    if (!should_write_data())
    {
        dirty = clean = elapsed_ms = 0;
        return -1;
    }

    if (incremental_saves_disabled)
        throw runtime_error( "Incremental saves are disabled until the next full save, due to a previous incremental save failure (dirty flags are inconsistent)" );

    try
    {
        wallclock_t start = wallclock();
        clean_objects = dirty_objects = 0;

        modified_serials.clear();
        deleted_serials.clear();

        ofstream ofs_data;
        ofstream ofs_index;

        ofs_data.exceptions( std::ios_base::failbit | std::ios_base::badbit );
        ofs_index.exceptions( std::ios_base::failbit | std::ios_base::badbit );
        
        unsigned save_index = incremental_save_count + 1;
        string data_basename = "incr-data-" + decint(save_index);
        string index_basename = "incr-index-" + decint(save_index);
        string data_pathname = config.world_data_path + data_basename + ".ndt";
        string index_pathname = config.world_data_path + index_basename + ".ndt";
        open_file( ofs_data,  data_pathname, ios::out );
        open_file( ofs_index, index_pathname, ios::out );

        write_system_data( ofs_data );
        write_global_properties( ofs_data);

        // TODO:
        //  guilds
        //  resources
        //  datastore

        write_dirty_storage( ofs_data );
        write_dirty_data( ofs_data );

        write_index( ofs_index );
        
        ofs_data.close();
        ofs_index.close();

        modified_serials.clear();
        deleted_serials.clear();

        commit_incremental( data_basename );
        commit_incremental( index_basename );
        ++incremental_save_count;

        wallclock_t finish = wallclock();
        dirty = dirty_objects;
        clean = clean_objects;
        elapsed_ms = wallclock_diff_ms( start, finish );
    }
    catch( std::exception& ex )
    {
        Log2( "Exception during incremental save: %s\n", ex.what() );
        incremental_saves_disabled = true;
        throw;
    }
    return 0;
}

void commit_incremental_saves()
{
    for( unsigned save_index = 1; ; ++save_index )
    {
        string data_basename = "incr-data-" + decint(save_index);
        string index_basename = "incr-index-" + decint(save_index);

        bool res1 = commit( data_basename );
        bool res2 = commit( index_basename );
        if (res1 || res2)
        {
            continue;
        }
        else
        {
            break;
        }
    }
}
