/*
History
=======
2005/06/06 Shinigami: added readobjects - to get a list of statics
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
2009/09/03 MuadDib:   Relocation of multi related cpp/h
2009/09/14 MuadDib:   Squatters code added to register.unregister mobs.
2009/09/15 MuadDib:   Better cleanup handling on house destroy. Alos clears registered_house off character.

Notes
=======

*/


#include "../../clib/stl_inc.h"

#include <assert.h>

#include "../../bscript/berror.h"
#include "../../bscript/executor.h"
#include "../../bscript/objmembers.h"
#include "../../bscript/objmethods.h"

#include "../../clib/cfgelem.h"
#include "../../clib/endian.h"
#include "../../clib/logfile.h"
#include "../../clib/passert.h"
#include "../../clib/stlutil.h"
#include "../../clib/strutil.h"

#include "../../plib/mapcell.h"
#include "../../plib/realm.h"
#include "../../plib/mapshape.h"

#include "../network/cgdata.h"
#include "../core.h"
#include "../fnsearch.h"
#include "../item/itemdesc.h"
#include "multidef.h"
#include "../objtype.h"
#include "../polcfg.h"
#include "../realms.h"
#include "../tiles.h"
#include "../ufunc.h"
#include "../uofile.h"
#include "../uoscrobj.h"
#include "../ustruct.h"
#include "../uvars.h"
#include "../uworld.h"

#include "house.h"

#include "../objecthash.h"

typedef list<Item*>         ItemList;
typedef list<Character*>    MobileList;

void list_contents( const UHouse* house,
                    ItemList& items_in,
                    MobileList& chrs_in )
{
    const MultiDef& md = house->multidef();
    unsigned short wxL, wyL, wxH, wyH;
    short x1 = house->x + md.minrx, y1 = house->y + md.minry; 
    short x2 = house->x + md.maxrx, y2 = house->y + md.maxry;
    zone_convert_clip( x1, y1, house->realm, wxL, wyL );
    zone_convert_clip( x2, y2, house->realm, wxH, wyH );
    for( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
        for( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
            ZoneCharacters& wchr = house->realm->zone[wx][wy].characters;
            for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
            {
                Character* chr = *itr;
                if (chr->x >= x1 && chr->x <= x2 &&
                    chr->y >= y1 && chr->y <= y2)
                {
                    Item* walkon;
                    UMulti* multi;
                    short newz;
                    if (house->realm->walkheight( chr, chr->x, chr->y, chr->z, &newz, &multi, &walkon ))
                    {
                        if (const_cast<const UMulti*>(multi) == house)
                            chrs_in.push_back( chr );
                    }
                }
            }

            ZoneItems& witem = house->realm->zone[wx][wy].items;
            for( ZoneItems::iterator itr = witem.begin(), end = witem.end(); itr != end; ++itr )
            {
                Item* item = *itr;
                if (item->x >= x1 && item->x <= x2 &&
                    item->y >= y1 && item->y <= y2)
                {
                    Item* walkon;
                    UMulti* multi;
                    short newz;
                    unsigned short sx = item->x;
                    unsigned short sy = item->y;
                    item->x = 0;    // move 'self' a bit so it doesn't interfere with itself
                    item->y = 0;
                    bool res = house->realm->walkheight( sx, sy, item->z, &newz, &multi, &walkon, true, MOVEMODE_LAND );
                    item->x = sx;
                    item->y = sy;
                    if (res)
                    {
                        if (const_cast<const UMulti*>(multi) == house)
                        {
                            if (tile_flags(item->graphic) & FLAG::WALKBLOCK)
                                items_in.push_front( item );
                            else
                                items_in.push_back( item );
                        }
                    }
                }
            }
        }
    }
}

UHouse::UHouse( const ItemDesc& itemdesc ) : UMulti( itemdesc ),
editing(false),
editing_floor_num(1),
revision(0),
custom(false)
{
}

void UHouse::create_components()
{
    const MultiDef& md = multidef();
    for( unsigned i = 0; i < md.elems.size(); ++i )
    {
        const MULTI_ELEM& elem = md.elems[i];
        if (!elem.is_static)
        {
            Item* item = Item::create( elem.objtype );
            item->x = x + elem.x;
            item->y = y + elem.y;
            item->z = static_cast<s8>(z + elem.z);
            item->setprop( "house_serial", "i" + decint( serial ) );
            item->disable_decay();
            item->movable( false );
			item->realm = realm;
            update_item_to_inrange( item );
            add_item_to_world( item );
            components_.push_back( Component(item) );
        }
    }
}

ObjArray* UHouse::component_list() const
{
    ObjArray* arr = new ObjArray;
    for( Components::const_iterator itr = components_.begin(), end = components_.end(); itr != end; ++itr )
    {
        Item* item = (*itr).get();
        if (item != NULL && !item->orphan())
        {
            arr->addElement( new EItemRefObjImp(item) );
        }
    }
    return arr;
}

ObjArray* UHouse::items_list() const
{
    ItemList itemlist;
    MobileList moblist;
    list_contents( this, itemlist, moblist );
    ObjArray* arr = new ObjArray;
    for( ItemList::iterator itr = itemlist.begin(); itr != itemlist.end(); ++itr )
    {
        Item* item = (*itr);

        if (const_find_in( components_, Component(item) ) == components_.end())
        {
            arr->addElement( new EItemRefObjImp( item ) );
        }
    }
    return arr;
}

ObjArray* UHouse::mobiles_list() const
{
    ItemList itemlist;
    MobileList moblist;
    list_contents( this, itemlist, moblist );
    ObjArray* arr = new ObjArray;
    for( MobileList::iterator itr = moblist.begin(); itr != moblist.end(); ++itr )
    {
        Character* chr = (*itr);
        arr->addElement( new ECharacterRefObjImp( chr ) );
    }
    return arr;
}

UHouse* UHouse::as_house()
{
    return this;
}

BObjectImp* UHouse::get_script_member_id( const int id ) const ///id test
{
    BObjectImp* imp = base::get_script_member_id( id );
    if (imp)
        return imp;

    switch(id)
    {
        case MBR_COMPONENTS: return component_list(); break;
        case MBR_ITEMS: return items_list(); break;
        case MBR_MOBILES: return mobiles_list(); break;
		case MBR_CUSTOM: return new BLong(IsCustom());
        default: return NULL;
    }
}

BObjectImp* UHouse::get_script_member( const char *membername ) const
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_script_member_id(objmember->id);
	else
		return NULL;
    /*
	BObjectImp* imp = base::get_script_member( membername );
    if (imp)
        return imp;

    if (stricmp( membername, "components" ) == 0)
    {
        return component_list();
    }
    else if (stricmp( membername, "items" ) == 0)
    {
        return items_list();
    }
    else if (stricmp( membername, "mobiles" ) == 0)
    {
        return mobiles_list();
    }
	else if (stricmp( membername, "custom" ) == 0)
	{
		return new BLong(IsCustom());
	}

    return NULL;
	*/
}

BObjectImp* UHouse::script_method_id( const int id, Executor& ex )
{
    BObjectImp* imp = base::script_method_id( id, ex );
    if (imp != NULL)
        return imp;

	BApplicObjBase* aob = NULL;
    switch(id)
    {
        case MTH_SETCUSTOM:
            long _custom;
            if (ex.getParam( 0, _custom ))
            {
                SetCustom( _custom ? true : false );
                return new BLong(1);
            }
            else
                return new BError( "Invalid parameter type" );
		case MTH_ADD_COMPONENT:
			if(ex.hasParams( 0 ))
				aob = ex.getApplicObjParam(0, &eitemrefobjimp_type);

			if(aob != NULL)
			{
				EItemRefObjImp* ir = static_cast<EItemRefObjImp*>(aob);
				ItemRef iref = ir->value();
				components_.push_back(iref);
				return new BLong(1);
			}
			else
				return new BError( "Invalid parameter type" );

		case MTH_ERASE_COMPONENT:
			if(ex.hasParams( 0 ))
				aob = ex.getApplicObjParam(0, &eitemrefobjimp_type);

			if(aob != NULL)
			{
				EItemRefObjImp* ir = static_cast<EItemRefObjImp*>(aob);
				ItemRef iref = ir->value();
				Components::iterator pos;
				pos = find(components_.begin(), components_.end(), iref);
				if(pos != components_.end())
					components_.erase(pos);
				else
					return new BError("Component not found");
				return new BLong(1);
			}
			else
				return new BError( "Invalid parameter type" );

        default: return NULL;
    }
}

BObjectImp* UHouse::script_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->script_method_id(objmethod->id, ex);
	else
		return NULL;
	/*
    BObjectImp* imp = base::script_method( membername, ex );
    if (imp != NULL)
        return imp;

    if (stricmp( membername, "setcustom" ) == 0)
    {
        long _custom;
        if (ex.getParam( 0, _custom ))
        {
            SetCustom( _custom ? true : false );
            return new BLong(1);
        }
        else
            return new BError( "Invalid parameter type" );
    }
	else if(stricmp( membername, "add_component" ) == 0)
	{
		BApplicObjBase* aob = NULL;
		if(ex.hasParams( 0 ))
			aob = ex.getApplicObjParam(0, &eitemrefobjimp_type);

        if(aob != NULL)
		{
			EItemRefObjImp* ir = static_cast<EItemRefObjImp*>(aob);
			ItemRef iref = ir->value();
			components_.push_back(iref);
			return new BLong(1);
		}
		else
			return new BError( "Invalid parameter type" );
		
	}
	else if(stricmp( membername, "erase_component" ) == 0)
	{
		BApplicObjBase* aob = NULL;
		if(ex.hasParams( 0 ))
			aob = ex.getApplicObjParam(0, &eitemrefobjimp_type);

        if(aob != NULL)
		{
			EItemRefObjImp* ir = static_cast<EItemRefObjImp*>(aob);
			ItemRef iref = ir->value();
			Components::iterator pos;
			pos = find(components_.begin(), components_.end(), iref);
			if(pos != components_.end())
				components_.erase(pos);
			else
				return new BError("Component not found");
			return new BLong(1);
		}
		else
			return new BError( "Invalid parameter type" );
	}
    return NULL;
	*/
}

void UHouse::readProperties( ConfigElem& elem )
{
    base::readProperties( elem );
    u32 tmp_serial;
    
    while (elem.remove_prop( "Component", &tmp_serial ))
    {
        Item* item = find_toplevel_item( tmp_serial );
        if (item != NULL)
        {
            components_.push_back( Component(item) );
        }
    }
    custom = elem.remove_bool("Custom",false);
    if(custom)
    {
        short ysize,xsize,xbase,ybase;
        const MultiDef& def = multidef();
        ysize = def.maxry - def.minry + 1; //+1 to include offset 0 in -3..3
        xsize = def.maxrx - def.minrx + 1; //+1 to include offset 0 in -3..3
        xbase = (short)abs(def.minrx);
        ybase = (short)abs(def.minry);
        CurrentDesign.InitDesign(ysize+1,xsize,xbase,ybase); //+1 for front steps outside multidef footprint
        WorkingDesign.InitDesign(ysize+1,xsize,xbase,ybase); //+1 for front steps outside multidef footprint
        BackupDesign.InitDesign(ysize+1,xsize,xbase,ybase); //+1 for front steps outside multidef footprint
        CurrentDesign.readProperties(elem, "Current");
        //CurrentDesign.testprint(cout);
        WorkingDesign.readProperties(elem, "Working");
        BackupDesign.readProperties(elem, "Backup");

        elem.remove_prop("DesignRevision",&revision);
    }
}

void UHouse::printProperties( ostream& os ) const
{
    base::printProperties( os );

    for( Components::const_iterator itr = components_.begin(), end = components_.end(); itr != end; ++itr )
    {
        Item* item = (*itr).get();
        if (item != NULL && !item->orphan())
        {
            os << "\tComponent\t" << hexint( item->serial ) << pf_endl;
        }
    }
    os << "\tCustom\t" << custom << pf_endl;
    if(custom)
    {
        CurrentDesign.printProperties(os, "Current");
        WorkingDesign.printProperties(os, "Working");
        BackupDesign.printProperties(os, "Backup");
        os << "\tDesignRevision\t" << revision << pf_endl;
    }
}

void UHouse::destroy_components()
{
    while (!components_.empty())
    {
        Item* item = components_.back().get();
        if (config.loglevel >= 5)
            Log( "Destroying component %p, serial=0x%lu\n", item, item->serial );
        if (!item->orphan())
            ::destroy_item( item );
        if (config.loglevel >= 5)
            Log( "Component destroyed\n" );
        components_.pop_back();
    }
}

bool UHouse::readshapes( MapShapeList& vec, unsigned short x, unsigned short y, short zbase )
{
    if(!custom)
        return false;
    
    bool result = false;
    HouseFloorZColumn* elems;
    HouseFloorZColumn::iterator itr;
    CustomHouseDesign* design;
    design = editing ? &WorkingDesign : &CurrentDesign; //consider having a list of players that should use the working set
    
    if (x+design->xoff < 0 || x+design->xoff >= static_cast<s32>(design->width) || 
        y+design->yoff < 0 || y+design->yoff >= static_cast<s32>(design->height) )
        return false;
    for(int i=0; i<CUSTOM_HOUSE_NUM_PLANES;i++)
    {
        elems = design->Elements[i].GetElementsAt(x,y);
        for(itr = elems->begin(); itr != elems->end(); ++itr)
        {   
            MapShape shape;
            shape.z = itr->z+zbase;
            shape.height = tileheight( itr->graphic );
            shape.flags = tile_flags( itr->graphic );
            if (!shape.height)
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

bool UHouse::readobjects( StaticList& vec, unsigned short x, unsigned short y, short zbase )
{
    if(!custom)
        return false;
    
    bool result = false;
    HouseFloorZColumn* elems;
    HouseFloorZColumn::iterator itr;
    CustomHouseDesign* design;
    design = editing ? &WorkingDesign : &CurrentDesign; //consider having a list of players that should use the working set
    
    if (x+design->xoff < 0 || x+design->xoff >= static_cast<s32>(design->width) || 
        y+design->yoff < 0 || y+design->yoff >= static_cast<s32>(design->height) )
        return false;
    for(int i=0; i<CUSTOM_HOUSE_NUM_PLANES;i++)
    {
        elems = design->Elements[i].GetElementsAt(x,y);
        for(itr = elems->begin(); itr != elems->end(); ++itr)
        {
            StaticRec rec(itr->graphic, static_cast<signed char>(itr->z+zbase), tile_flags( itr->graphic ), tileheight( itr->graphic ));
            if (!rec.height)
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

//consider: storing editing char serial list on the house, to validate who should see the working set
UHouse* UHouse::FindWorkingHouse(u32 chrserial)
{
    Character* chr = find_character(chrserial);
    if(chr == NULL)
        return NULL;
    if(chr->client == NULL)
        return NULL;

    u32 house_serial = chr->client->gd->custom_house_serial;

    UMulti* multi = system_find_multi(house_serial);
    if(multi == NULL)
        return NULL;

    UHouse* house = multi->as_house();
    if(house == NULL)
        return NULL;

    return house;
}

//fixme realm
bool multis_exist_in( unsigned short mywest, unsigned short mynorth, 
                      unsigned short myeast, unsigned short mysouth, Realm* realm )
{
    unsigned short wxL, wyL, wxH, wyH;

    zone_convert_clip( mywest - 100, mynorth - 100, realm, wxL, wyL );
    zone_convert_clip( myeast + 100, mysouth + 100, realm, wxH, wyH );
    for( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
        for( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
            ZoneMultis& wmulti = realm->zone[wx][wy].multis;

            for( ZoneMultis::iterator itr = wmulti.begin(), end = wmulti.end(); itr != end; ++itr )
            {
                UMulti* it = (*itr);
                const MultiDef& edef = it->multidef();
                // find out if any of our walls would fall within its footprint. 
                unsigned short itswest, itseast, itsnorth, itssouth;

                itswest = static_cast<unsigned short>(it->x + edef.minrx);
                itseast = static_cast<unsigned short>(it->x + edef.maxrx);
                itsnorth = static_cast<unsigned short>(it->y + edef.minry);
                itssouth = static_cast<unsigned short>(it->y + edef.maxry);
        
                if (mynorth >= itsnorth && mynorth <= itssouth)         // North
                {
                    if ((mywest >= itswest && mywest <= itseast) ||     // NW
                        (myeast >= itswest && myeast <= itseast))       // NE
                    {
                        return true;
                    }
                }
                if (mysouth >= itsnorth && mysouth <= itssouth)         // South
                {
                    if ((mywest >= itswest && mywest <= itseast) ||     // SW
                        (myeast >= itswest && myeast <= itseast))       // SE
                    {
                        return true;
                    }
            
                }

                if (itsnorth >= mynorth && itsnorth <= mysouth)         // North
                {
                    if ((itswest >= mywest && itswest <= myeast) ||     // NW
                        (itseast >= mywest && itseast <= myeast))       // NE
                    {
                        return true;
                    }
                }
                if (itssouth >= mynorth && itssouth <= mysouth)         // South
                {
                    if ((itswest >= mywest && itswest <= myeast) ||     // SW
                        (itseast >= mywest && itseast <= myeast))       // SE
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool objects_exist_in( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, Realm* realm )
{
    unsigned short wxL, wyL, wxH, wyH;
    zone_convert_clip( x1, y1, realm, wxL, wyL );
    zone_convert_clip( x2, y2, realm, wxH, wyH );
    for( unsigned short wx = wxL; wx <= wxH; ++wx )
    {
        for( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
            ZoneCharacters& wchr = realm->zone[wx][wy].characters;
            for( ZoneCharacters::iterator itr = wchr.begin(), end = wchr.end(); itr != end; ++itr )
            {
                Character* chr = *itr;
                if (chr->x >= x1 && chr->x <= x2 &&
                    chr->y >= y1 && chr->y <= y2)
                {
                    return true;
                }
            }

            ZoneItems& witem = realm->zone[wx][wy].items;
            for( ZoneItems::iterator itr = witem.begin(), end = witem.end(); itr != end; ++itr )
            {
                Item* item = *itr;
                if (item->x >= x1 && item->x <= x2 &&
                    item->y >= y1 && item->y <= y2)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool statics_cause_problems( unsigned short x1, unsigned short y1,
                            unsigned short x2, unsigned short y2,
                            s8 z, long flags, Realm* realm)
{
    for( unsigned short x = x1; x <= x2; ++x )
    {
        for( unsigned short y = y1; y <= y2; ++y )
        {
            short newz;
            UMulti* multi;
            Item* item;
            if (!realm->walkheight( x, y, z, &newz, &multi, &item, true, MOVEMODE_LAND ))
            {
                Log( "Refusing to place house at %d,%d,%d: can't stand there\n", 
                     int(x),int(y),int(z) );
                return true;
            }
            if ( labs( z-newz ) > 2)
            {
                Log( "Refusing to place house at %d,%d,%d: result Z (%d) is too far afield\n",
                     int(x),int(y),int(z),int(newz) );
                return true;
            }
        }
    }
    return false;
}

BObjectImp* UHouse::scripted_create( const ItemDesc& descriptor, u16 x, u16 y, s8 z, Realm* realm, long flags )
{
    const MultiDef* md = MultiDefByGraphic( descriptor.graphic );
    if (md == NULL)
    {
        return new BError( "Multi definition not found for House, objtype=" 
                           + hexint(descriptor.objtype) + ", graphic=" 
                           + hexint(descriptor.graphic) );
    }

	if (~flags & CRMULTI_IGNORE_MULTIS)
    {
        if (multis_exist_in( x + md->minrx - 1, y + md->minry - 5,
                             x + md->maxrx + 1, y + md->maxry + 5, realm ))
	    {
            return new BError( "Location intersects with another structure" );
	    }
    }
    
    if (~flags & CRMULTI_IGNORE_OBJECTS)
    {
        if (objects_exist_in( x + md->minrx, y + md->minry,
                              x + md->maxrx, y + md->maxry, realm ))
        {
            return new BError( "Something is blocking that location" );
        }
    }
    if (~flags & CRMULTI_IGNORE_FLATNESS)
    {
        if (statics_cause_problems( x + md->minrx - 1, y + md->minry - 1, 
                                    x + md->maxrx + 1, y + md->maxry + 1, 
                                    z,
                                    flags, realm ))
        {
            return new BError( "That location is not suitable" );
        }
    }

    UHouse* house = new UHouse( descriptor );
    house->serial = GetNewItemSerialNumber();
    house->serial_ext = ctBEu32( house->serial );
    house->x = x;
    house->y = y;
    house->z = z;
	house->realm = realm;
    update_item_to_inrange( house );
    add_multi_to_world( house );
    house->create_components();

	////Hash
	objecthash.Insert(house);
	////

    return house->make_ref();
}


void move_to_ground( Item* item )
{
    item->set_dirty();
    item->movable(true);
    item->set_decay_after( 60 ); // just a dummy in case decay=0
    item->restart_decay_timer();
    for( unsigned short xd = 0; xd < 5; ++xd )
    {
        for( unsigned short yd = 0; yd < 5; ++yd )
        {
            Item* walkon;
            UMulti* multi;
            short newz;
            unsigned short sx = item->x;
            unsigned short sy = item->y;
            item->x = 0;    // move 'self' a bit so it doesn't interfere with itself
            item->y = 0;
            bool res = item->realm->walkheight( sx+xd, sy+yd, item->z, &newz, &multi, &walkon, true, MOVEMODE_LAND );
            item->x = sx;
            item->y = sy;
            if (res)
            {
                move_item( item, item->x+xd, item->y+yd, static_cast<signed char>(newz), NULL );
                return;
            }
        }
    }
    short newz;
    if (item->realm->groundheight( item->x, item->y, &newz ))
    {
        move_item( item, item->x, item->y, static_cast<signed char>(newz), NULL ); 
        return;
    }
}


void move_to_ground( Character* chr )
{
    move_character_to( chr, chr->x, chr->y, chr->z, MOVEITEM_FORCELOCATION, NULL );
}

//void send_remove_object_if_inrange( Client *client, const UObject *item );

BObjectImp* destroy_house( UHouse* house )
{
   
    house->destroy_components();

    list<Item*> item_contents;
    list<Character*> chr_contents;
    list_contents( house, item_contents, chr_contents );

    ConstForEach( clients, send_remove_object_if_inrange, static_cast<const Item*>(house) );
    remove_multi_from_world( house );

    while (!item_contents.empty())
    {
        Item* item = item_contents.front();
        move_to_ground( item );

        item_contents.pop_front();
    }

    while (!chr_contents.empty())
    {
        Character* chr = chr_contents.back();
        move_to_ground( chr );
		chr->registered_house = 0;
        chr_contents.pop_back();
    }
    
	house->ClearSquatters();

    house->destroy();
    
    return new BLong(1);
}

void UHouse::register_object( UObject* obj )
{ 
	if (find( squatters_.begin(), squatters_.end(), obj ) == squatters_.end())
	{
		set_dirty();
		squatters_.push_back( Squatter(obj) );
	}
}

void UHouse::unregister_object( UObject* obj )
{ 
	Squatters::iterator this_squatter = find( squatters_.begin(), squatters_.end(), obj );

	if ( this_squatter != squatters_.end())
	{
		set_dirty();
		squatters_.erase( this_squatter );
	}
}

void UHouse::ClearSquatters( )
{ 
	squatters_.clear();
}