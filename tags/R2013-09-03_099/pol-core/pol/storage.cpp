/*
History
=======
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors

Notes
=======

*/

#include "../clib/stl_inc.h"

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <stdio.h>
#include <time.h>

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/clib.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"

#include "fnsearch.h"
#include "gflag.h"
#include "item/item.h"
#include "loaddata.h"
#include "polcfg.h"
#include "savedata.h"
#include "storage.h"
#include "containr.h"
#include "ufunc.h"

StorageArea::StorageArea( string name ) : 
    _name(name)
{
}

StorageArea::~StorageArea()
{
    while (!_items.empty())
    {
        Cont::iterator itr = _items.begin();
        Item* item = (*itr).second;
        item->destroy();
        _items.erase( itr );
    }
}

Item* StorageArea::find_root_item( const string& name )
{
        // LINEAR_SEARCH
    Cont::iterator itr = _items.find( name );
    if (itr != _items.end())
    {
        return (*itr).second;
    }
    return NULL;
}

bool StorageArea::delete_root_item( const string& name )
{
    Cont::iterator itr = _items.find( name );
    if (itr != _items.end())
    {
        Item* item = (*itr).second;
        item->destroy();
        _items.erase( itr );
        return true;
    }
    return false;
}

void StorageArea::insert_root_item( Item* item )
{
    item->inuse( true );

    _items.insert( make_pair(item->name(), item) );
}

extern Item* read_item( ConfigElem& elem ); // from UIMPORT.CPP

void StorageArea::load_item( ConfigElem& elem )
{
    // if this object is modified in a subsequent incremental save,
    // don't load it yet. 
    pol_serial_t serial = 0;
    elem.get_prop( "SERIAL", &serial );
    if (get_save_index( serial ) > current_incremental_save)
        return;

	u32 container_serial = 0;
	elem.remove_prop( "CONTAINER", &container_serial );

    Item *item = read_item( elem );
	//Austin added 8/10/2006, protect against further crash if item is null. Should throw instead?
	if (item == NULL)
	{
		elem.warn_with_line("Error reading item SERIAL or OBJTYPE.");
		return;
	}
	if (container_serial == 0)
    {
        insert_root_item( item );
    }
	else
	{
        Item *cont_item = ::system_find_item( container_serial );

		if (cont_item)
		{
            add_loaded_item( cont_item, item );
		}
        else
        {
            defer_item_insertion( item, container_serial );
        }
	}
}
StorageArea* Storage::find_area( const string& name )
{
    AreaCont::iterator itr = areas.find( name );
    if (itr == areas.end())
        return NULL;
    else 
        return (*itr).second;
}

StorageArea* Storage::create_area( const string& name )
{
    AreaCont::iterator itr = areas.find( name );
    if (itr == areas.end())
    {
        StorageArea *area = new StorageArea( name );
        areas[ name ] = area;
        return area;
    }
    else
    {
        return (*itr).second;
    }
}

StorageArea* Storage::create_area( ConfigElem& elem )
{
    const char* rest = elem.rest();
    if (rest != NULL && rest[0])
    {
        return create_area( rest );
    }
    else
    {
        string name = elem.remove_string( "NAME" );
        return create_area( name );
    }
}


void StorageArea::print( ostream& os ) const
{
    for( Cont::const_iterator itr = _items.begin();
         itr != _items.end();
         ++itr )
    {
		const Item *item = (*itr).second;
		if ( item->saveonexit() )
			os << *item;
    }
}

void StorageArea::on_delete_realm(Realm *realm)
{
	for( Cont::const_iterator itr = _items.begin(), itrend=_items.end();
         itr != itrend;
         ++itr )
	{
		Item *item = (*itr).second;
        if (item)
        {
			setrealmif(item, (void*)realm);
			if (item->isa( UObject::CLASS_CONTAINER ))
			{
				UContainer* cont = static_cast<UContainer*>(item);
				cont->for_each_item(setrealmif, (void*)realm);
			}
        }
	}
}


ostream& operator<<( ostream& os, const StorageArea& area )
{
    area.print( os );
    return os;
}


void Storage::on_delete_realm(Realm *realm)
{
	for( AreaCont::const_iterator itr  = areas.begin(), itrend = areas.end();
         itr != itrend;
         ++itr )
	{
		itr->second->on_delete_realm(realm);
	}
}

void Storage::read( ConfigFile& cf )
{
    static int num_until_dot = 1000;
    unsigned int nobjects = 0;

    StorageArea *area = NULL;
    ConfigElem elem;

    clock_t start = clock();

    while (cf.read( elem ))
    {
        if (--num_until_dot == 0)
        {
            cout << ".";
            num_until_dot = 1000;
        }
        if (elem.type_is( "StorageArea" ))
        {
            area = create_area( elem );
        }
        else if (elem.type_is("Item"))
        {
            if (area != NULL)
            {
                try 
                {
                    area->load_item( elem );
                }
                catch( std::exception& )
                {
                    if (!config.ignore_load_errors)
                        throw;
                }
            }
            else
            {
                cerr << "Storage: Got an ITEM element, but don't have a StorageArea to put it." << endl;
                throw runtime_error( "Data file integrity error" );
            }
        }
        else
        {
            cerr << "Unexpected element type " << elem.type() << " in storage file." << endl;
            throw runtime_error( "Data file integrity error" );
        }
		++nobjects;
    }

	clock_t end = clock();
	int ms = static_cast<int>((end-start) * 1000.0 / CLOCKS_PER_SEC);

	cout << " " << nobjects << " elements in " << ms << " ms." << std::endl;
}

void Storage::print( ostream& os ) const
{
    for( AreaCont::const_iterator itr  = areas.begin();
         itr != areas.end();
         ++itr )
    {
        os << "StorageArea" << endl
           << "{" << endl
           << "\tName\t" << (*itr).first << endl
           << "}" << endl
           << endl
           << *((*itr).second)
           << endl;
    }
}

ostream& operator<<( ostream& os, const Storage& storage )
{
    storage.print( os );
    return os;
}

void Storage::clear()
{
    while (!areas.empty())
    {
        delete ( (*areas.begin()).second);
        areas.erase( areas.begin() );
    }
}
Storage storage;

#include "../bscript/contiter.h"
#include "../bscript/impstr.h"
#include "../bscript/berror.h"
#include "mkscrobj.h"

class StorageAreaIterator : public ContIterator
{
public:
    StorageAreaIterator( StorageArea* area, BObject* pIter );
    virtual BObject* step();
private:
    BObject* m_pIterVal;
    string key;
    StorageArea* _area;
};

StorageAreaIterator::StorageAreaIterator( StorageArea* area, BObject* pIter ) :
    m_pIterVal( pIter ),
    key(""),
    _area(area)
{
}

BObject* StorageAreaIterator::step()
{
    StorageArea::Cont::iterator itr = _area->_items.lower_bound( key );
    if (!key.empty() && itr != _area->_items.end())
    {
        ++itr;
    }

    if (itr == _area->_items.end())
        return NULL;

    key = (*itr).first;
    m_pIterVal->setimp( new String(key) );
    BObject* result = new BObject( make_itemref( (*itr).second ) );
    return result;
}


class StorageAreaImp : public BObjectImp
{
public:
    StorageAreaImp( StorageArea* area ) : BObjectImp( BObjectImp::OTUnknown ), _area(area) {}
    
    virtual BObjectImp* copy() const
    {
        return new StorageAreaImp( _area );
    }

    virtual string getStringRep() const
    {
        return _area->_name;
    }

    virtual size_t sizeEstimate() const
    {
        return sizeof(*this);
    }

    ContIterator* createIterator( BObject* pIterVal )
    {
        return new StorageAreaIterator( _area, pIterVal );
    }

    BObjectRef get_member( const char* membername );

private:
    StorageArea* _area;
};
BObjectRef StorageAreaImp::get_member( const char* membername )
{
    if (stricmp( membername, "count") == 0)
    {
        return BObjectRef( new BLong( static_cast<int>(_area->_items.size()) ) );
    }
    else if (stricmp( membername, "totalcount") == 0)
    {
        unsigned int total = 0;
        for( StorageArea::Cont::iterator itr = _area->_items.begin(); itr != _area->_items.end(); ++itr )
        {
            Item* item = (*itr).second;
            total += item->item_count();
        }
        return BObjectRef( new BLong( total ) );
    }
    return BObjectRef( UninitObject::create() );
}


class StorageAreasIterator : public ContIterator
{
public:
    StorageAreasIterator( BObject* pIter );
    virtual BObject* step();
private:
    BObject* m_pIterVal;
    string key;
};

StorageAreasIterator::StorageAreasIterator( BObject* pIter ) :
    m_pIterVal( pIter ),
    key("")
{
}

BObject* StorageAreasIterator::step()
{
    Storage::AreaCont::iterator itr = storage.areas.lower_bound( key );
    if (!key.empty() && itr != storage.areas.end())
    {
        ++itr;
    }

    if (itr == storage.areas.end())
        return NULL;

    key = (*itr).first;
    m_pIterVal->setimp( new String(key) );
    BObject* result = new BObject( new StorageAreaImp( (*itr).second ) );
    return result;
}

class StorageAreasImp : public BObjectImp
{
public:
    StorageAreasImp() : BObjectImp( BObjectImp::OTUnknown ) {}
    
    virtual BObjectImp* copy() const
    {
        return new StorageAreasImp();
    }

    virtual string getStringRep() const
    {
        return "<StorageAreas>";
    }

    virtual size_t sizeEstimate() const
    {
        return sizeof(*this);
    }

    ContIterator* createIterator( BObject* pIterVal )
    {
        return new StorageAreasIterator( pIterVal );
    }

    BObjectRef get_member( const char* membername );

    BObjectRef OperSubscript( const BObject& obj );

};

BObjectImp* CreateStorageAreasImp()
{
    return new StorageAreasImp();
}

BObjectRef StorageAreasImp::get_member( const char* membername )
{
    if (stricmp( membername, "count") == 0)
    {
        return BObjectRef( new BLong( static_cast<int>(storage.areas.size()) ) );
    }
    return BObjectRef( UninitObject::create() );
}
BObjectRef StorageAreasImp::OperSubscript( const BObject& obj )
{
	if (obj.isa( OTString ))
	{
		String& rtstr = (String&) obj.impref();
        string key = rtstr.value();
    
        Storage::AreaCont::iterator itr = storage.areas.find( key );
        if (itr != storage.areas.end())
        {
            return BObjectRef( new BObject( new StorageAreaImp( (*itr).second ) ) );
        }
        else
        {
            return BObjectRef( new BObject( new BError( "Storage Area not found" ) ) );
        }
    }
    return BObjectRef( new BObject( new BError( "Invalid parameter type" ) ) );
}

